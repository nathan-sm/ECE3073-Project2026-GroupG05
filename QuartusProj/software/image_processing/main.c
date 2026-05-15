// Image processing Nios II core — receives frame buffer tokens from the communications
// core, applies the selected processing mode, and writes pixels to the VGA pixel buffer.
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

// Processing output buffer � avoids writing into shared triple buffers
uint8_t *processing_buffers[] = {
		(uint8_t*)(PROCESSED_IMG_BUF_A),
		(uint8_t*)(PROCESSED_IMG_BUF_B)
};

volatile int newFrameReady = 0;
volatile uint32_t currentFrameIndex = 0;

uint32_t bufToSend = 0;

// ISR: called when the communications core deposits a new frame buffer token
static void mailbox_rx_isr(void* context) {
    currentFrameIndex = IORD(DATA_MAILBOX_BASE, 0);
    newFrameReady = 1;
}

// ---- Image Processing Functions ----

// Flips the image 180 degrees (reverses pixel order).
// @param input   Source image buffer
// @param output  Destination image buffer
// @param width   Image width in pixels
// @param height  Image height in pixels
// @param bpp     Bytes per pixel
void process_flip(uint8_t *input, uint8_t *output, int width, int height, int bpp) {
    int totalPixels = width * height;
    for (int i = 0; i < totalPixels; i++) {
        int srcIdx = (totalPixels - 1 - i) * bpp;
        int dstIdx = i * bpp;
        for (int b = 0; b < bpp; b++) {
            output[dstIdx + b] = input[srcIdx + b];
        }
    }
}

// Applies a 3x3 convolution kernel to a greyscale image, skipping the 1-pixel border.
// @param inputImage   Source greyscale buffer
// @param outputImage  Destination greyscale buffer (border pixels are zeroed)
// @param kernel       3x3 kernel stored in row-major order
// @param width        Image width in pixels
// @param height       Image height in pixels
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
    int kernelX[9] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
    int kernelY[9] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};

    for (int i = 1; i < height - 1; i++) {
        for (int j = 1; j < width - 1; j++) {
            int gx = 0;
            int gy = 0;

            for (int ki = -1; ki <= 1; ki++) {
                for (int kj = -1; kj <= 1; kj++) {
                    int pixel = input[(i + ki) * width + (j + kj)];
                    gx += pixel * kernelX[(ki + 1) * 3 + (kj + 1)];
                    gy += pixel * kernelY[(ki + 1) * 3 + (kj + 1)];
                }
            }

            int magnitude = (gx < 0 ? -gx : gx) + (gy < 0 ? -gy : gy);
            if (magnitude > 255) { magnitude = 255; }
            output[i * width + j] = (magnitude >= SOBEL_EDGE_THRESHOLD) ? (uint8_t)magnitude : 0;
        }
    }

    // Zero the border pixels that the loop skips
    for (int x = 0; x < width; x++) { output[x] = 0; }
    for (int x = 0; x < width; x++) { output[(height - 1) * width + x] = 0; }
    for (int y = 0; y < height; y++) { output[y * width] = 0; }
    for (int y = 0; y < height; y++) { output[y * width + (width - 1)] = 0; }
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

    IOWR(DATA_MAILBOX_BASE, 3, MAILBOX_IRQ_ENABLE);

	printf("Img proc init.\n");

    while(1) {
    	// Busy wait until a new frame is ready
    	while (!newFrameReady);
    	// Immediately update frame ready flag
    	newFrameReady = 0;

		if (currently_displaying != -1) {
			while (IORD(ACK_MAILBOX_BASE, 2) & 0x02);
			IOWR(ACK_MAILBOX_BASE, 0, currently_displaying);
		}

		currently_displaying = currentFrameIndex;

		// Get source buffer (uncached)
		uint8_t* source = (uint8_t*)(buffers[currently_displaying]);
		uint8_t *processing_buffer = processing_buffers[bufToSend];

//    	printf("Img proc received frame, address: %d\n", (int)source);

		uint8_t isQuad = shared_display->isQuad;
		int processMode = (IORD(SW_BASE, 0) >> 1) & 0x3;  // SW[2:1]

		if (isQuad)
		{
			// Quad mode: one quarter image received, process 4 ways

			// Slot 0: raw
			for (size_t i = 0; i < QUAD_IMAGE_SIZE; i++)
			{
				processing_buffer[i] = source[i];
			}

			// Slot 1: flip
			process_flip(source,
						 processing_buffer + QUAD_IMAGE_SIZE,
						 QUAD_IMAGE_WIDTH, QUAD_IMAGE_HEIGHT, 1);

			// Slot 2: blur
			box_blur(source,
					 processing_buffer + 2 * QUAD_IMAGE_SIZE,
					 QUAD_IMAGE_WIDTH, QUAD_IMAGE_HEIGHT);

			// Slot 3: edge
			sobel_edge_detection(source,
								 processing_buffer + 3 * QUAD_IMAGE_SIZE,
								 QUAD_IMAGE_WIDTH, QUAD_IMAGE_HEIGHT);
		}
		else
		{
			// Full mode: SW[2:1] selects processing
			switch (processMode) {
				case PROC_FLIP:
					process_flip(source, processing_buffer,
								 IMAGE_WIDTH, IMAGE_HEIGHT, 1);
					break;
				case PROC_BLUR:
					box_blur(source, processing_buffer,
							 IMAGE_WIDTH, IMAGE_HEIGHT);
					break;
				case PROC_EDGE:
					sobel_edge_detection(source, processing_buffer,
										 IMAGE_WIDTH, IMAGE_HEIGHT);
					break;
				default: // PROC_RAW
//					printf("No filter, copy %d to %d\n", (int)source, (int)processing_buffer);
					for (size_t i = 0; i < IMAGE_SIZE; i++)
					{
						processing_buffer[i] = source[i];
					}
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
