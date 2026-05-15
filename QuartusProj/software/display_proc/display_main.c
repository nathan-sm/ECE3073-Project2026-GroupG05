#include <stdio.h>

#include "io.h"
#include "system.h"
#include "sys/alt_irq.h"

#include "common_defs.h"
#include "memory_addresses.h"

volatile int nextFrameReady = 0;
volatile uint32_t bufToDraw = 0;
uint32_t lastFrameTime = 0;

SharedDisplayState *displayState = (SharedDisplayState*)(SHARED_DISPLAY_STATE);
uint8_t *sourceBuffers[] = {
		(uint8_t*)(PROCESSED_IMG_BUF_A),
		(uint8_t*)(PROCESSED_IMG_BUF_B)
};

// ---- Display Functions ----

// Write a full image to pixel buffer using optimized 32-bit reads
void display_full_image(uint8_t *buffer)
{
    uint32_t* src_ptr32 = (uint32_t*)(buffer);
    uint32_t* const src_end32 = (uint32_t*)(buffer + IMAGE_SIZE);

    int addr = 0;

    while (src_ptr32 < src_end32) {
        uint32_t block = *src_ptr32++;

        IOWR(IMG_ADDY_BASE, 0, addr++);
        IOWR(PIXEL_DAT_BASE, 0, (block >> 4) & 0xF);

        IOWR(IMG_ADDY_BASE, 0, addr++);
        IOWR(PIXEL_DAT_BASE, 0, (block >> 12) & 0xF);

        IOWR(IMG_ADDY_BASE, 0, addr++);
        IOWR(PIXEL_DAT_BASE, 0, (block >> 20) & 0xF);

        IOWR(IMG_ADDY_BASE, 0, addr++);
        IOWR(PIXEL_DAT_BASE, 0, (block >> 28) & 0xF);
    }
}

// Write a quarter image to a specific quadrant of the pixel buffer
void display_quad_image(uint8_t *buffer, uint32_t imageIndex, uint32_t displayIndex)
{
    uint32_t imgAddr = imageIndex * QUAD_IMAGE_SIZE;
    uint32_t pixelBufferAddr = 0;

    if (displayIndex & 0x1)
    {
        pixelBufferAddr += QUAD_IMAGE_WIDTH;
    }
    if (displayIndex & 0x2)
    {
        pixelBufferAddr += 2 * QUAD_IMAGE_SIZE;
    }

    for (uint32_t i = 0; i < QUAD_IMAGE_HEIGHT; i++) {
        for (uint32_t j = 0; j < QUAD_IMAGE_WIDTH; j++) {
            IOWR(IMG_ADDY_BASE, 0, pixelBufferAddr);
            IOWR(PIXEL_DAT_BASE, 0, buffer[imgAddr] >> 4);
            imgAddr++;
            pixelBufferAddr++;
        }
        pixelBufferAddr += IMAGE_WIDTH - QUAD_IMAGE_WIDTH;
    }
}

// FPS DISPLAY FUNCTION
void display_fps(uint32_t elapsed) {
    if (elapsed == 0) return; // Prevent divide by zero on first frame

    uint32_t fps_100 = 100000000 / elapsed;

    const uint8_t SEG[10] = {
        0xC0, 0xF9, 0xA4, 0xB0, 0x99,
        0x92, 0x82, 0xF8, 0x80, 0x90
    };

    int d3 = (fps_100 / 1000) % 10;
    int d2 = (fps_100 / 100)  % 10;
    int d1 = (fps_100 / 10)   % 10;
    int d0 =  fps_100         % 10;

    uint8_t hex3 = SEG[d3];
    uint8_t hex2 = SEG[d2] & ~(1 << 7);
    uint8_t hex1 = SEG[d1];
    uint8_t hex0 = SEG[d0];

    uint32_t hex0_2 = ((uint32_t)hex2 << 16) | ((uint32_t)hex1 << 8) | hex0;
    uint32_t hex3_5 = (0xFF << 16) | (0xFF << 8) | hex3;

    IOWR(HEX20_BASE, 0, hex0_2);
    IOWR(HEX53_BASE, 0, hex3_5);
}

static void display_frame_rx_isr(void *context)
{
	nextFrameReady = 1;
	bufToDraw = IORD(DISPLAY_FRAME_MAILBOX_BASE, 0);
}

// benchmarking: print averages every N frames
#define BENCH_INTERVAL 20

int main()
{
	printf("Display proc main.\n");

    alt_ic_isr_register(
        DISPLAY_FRAME_MAILBOX_IRQ_INTERRUPT_CONTROLLER_ID,
        DISPLAY_FRAME_MAILBOX_IRQ,
        display_frame_rx_isr,
        NULL,
        NULL
    );

	IOWR(DISPLAY_FRAME_MAILBOX_BASE, 3, 0x1);

	lastFrameTime = IORD(USEC_COUNTER_BASE, 0);

	// benchmarking accumulators
	uint32_t bench_count = 0;
	uint32_t sum_frame   = 0;
	uint32_t sum_display = 0;

	printf("Display proc init. Benchmarking every %d frames.\n\n", BENCH_INTERVAL);

	while (1)
	{
		while (!nextFrameReady);
		nextFrameReady = 0;

		// frame-to-frame timing
		uint32_t currentTime = IORD(USEC_COUNTER_BASE, 0);
		uint32_t frameTime = 0;
		if (lastFrameTime != 0) {
			frameTime = currentTime - lastFrameTime;
			display_fps(frameTime);
			sum_frame += frameTime;
		}
		lastFrameTime = currentTime;

		int isQuad = displayState->isQuad;
		uint8_t *bufferDrawPtr = sourceBuffers[bufToDraw];

		// time the display
		uint32_t t_disp = IORD(USEC_COUNTER_BASE, 0);

		if (isQuad)
		{
			display_quad_image(bufferDrawPtr, displayState->quadDisplayIndices[0], 0);
			display_quad_image(bufferDrawPtr, displayState->quadDisplayIndices[1], 1);
			display_quad_image(bufferDrawPtr, displayState->quadDisplayIndices[2], 2);
			display_quad_image(bufferDrawPtr, displayState->quadDisplayIndices[3], 3);
		}
		else
		{
			display_full_image(bufferDrawPtr);
		}

		uint32_t disp_time = IORD(USEC_COUNTER_BASE, 0) - t_disp;
		sum_display += disp_time;
		bench_count++;

		if (bench_count >= BENCH_INTERVAL) {
			uint32_t avg_frame = sum_frame / bench_count;
			uint32_t avg_disp  = sum_display / bench_count;
			// processing time estimate: frame interval minus display time
			uint32_t avg_proc  = (avg_frame > avg_disp) ? avg_frame - avg_disp : 0;

			uint32_t fps_whole = avg_frame > 0 ? 1000000 / avg_frame : 0;
			uint32_t fps_frac  = avg_frame > 0 ? (100000000 / avg_frame) % 100 : 0;

			printf("===================================================\n");
			printf("  %s mode (avg over %lu frames)\n",
				   isQuad ? "QUAD" : "SINGLE", bench_count);
			printf("===================================================\n");
			printf("  Display:        %lu us\n", avg_disp);
			printf("  Processing:     ~%lu us (frame - display)\n", avg_proc);
			printf("  -------------------------------------------\n");
			printf("  Frame interval: %lu us\n", avg_frame);
			printf("  FPS:            %lu.%02lu\n\n", fps_whole, fps_frac);

			bench_count = 0;
			sum_frame   = 0;
			sum_display = 0;
		}
	}
}
