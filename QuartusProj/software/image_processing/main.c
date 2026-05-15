#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "system.h"
#include "io.h"
#include "sys/alt_irq.h"
#include <stdlib.h>

// Benchmarking: print averages every N frames to reduce JTAG UART overhead
#define BENCH_INTERVAL 20

#include "common_defs.h"
#include "memory_addresses.h"

// Processing modes (selected by SW[2:1])
#define PROC_RAW   0
#define PROC_FLIP  1
#define PROC_BLUR  2
#define PROC_EDGE  3

uint8_t* buffers[3] = {
    (uint8_t*)(IMAGE_READ_BUF_A),
    (uint8_t*)(IMAGE_READ_BUF_B),
    (uint8_t*)(IMAGE_READ_BUF_C)
};

SharedAccelData* shared_accel = (SharedAccelData*)(SHARED_ACCEL_DATA);
SharedDisplayState* shared_display = (SharedDisplayState*)((SHARED_MEM_BASE + (3 * IMAGE_SIZE) + sizeof(SharedAccelData) + 8) | 0x80000000);

// Processing output buffer � avoids writing into shared triple buffers
uint8_t processing_buffer[IMAGE_SIZE];

volatile int new_frame_ready = 0;
volatile uint32_t current_frame_index = 0;

static void mailbox_rx_isr(void* context) {
    current_frame_index = IORD(DATA_MAILBOX_BASE, 0);
    new_frame_ready = 1;
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

// ---- Image Processing Functions ----

void process_flip(uint8_t *input, uint8_t *output, int width, int height, int bpp)
{
    int totalPixels = width * height;
    for (int i = 0; i < totalPixels; i++) {
        int srcIdx = (totalPixels - 1 - i) * bpp;
        int dstIdx = i * bpp;
        for (int b = 0; b < bpp; b++) {
            output[dstIdx + b] = input[srcIdx + b];
        }
    }
}

void convolve(uint8_t *inputImage, uint8_t *outputImage, int *kernel, int width, int height) {
	// iterate through every row and column skipping the borders
	for (int i = 1; i < height - 1; i++) {
		for (int j = 1; j < width - 1; j++) {
			// get the weighted pixel sum and kernel weight (normalisation)
			int sum = 0;
			int total_weight = 0;

			// looping over 3x3 neighbourhood around pixel (i,j)
			for (int ki = -1; ki <= 1; ki++) {
				for (int kj = -1; kj <= 1; kj++) {
					// get neighbouring pixel value
					int pixel = inputImage[(i + ki) * width + (j + kj)];
					// get matching kernel weight (stored in row-major index)
					int weight = kernel[(ki + 1) * 3 + (kj + 1)];
					sum += pixel * weight;
					total_weight += weight;
				}
			}
			// avoid dividing by zero if kernel weights = 0
			if (total_weight == 0) total_weight = 1;
			// normalise
			int result = sum / total_weight;
			// restrict result to valid pixel range [0, 255]
			if (result < 0) result = 0;
			if (result > 255) result = 255;
			// write the result to the output buffer
			outputImage[i * width + j] = (uint8_t)result;
		}
	}
	// border pixels zero, skipped by convolution loop, setting them to black
	for (int x = 0; x < width; x++) outputImage[x] = 0;
	for (int x = 0; x < width; x++) outputImage[(height-1)*width + x] = 0;
	for (int y = 0; y < height; y++) outputImage[y*width] = 0;
	for (int y = 0; y < height; y++) outputImage[y*width + (width - 1)] = 0;
}

void box_blur(uint8_t *input, uint8_t *output, int width, int height) {
	// 3x3 box kernel : all values = 1
	// convolve will divide by total_weight = 9, giving average
	int blur_kernel[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};

	// run a pass of convolution with box kernel
	convolve(input, output, blur_kernel, width, height);
}

void sobel_edge_detection(uint8_t *input, uint8_t *output, int width, int height) {
	// detecting vertical (Gx) and horizontal (Gy) edges
	int kernel_x[9] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
	int kernel_y[9] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};

	int threshold = 60;

	for (int i = 1; i < height - 1; i++) {
		for (int j = 1; j < width -1; j++) {

			int gx = 0;
			int gy = 0;

			for (int ki = -1; ki <= 1; ki++) {
				for (int kj = -1; kj <= 1; kj++) {
					int pixel = input[(i + ki) * width + (j + kj)];
					gx += pixel * kernel_x[(ki+1)*3 + (kj+1)];
					gy += pixel * kernel_y[(ki+1)*3 + (kj+1)];
				}
			}

			int magnitude = (gx < 0 ? -gx : gx) + (gy < 0 ? -gy : gy);
			if (magnitude > 255) magnitude = 255;
			output[i * width + j] = (magnitude >= threshold) ? (uint8_t)magnitude : 0;
		}
	}

	// zero border pixels
	for (int x = 0; x < width; x++) output[x] = 0;
	for (int x = 0; x < width; x++) output[(height -1)*width + x] = 0;
	for (int y = 0; y < height; y++) output[y*width] = 0;
	for (int y = 0; y < height; y++) output[y*width + (width-1)] = 0;
}

// ---- Display Functions ----

// Write a full image to pixel buffer using optimized 32-bit reads
void display_full_image(uint8_t *buffer)
{
    volatile int* const pixel_dat_ptr = (volatile int*)(PIXEL_DAT_BASE | 0x80000000);
    volatile int* const img_addy_ptr  = (volatile int*)(IMG_ADDY_BASE  | 0x80000000);

    uint32_t* src_ptr32 = (uint32_t*)((uint32_t)buffer | 0x80000000);
    uint32_t* const src_end32 = (uint32_t*)(((uint32_t)buffer + IMAGE_SIZE) | 0x80000000);

    int addr = 0;

    while (src_ptr32 < src_end32) {
        uint32_t block = *src_ptr32++;

        *img_addy_ptr = addr++;
        *pixel_dat_ptr = (block & 0xFF) >> 4;

        *img_addy_ptr = addr++;
        *pixel_dat_ptr = ((block >> 8) & 0xFF) >> 4;

        *img_addy_ptr = addr++;
        *pixel_dat_ptr = ((block >> 16) & 0xFF) >> 4;

        *img_addy_ptr = addr++;
        *pixel_dat_ptr = ((block >> 24) & 0xFF) >> 4;
    }
}

// Write a quarter image to a specific quadrant of the pixel buffer
void display_quad_image(uint8_t *buffer, uint32_t imageIndex, uint32_t displayIndex)
{
    volatile int* const pixel_dat_ptr = (volatile int*)(PIXEL_DAT_BASE | 0x80000000);
    volatile int* const img_addy_ptr  = (volatile int*)(IMG_ADDY_BASE  | 0x80000000);

    uint8_t* src = (uint8_t*)((uint32_t)buffer | 0x80000000);

    uint32_t imgAddr = imageIndex * QUAD_IMAGE_SIZE;
    uint32_t pixelBufferAddr = 0;

    if (displayIndex & 0x1)
        pixelBufferAddr += QUAD_IMAGE_WIDTH;
    if (displayIndex & 0x2)
        pixelBufferAddr += QUAD_IMAGE_SIZE * 2;

    for (uint32_t i = 0; i < QUAD_IMAGE_HEIGHT; i++) {
        for (uint32_t j = 0; j < QUAD_IMAGE_WIDTH; j++) {
            *img_addy_ptr  = pixelBufferAddr;
            *pixel_dat_ptr = src[imgAddr] >> 4;
            imgAddr++;
            pixelBufferAddr++;
        }
        pixelBufferAddr += IMAGE_WIDTH - QUAD_IMAGE_WIDTH;
    }
}


int main() {
    int currently_displaying = -1;
    uint32_t last_frame_time = 0;

    // ---- Benchmarking accumulators ----
    // Accumulate over BENCH_INTERVAL frames, then print averages
    uint32_t bench_count  = 0;
    uint32_t sum_total    = 0;   // processing + display combined
    uint32_t sum_process  = 0;   // processing only
    uint32_t sum_display  = 0;   // display only
    uint32_t sum_frame    = 0;   // frame-to-frame interval
    // Quad-mode per-step breakdown
    uint32_t sum_flip = 0;
    uint32_t sum_blur = 0;
    uint32_t sum_edge = 0;

    printf("Image Processing Core started\n");
    printf("Benchmarking enabled: printing every %d frames\n\n", BENCH_INTERVAL);

    alt_ic_isr_register(
        DATA_MAILBOX_IRQ_INTERRUPT_CONTROLLER_ID,
        DATA_MAILBOX_IRQ,
        mailbox_rx_isr,
        NULL,
        NULL
    );

    IOWR(DATA_MAILBOX_BASE, 3, 0x01);

    while(1) {
        // Update shared quad mode flag from switches each iteration
        // Core 0 reads this to know what frame size to request
        shared_display->isQuad = (IORD(SW_BASE, 0) & 0x1) ? 1 : 0;

        if (new_frame_ready) {
            new_frame_ready = 0;

            // ---- Frame-to-frame timing ----
            uint32_t current_time = IORD(USEC_COUNTER_BASE, 0);
            uint32_t frameTime = 0;
            if (last_frame_time != 0) {
                frameTime = current_time - last_frame_time;
                display_fps(frameTime);
                sum_frame += frameTime;
            }
            last_frame_time = current_time;

            if (currently_displaying != -1) {
                while (IORD(ACK_MAILBOX_BASE, 2) & 0x02);
                IOWR(ACK_MAILBOX_BASE, 0, currently_displaying);
            }

            currently_displaying = current_frame_index;

            // Get source buffer (uncached)
            uint8_t* source = (uint8_t*)((uint32_t)buffers[currently_displaying] | 0x80000000);

            uint8_t isQuad = shared_display->isQuad;
            int processMode = (IORD(SW_BASE, 0) >> 1) & 0x3;  // SW[2:1]

            // ---- Timing variables ----
            uint32_t t_total_start, t_step, t_disp_start;
            uint32_t total_time = 0;
            uint32_t proc_time  = 0;
            uint32_t disp_time  = 0;
            uint32_t flip_time = 0, blur_time = 0, edge_time = 0;

            if (isQuad)
            {
                // ================================================
                // QUAD MODE: process all 4 images then display all
                // Measures: total time from start of processing
                //           to end of pixel buffer writes
                // ================================================
                t_total_start = IORD(USEC_COUNTER_BASE, 0);

                // Slot 0: raw (memcpy)
                memcpy(processing_buffer, source, QUAD_IMAGE_SIZE);

                // Slot 1: flip
                t_step = IORD(USEC_COUNTER_BASE, 0);
                process_flip(source,
                             processing_buffer + QUAD_IMAGE_SIZE,
                             QUAD_IMAGE_WIDTH, QUAD_IMAGE_HEIGHT, 1);
                flip_time = IORD(USEC_COUNTER_BASE, 0) - t_step;

                // Slot 2: blur
                t_step = IORD(USEC_COUNTER_BASE, 0);
                box_blur(source,
                         processing_buffer + 2 * QUAD_IMAGE_SIZE,
                         QUAD_IMAGE_WIDTH, QUAD_IMAGE_HEIGHT);
                blur_time = IORD(USEC_COUNTER_BASE, 0) - t_step;

                // Slot 3: edge
                t_step = IORD(USEC_COUNTER_BASE, 0);
                sobel_edge_detection(source,
                                     processing_buffer + 3 * QUAD_IMAGE_SIZE,
                                     QUAD_IMAGE_WIDTH, QUAD_IMAGE_HEIGHT);
                edge_time = IORD(USEC_COUNTER_BASE, 0) - t_step;

                proc_time = IORD(USEC_COUNTER_BASE, 0) - t_total_start;

                // Display all 4 quadrants
                t_disp_start = IORD(USEC_COUNTER_BASE, 0);
                display_quad_image(processing_buffer, shared_display->quadDisplayIndices[0], 0);
                display_quad_image(processing_buffer, shared_display->quadDisplayIndices[1], 1);
                display_quad_image(processing_buffer, shared_display->quadDisplayIndices[2], 2);
                display_quad_image(processing_buffer, shared_display->quadDisplayIndices[3], 3);
                disp_time = IORD(USEC_COUNTER_BASE, 0) - t_disp_start;

                total_time = IORD(USEC_COUNTER_BASE, 0) - t_total_start;

                sum_flip += flip_time;
                sum_blur += blur_time;
                sum_edge += edge_time;
            }
            else
            {
                // ================================================
                // SINGLE MODE: process one image then display it
                // Measures: total time from start of processing
                //           to end of pixel buffer write
                // ================================================
                t_total_start = IORD(USEC_COUNTER_BASE, 0);

                switch (processMode) {
                    case PROC_FLIP:
                        process_flip(source, processing_buffer,
                                     IMAGE_WIDTH, IMAGE_HEIGHT, 1);
                        proc_time = IORD(USEC_COUNTER_BASE, 0) - t_total_start;
                        t_disp_start = IORD(USEC_COUNTER_BASE, 0);
                        display_full_image(processing_buffer);
                        disp_time = IORD(USEC_COUNTER_BASE, 0) - t_disp_start;
                        break;
                    case PROC_BLUR:
                        box_blur(source, processing_buffer,
                                 IMAGE_WIDTH, IMAGE_HEIGHT);
                        proc_time = IORD(USEC_COUNTER_BASE, 0) - t_total_start;
                        t_disp_start = IORD(USEC_COUNTER_BASE, 0);
                        display_full_image(processing_buffer);
                        disp_time = IORD(USEC_COUNTER_BASE, 0) - t_disp_start;
                        break;
                    case PROC_EDGE:
                        sobel_edge_detection(source, processing_buffer,
                                             IMAGE_WIDTH, IMAGE_HEIGHT);
                        proc_time = IORD(USEC_COUNTER_BASE, 0) - t_total_start;
                        t_disp_start = IORD(USEC_COUNTER_BASE, 0);
                        display_full_image(processing_buffer);
                        disp_time = IORD(USEC_COUNTER_BASE, 0) - t_disp_start;
                        break;
                    default: // PROC_RAW
                        proc_time = 0;
                        t_disp_start = IORD(USEC_COUNTER_BASE, 0);
                        display_full_image(source);
                        disp_time = IORD(USEC_COUNTER_BASE, 0) - t_disp_start;
                        break;
                }

                total_time = IORD(USEC_COUNTER_BASE, 0) - t_total_start;
            }

            // ---- Accumulate benchmarking data ----
            sum_total   += total_time;
            sum_process += proc_time;
            sum_display += disp_time;
            bench_count++;

            // ---- Print averages every BENCH_INTERVAL frames ----
            if (bench_count >= BENCH_INTERVAL) {
                uint32_t avg_total = sum_total   / bench_count;
                uint32_t avg_proc  = sum_process / bench_count;
                uint32_t avg_disp  = sum_display / bench_count;
                uint32_t avg_frame = sum_frame   / bench_count;

                // Calculate FPS from the total processing+display time
                uint32_t fps_whole = avg_total > 0 ? 1000000 / avg_total : 0;
                uint32_t fps_frac  = avg_total > 0 ? (100000000 / avg_total) % 100 : 0;

                if (isQuad) {
                    uint32_t avg_flip = sum_flip / bench_count;
                    uint32_t avg_blur = sum_blur / bench_count;
                    uint32_t avg_edge = sum_edge / bench_count;

                    printf("===================================================\n");
                    printf("  Display 4 video frames in quad-image mode\n");
                    printf("===================================================\n");
                    printf("  Per-step breakdown:\n");
                    printf("    Flip:         %lu us\n", avg_flip);
                    printf("    Blur:         %lu us\n", avg_blur);
                    printf("    Edge:         %lu us\n", avg_edge);
                    printf("  Processing:     %lu us\n", avg_proc);
                    printf("  Display:        %lu us\n", avg_disp);
                    printf("  -------------------------------------------\n");
                    printf("  Total time:     %lu us\n", avg_total);
                    printf("  FPS:            %lu.%02lu\n", fps_whole, fps_frac);
                    printf("  Frame interval: %lu us\n\n", avg_frame);
                } else {
                    const char* task_desc;
                    switch (processMode) {
                        case PROC_FLIP: task_desc = "Display a flipped video frame in single-image mode"; break;
                        case PROC_BLUR: task_desc = "Display a blurred video frame in single-image mode"; break;
                        case PROC_EDGE: task_desc = "Display an edge-detected video frame in single-image mode"; break;
                        default:        task_desc = "Display an unaltered video frame in single-image mode"; break;
                    }

                    printf("===================================================\n");
                    printf("  %s\n", task_desc);
                    printf("===================================================\n");
                    printf("  Processing:     %lu us\n", avg_proc);
                    printf("  Display:        %lu us\n", avg_disp);
                    printf("  -------------------------------------------\n");
                    printf("  Total time:     %lu us\n", avg_total);
                    printf("  FPS:            %lu.%02lu\n", fps_whole, fps_frac);
                    printf("  Frame interval: %lu us\n\n", avg_frame);
                }

                // Reset accumulators
                bench_count = 0;
                sum_total   = 0;
                sum_process = 0;
                sum_display = 0;
                sum_frame   = 0;
                sum_flip    = 0;
                sum_blur    = 0;
                sum_edge    = 0;
            }
        }
    }
    return 0;
}
