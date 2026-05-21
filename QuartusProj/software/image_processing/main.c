// Image Processing Nios II core -- applies selected processing mode to incoming
// frames and forwards processed output to the display core via mailbox.
// Created By: Nathan Morris       (32532601)
//             Ritwam Shohaum      (33156816)
//             Shuk Kan LUI        (33891885)
//             Evgeny Solomin      (34977260)
// Created Date: 2026
// version = '1.0'
#include <stdlib.h>
#include <stdio.h>

#include <stdint.h>
#include <string.h>
#include "system.h"
#include "io.h"
#include "sys/alt_irq.h"

#include "common_defs.h"
#include "memory_addresses.h"

// Processing mode codes selected by SW[2:1]
#define PROC_RAW   0
#define PROC_FLIP  1
#define PROC_BLUR  2
#define PROC_EDGE  3

// Mailbox register offsets
#define MAILBOX_STATUS_FULL 0x02  // bit in the STATUS register indicating mailbox is full
#define MAILBOX_IRQ_ENABLE  0x01  // value written to the IRQ enable register

// Threshold used by Sobel edge detection to binarise gradient magnitude
#define SOBEL_EDGE_THRESHOLD 60

// Triple buffer base addresses in SDRAM (cached — read-only from this core's perspective)
uint8_t* buffers[3] = {
    (uint8_t*)(IMAGE_READ_BUF_A),
    (uint8_t*)(IMAGE_READ_BUF_B),
    (uint8_t*)(IMAGE_READ_BUF_C)
};

SharedAccelData* shared_accel = (SharedAccelData*)(SHARED_ACCEL_DATA);
SharedDisplayState* shared_display = (SharedDisplayState*)(SHARED_DISPLAY_STATE);

// Processing output buffer - avoids writing into shared triple buffers
uint16_t *processing_buffers[] = {
		(uint16_t*)(PROCESSED_IMG_BUF_A),
		(uint16_t*)(PROCESSED_IMG_BUF_B)
};

SharedTimingData *sharedTimingData = (SharedTimingData*)(SHARED_TIMING_DATA);

volatile int new_frame_ready = 0;
volatile uint32_t current_frame_index = 0;

uint32_t bufToSend = 0;

// Map processing arrays explicitly into the massive shared SDRAM void
// to prevent the Image Core's .bss partition from overflowing.
#define SCRATCHPAD_BASE (SHARED_TIMING_DATA + 1024) // Start safely after timing data

uint16_t *unpacked_rgb     = (uint16_t*)(SCRATCHPAD_BASE);
uint8_t  *grayscale_buffer = (uint8_t*)(SCRATCHPAD_BASE + (IMAGE_SIZE * 2));
uint8_t  *edge_buffer      = (uint8_t*)(SCRATCHPAD_BASE + (IMAGE_SIZE * 3));
uint8_t  *r_chan           = (uint8_t*)(SCRATCHPAD_BASE + (IMAGE_SIZE * 4));
uint8_t  *g_chan           = (uint8_t*)(SCRATCHPAD_BASE + (IMAGE_SIZE * 5));
uint8_t  *b_chan           = (uint8_t*)(SCRATCHPAD_BASE + (IMAGE_SIZE * 6));
uint8_t  *r_blur           = (uint8_t*)(SCRATCHPAD_BASE + (IMAGE_SIZE * 7));
uint8_t  *g_blur           = (uint8_t*)(SCRATCHPAD_BASE + (IMAGE_SIZE * 8));
uint8_t  *b_blur           = (uint8_t*)(SCRATCHPAD_BASE + (IMAGE_SIZE * 9));

// ISR: called when the communications core deposits a new frame buffer token
static void mailbox_rx_isr(void* context) {
    current_frame_index = IORD(DATA_MAILBOX_BASE, 0);
    new_frame_ready = 1;
}

// ---- Image Processing Functions ----

void process_flip(uint16_t *input, uint16_t *output, int width, int height)
{
    int totalPixels = width * height;
    for (int i = 0; i < totalPixels; i++) {
        output[i] = input[totalPixels - 1 - i];
    }
}

void convolve(uint8_t *inputImage, uint8_t *outputImage, int *kernel, int width, int height) {
    for (int i = 1; i < height - 1; i++) {
        for (int j = 1; j < width - 1; j++) {
            int sum = 0;
            int totalWeight = 0;

            for (int ki = -1; ki <= 1; ki++) {
                for (int kj = -1; kj <= 1; kj++) {
                    int pixel = inputImage[(i + ki) * width + (j + kj)];
                    int weight = kernel[(ki + 1) * 3 + (kj + 1)];
                    sum += pixel * weight;
                    totalWeight += weight;
                }
            }

            if (totalWeight == 0) { totalWeight = 1; } // prevent divide-by-zero for zero-sum kernels
            int result = sum / totalWeight;
            if (result < 0) { result = 0; }
            if (result > 255) { result = 255; }
            outputImage[i * width + j] = (uint8_t)result;
        }
    }

    // Zero the border pixels that the convolution loop skips
    for (int x = 0; x < width; x++) { outputImage[x] = 0; }
    for (int x = 0; x < width; x++) { outputImage[(height - 1) * width + x] = 0; }
    for (int y = 0; y < height; y++) { outputImage[y * width] = 0; }
    for (int y = 0; y < height; y++) { outputImage[y * width + (width - 1)] = 0; }
}

// Applies a 3x3 box blur (uniform average) to a greyscale image.
// @param input   Source greyscale buffer
// @param output  Destination greyscale buffer
// @param width   Image width in pixels
// @param height  Image height in pixels
void box_blur(uint8_t *input, uint8_t *output, int width, int height) {
    int blurKernel[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
    convolve(input, output, blurKernel, width, height);
}

// Applies Sobel edge detection and thresholds the result to a binary edge map.
// @param input   Source greyscale buffer
// @param output  Destination greyscale buffer
// @param width   Image width in pixels
// @param height  Image height in pixels
void sobel_edge_detection(uint8_t *input, uint8_t *output, int width, int height) {
    int threshold = 60;

    // Go through image by rows, skipping the outer pixel border
    for (int i = 1; i < height - 1; i++) {

        // Pre-compute the memory address for the 3 rows we are currently looking at
        // This is much faster than calculating (i * width) for every single pixel
        const uint8_t *row_above = input + (i - 1) * width;
        const uint8_t *row_curr  = input + i       * width;
        const uint8_t *row_below = input + (i + 1) * width;
        uint8_t *out_row         = output + i      * width;

        // Go through the current row for each pixel
        for (int j = 1; j < width - 1; j++) {

            // Load neighbouring pixels directly from the cached row pointers
            int tl = row_above[j - 1];
            int tc = row_above[j];
            int tr = row_above[j + 1];
            int ml = row_curr [j - 1];
            int mr = row_curr [j + 1];
            int bl = row_below[j - 1];
            int bc = row_below[j];
            int br = row_below[j + 1];

            // Calculate horizontal (gx) and vertical (gy) edge strength using bit shifts (<< 1).
            // This completely unrolls the 3x3 loop and skips all zero multiplications!
            int gx = (tr - tl) + ((mr - ml) << 1) + (br - bl);
            int gy = (bl - tl) + ((bc - tc) << 1) + (br - tr);

            // Absolute value without branching (if-statements)
            int abs_gx = gx < 0 ? -gx : gx;
            int abs_gy = gy < 0 ? -gy : gy;

            // Get total edge strength and cap value at 255 to prevent overflow
            int magnitude = abs_gx + abs_gy;
            if (magnitude > 255) magnitude = 255;

            // If edge is strong, draw it. If not, set to black (0).
            out_row[j] = (magnitude >= threshold) ? (uint8_t)magnitude : 0;
        }
    }

    // Zero the border pixels
    for (int x = 0; x < width; x++)  output[x] = 0;
    for (int x = 0; x < width; x++)  output[(height - 1) * width + x] = 0;
    for (int y = 0; y < height; y++) output[y * width] = 0;
    for (int y = 0; y < height; y++) output[y * width + (width - 1)] = 0;
}

void process_rgb_blur(uint16_t* rgb_in, uint16_t* rgb_out, int width, int height) {
    int total = width * height;
    int blur_kernel[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};

    for(int i = 0; i < total; i++) {
        r_chan[i] = (rgb_in[i] >> 8) & 0xF;
        g_chan[i] = (rgb_in[i] >> 4) & 0xF;
        b_chan[i] = rgb_in[i] & 0xF;
    }

    convolve(r_chan, r_blur, blur_kernel, width, height);
    convolve(g_chan, g_blur, blur_kernel, width, height);
    convolve(b_chan, b_blur, blur_kernel, width, height);

    for(int i = 0; i < total; i++) {
        rgb_out[i] = ((r_blur[i] & 0xF) << 8) | ((g_blur[i] & 0xF) << 4) | (b_blur[i] & 0xF);
    }
}

void process_rgb_edges(uint16_t* rgb_in, uint16_t* rgb_out, int width, int height) {
    int total = width * height;

    for(int i = 0; i < total; i++) {
        uint16_t p = rgb_in[i];
        uint8_t r = ((p >> 8) & 0xF) * 17;
        uint8_t g = ((p >> 4) & 0xF) * 17;
        uint8_t b = (p & 0xF) * 17;
        grayscale_buffer[i] = (r * 76 + g * 150 + b * 29) >> 8;
    }

    sobel_edge_detection(grayscale_buffer, edge_buffer, width, height);

    for(int i = 0; i < total; i++) {
        uint8_t val = edge_buffer[i] >> 4;
        rgb_out[i] = (val << 8) | (val << 4) | val;
    }
}

int main() {
	printf("Img proc main.\n");

    int currently_displaying = -1;

    alt_ic_isr_register(
        DATA_MAILBOX_IRQ_INTERRUPT_CONTROLLER_ID,
        DATA_MAILBOX_IRQ,
        mailbox_rx_isr,
        NULL,
        NULL
    );

    IOWR(DATA_MAILBOX_BASE, 3, 0x01);

    sharedTimingData->noFilterTime = 0;
    sharedTimingData->flipTime = 0;
    sharedTimingData->blurTime = 0;
    sharedTimingData->sobelTime = 0;

	printf("Img proc init.\n");

    while(1) {
    	// Busy wait until a new frame is ready
    	while (!new_frame_ready);
    	// Immediately update frame ready flag
    	new_frame_ready = 0;

		if (currently_displaying != -1) {
			while (IORD(ACK_MAILBOX_BASE, 2) & 0x02);
			IOWR(ACK_MAILBOX_BASE, 0, currently_displaying);
		}

		currently_displaying = current_frame_index;

		// Get source buffer (uncached)
		uint8_t* source = (uint8_t*)(buffers[currently_displaying]);
		uint16_t *processing_buffer = processing_buffers[bufToSend];

//    	printf("Img proc received frame, address: %d\n", (int)source);

		uint8_t isQuad = shared_display->isQuad;
		int processMode = (IORD(SW_BASE, 0) >> 1) & 0x3;  // SW[2:1]

        // ALWAYS unpack the full frame from the camera
        int num_pixels = IMAGE_SIZE;

        // Unpack Packed RGB Data via Nibble Shuffle
        int byte_idx = 0;
        for (int i = 0; i < num_pixels; i += 2) {
            uint8_t b0 = source[byte_idx++];
            uint8_t b1 = source[byte_idx++];
            uint8_t b2 = source[byte_idx++];

            uint16_t p0_raw = (b0 << 4) | (b1 >> 4);
            uint16_t p1_raw = ((b1 & 0x0F) << 8) | b2;

            // FIX: Swapped assignments ([i+1] first) to fix the jagged zipper effect
            unpacked_rgb[i+1] = (((p0_raw >> 4) & 0xF) << 8) | ((p0_raw & 0xF) << 4) | ((p0_raw >> 8) & 0xF);
            unpacked_rgb[i]   = (((p1_raw >> 4) & 0xF) << 8) | ((p1_raw & 0xF) << 4) | ((p1_raw >> 8) & 0xF);
        }

		if (isQuad)
		{
			// Quad mode: one quarter image received, process 4 ways

			// Slot 0: raw
			uint32_t noFilterBeginTime = IORD(USEC_COUNTER_BASE, 0);

            // SOFTWARE DOWNSAMPLING: Shrink 320x240 to 160x120 by skipping every other pixel
            uint16_t* quad_source = processing_buffer; // Write directly into Slot 0
            for (int y = 0; y < QUAD_IMAGE_HEIGHT; y++) {
                for (int x = 0; x < QUAD_IMAGE_WIDTH; x++) {
                    quad_source[y * QUAD_IMAGE_WIDTH + x] = unpacked_rgb[(y * 2) * IMAGE_WIDTH + (x * 2)];
                }
            }

			uint32_t noFilterEndTime = IORD(USEC_COUNTER_BASE, 0);
			sharedTimingData->noFilterTime = noFilterEndTime - noFilterBeginTime;

			// Slot 1: flip
			uint32_t flipBeginTime = IORD(USEC_COUNTER_BASE, 0);
			process_flip(quad_source,
						 processing_buffer + QUAD_IMAGE_SIZE,
						 QUAD_IMAGE_WIDTH, QUAD_IMAGE_HEIGHT);
			uint32_t flipEndTime = IORD(USEC_COUNTER_BASE, 0);
			sharedTimingData->flipTime = flipEndTime - flipBeginTime;

			// Slot 2: blur
			uint32_t blurBeginTime = IORD(USEC_COUNTER_BASE, 0);
			process_rgb_blur(quad_source,
					 processing_buffer + 2 * QUAD_IMAGE_SIZE,
					 QUAD_IMAGE_WIDTH, QUAD_IMAGE_HEIGHT);
			uint32_t blurEndTime = IORD(USEC_COUNTER_BASE, 0);
			sharedTimingData->blurTime = blurEndTime - blurBeginTime;

			// Slot 3: edge
			uint32_t sobelBeginTime = IORD(USEC_COUNTER_BASE, 0);
			process_rgb_edges(quad_source,
								 processing_buffer + 3 * QUAD_IMAGE_SIZE,
								 QUAD_IMAGE_WIDTH, QUAD_IMAGE_HEIGHT);
			uint32_t sobelEndTime = IORD(USEC_COUNTER_BASE, 0);
			sharedTimingData->sobelTime = sobelEndTime - sobelBeginTime;
		}
		else
		{
			// Full mode: SW[2:1] selects processing
			uint32_t fullFilterBeginTime = IORD(USEC_COUNTER_BASE, 0);
			switch (processMode) {
				case PROC_FLIP:
					process_flip(unpacked_rgb, processing_buffer,
								 IMAGE_WIDTH, IMAGE_HEIGHT);
					break;
				case PROC_BLUR:
					process_rgb_blur(unpacked_rgb, processing_buffer,
							 IMAGE_WIDTH, IMAGE_HEIGHT);
					break;
				case PROC_EDGE:
					process_rgb_edges(unpacked_rgb, processing_buffer,
										 IMAGE_WIDTH, IMAGE_HEIGHT);
					break;
				default: // PROC_RAW
//					printf("No filter, copy %d to %d\n", (int)source, (int)processing_buffer);
					for (size_t i = 0; i < IMAGE_SIZE; i++)
					{
						processing_buffer[i] = unpacked_rgb[i];
					}
					break;
			}
			uint32_t fullFilterEndTime = IORD(USEC_COUNTER_BASE, 0);

			sharedTimingData->noFilterTime = 0;
			sharedTimingData->flipTime = 0;
			sharedTimingData->blurTime = 0;
			sharedTimingData->sobelTime = 0;

			uint32_t fullFilterTime = fullFilterEndTime - fullFilterBeginTime;
			switch (processMode)
			{
			case PROC_RAW:
				sharedTimingData->noFilterTime = fullFilterTime;
				break;
			case PROC_FLIP:
				sharedTimingData->flipTime = fullFilterTime;
				break;
			case PROC_BLUR:
				sharedTimingData->blurTime = fullFilterTime;
				break;
			case PROC_EDGE:
				sharedTimingData->sobelTime = fullFilterTime;
				break;
			}
		}

//		printf("Img proc passing frame to display, proc buffer: %d\n", (int)processing_buffer);
//		printf("Pixel value at 1 0 source: %d\n", (int)(*(source + 1)));
//		printf("Pixel value at 1 0 IORD: %d\n", (int)((IORD(source, 0) >> 8) & 0xff));
//		printf("Pixel value at 1 0 dest: %d\n", (int)(*(processing_buffer + 1)));

		// Signal to display proc that the next frame is ready
		while (IORD(DISPLAY_FRAME_MAILBOX_BASE, 2) & 0x2);
		IOWR(DISPLAY_FRAME_MAILBOX_BASE, 0, bufToSend);

		bufToSend ^= 0x1;
    }

    return 0;
}
