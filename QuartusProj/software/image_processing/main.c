// Image processing Nios II core — receives frame buffer tokens from the communications
// core, applies the selected processing mode, and writes pixels to the VGA pixel buffer.
// Created By: Nathan Morris       (32532601)
//             Ritwam Shohaum      (33156816)
//             Shuk Kan LUI        (33891885)
//             Evgeny Solomin      (34977260)
// Created Date: 2026
// version = '1.0'

#include <stdint.h>
#include <string.h>
#include "system.h"
#include "io.h"
#include "sys/alt_irq.h"
#include <stdlib.h>
#include <stdio.h>


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

SharedAccelData* sharedAccel = (SharedAccelData*)(SHARED_ACCEL_DATA);
SharedDisplayState* sharedDisplay = (SharedDisplayState*)(SHARED_DISPLAY_STATE | UNCACHED_MEM_MASK);

// Scratch buffer for processing output — avoids writing into the shared triple buffers
uint8_t processingBuffer[IMAGE_SIZE];

volatile int newFrameReady = 0;
volatile uint32_t currentFrameIndex = 0;

// ISR: called when the communications core deposits a new frame buffer token
static void mailbox_rx_isr(void* context) {
    currentFrameIndex = IORD(DATA_MAILBOX_BASE, 0);
    newFrameReady = 1;
}

// Writes the current FPS (derived from elapsed microseconds) to the 7-segment displays.
// @param elapsed Microseconds elapsed since the previous frame was delivered
void display_fps(uint32_t elapsed) {
    if (elapsed == 0) { return; } // prevent divide-by-zero on the first frame

    // Store 100x FPS so we can use integer arithmetic for the decimal digit
    uint32_t fps100 = 100000000 / elapsed;

    const uint8_t SEG[10] = {
        0xC0, 0xF9, 0xA4, 0xB0, 0x99,
        0x92, 0x82, 0xF8, 0x80, 0x90
    };

    int d3 = (fps100 / 1000) % 10;
    int d2 = (fps100 / 100)  % 10;
    int d1 = (fps100 / 10)   % 10;
    int d0 =  fps100         % 10;

    uint8_t hex3 = SEG[d3];
    uint8_t hex2 = SEG[d2] & ~(1 << 7); // decimal point on HEX2
    uint8_t hex1 = SEG[d1];
    uint8_t hex0 = SEG[d0];

    uint32_t hex02 = ((uint32_t)hex2 << 16) | ((uint32_t)hex1 << 8) | hex0;
    uint32_t hex35 = ((uint32_t)HEX_BLANK << 16) | ((uint32_t)HEX_BLANK << 8) | hex3;

    IOWR(HEX20_BASE, 0, hex02);
    IOWR(HEX53_BASE, 0, hex35);
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
    // go through image by rows
    // skip the outer pixel border to avoid reading memory outside the image
    for (int i = 1; i < height - 1; i++) {
        // find the memory address for the 3 rows we are currently looking at
        // faster than calculating every pixel
        const uint8_t *row_above = input + (i - 1) * width;
        const uint8_t *row_curr  = input + i       * width;
        const uint8_t *row_below = input + (i + 1) * width;
        uint8_t *out_row   = output + i * width;

        // go through the current row fir each pixel
        for (int j = 1; j < width - 1; j++) {
            // get neighbouring pixels, faster than 3x3 looping
            int tl = row_above[j - 1];
            int tc = row_above[j];
            int tr = row_above[j + 1];
            int ml = row_curr [j - 1];
            int mr = row_curr [j + 1];
            int bl = row_below[j - 1];
            int bc = row_below[j];
            int br = row_below[j + 1];

            // calc. horizontal and vertical (gx and gy) edge strength using bit shift
            // faster than using multiplication
            int gx = (tr - tl) + ((mr - ml) << 1) + (br - bl);
            int gy = (bl - tl) + ((bc - tc) << 1) + (br - tr);

            // turn negative numbers to postive
            int abs_gx = gx < 0 ? -gx : gx;
            int abs_gy = gy < 0 ? -gy : gy;

            // get total edge strength and cap value at 255 preventing overflow
            int magnitude = abs_gx + abs_gy;
            if (magnitude > 255) magnitude = 255;

            // if edge is strong draw it if not set to black (USING TEAMMATE's NEW MACRO)
            out_row[j] = (magnitude >= SOBEL_EDGE_THRESHOLD) ? (uint8_t)magnitude : 0;
        }
    }
    // zero border pixels
    for (int x = 0; x < width; x++)  output[x] = 0;
    for (int x = 0; x < width; x++)  output[(height - 1) * width + x] = 0;
    for (int y = 0; y < height; y++) output[y * width] = 0;
    for (int y = 0; y < height; y++) output[y * width + (width - 1)] = 0;
}
// ---- Display Functions ----

// Writes a full 320x240 image to the pixel buffer using 32-bit reads for efficiency.
// @param buffer Pointer to the source greyscale image (IMAGE_SIZE bytes)
void display_full_image(uint8_t *buffer) {
    volatile int* const pixelDatPtr = (volatile int*)(PIXEL_DAT_BASE | UNCACHED_MEM_MASK);
    volatile int* const imgAddyPtr  = (volatile int*)(IMG_ADDY_BASE  | UNCACHED_MEM_MASK);

    uint32_t* srcPtr32 = (uint32_t*)((uint32_t)buffer | UNCACHED_MEM_MASK);
    uint32_t* const srcEnd32 = (uint32_t*)(((uint32_t)buffer + IMAGE_SIZE) | UNCACHED_MEM_MASK);

    int addr = 0;

    while (srcPtr32 < srcEnd32) {
        uint32_t block = *srcPtr32++;

        *imgAddyPtr = addr++;
        *pixelDatPtr = (block & 0xFF) >> 4;

        *imgAddyPtr = addr++;
        *pixelDatPtr = ((block >> 8) & 0xFF) >> 4;

        *imgAddyPtr = addr++;
        *pixelDatPtr = ((block >> 16) & 0xFF) >> 4;

        *imgAddyPtr = addr++;
        *pixelDatPtr = ((block >> 24) & 0xFF) >> 4;
    }
}

// Writes one quarter-size image to a specific quadrant of the pixel buffer.
// @param buffer       Pointer to a buffer holding 4 packed quarter images
// @param imageIndex   Which of the 4 sub-images to use as the source (0–3)
// @param displayIndex Which screen quadrant to target (0=TL, 1=TR, 2=BL, 3=BR)
void display_quad_image(uint8_t *buffer, uint32_t imageIndex, uint32_t displayIndex) {
    volatile int* const pixelDatPtr = (volatile int*)(PIXEL_DAT_BASE | UNCACHED_MEM_MASK);
    volatile int* const imgAddyPtr  = (volatile int*)(IMG_ADDY_BASE  | UNCACHED_MEM_MASK);

    uint8_t* src = (uint8_t*)((uint32_t)buffer | UNCACHED_MEM_MASK);

    uint32_t imgAddr = imageIndex * QUAD_IMAGE_SIZE;
    uint32_t pixelBufferAddr = 0;

    if (displayIndex & 0x1) { pixelBufferAddr += QUAD_IMAGE_WIDTH; }
    if (displayIndex & 0x2) { pixelBufferAddr += QUAD_IMAGE_SIZE * 2; }

    for (uint32_t i = 0; i < QUAD_IMAGE_HEIGHT; i++) {
        for (uint32_t j = 0; j < QUAD_IMAGE_WIDTH; j++) {
            *imgAddyPtr  = pixelBufferAddr;
            *pixelDatPtr = src[imgAddr] >> 4;
            imgAddr++;
            pixelBufferAddr++;
        }
        pixelBufferAddr += IMAGE_WIDTH - QUAD_IMAGE_WIDTH;
    }
}

// Main loop: waits for a frame token from the communications core, applies the
// selected processing mode, and writes the result to the VGA pixel buffer.
// @return 0 (loop runs indefinitely)
int main() {
    int currentlyDisplaying = -1;
    uint32_t lastFrameTime = 0;

    alt_ic_isr_register(
        DATA_MAILBOX_IRQ_INTERRUPT_CONTROLLER_ID,
        DATA_MAILBOX_IRQ,
        mailbox_rx_isr,
        NULL,
        NULL
    );

    IOWR(DATA_MAILBOX_BASE, 3, MAILBOX_IRQ_ENABLE);

    while (1) {
        // Update the shared quad-mode flag from the switches on each iteration
        sharedDisplay->isQuad = (IORD(SW_BASE, 0) & 0x1) ? 1 : 0;

        if (newFrameReady) {
            newFrameReady = 0;

            uint32_t currentTime = IORD(USEC_COUNTER_BASE, 0);
            if (lastFrameTime != 0) {
                uint32_t frameTime = currentTime - lastFrameTime;
                display_fps(frameTime);
            }
            lastFrameTime = currentTime;

            if (currentlyDisplaying != -1) {
                while (IORD(ACK_MAILBOX_BASE, 2) & MAILBOX_STATUS_FULL);
                IOWR(ACK_MAILBOX_BASE, 0, currentlyDisplaying);
            }

            currentlyDisplaying = currentFrameIndex;

            uint8_t* source = (uint8_t*)((uint32_t)buffers[currentlyDisplaying] | UNCACHED_MEM_MASK);

            uint8_t isQuad = sharedDisplay->isQuad;
            int processMode = (IORD(SW_BASE, 0) >> 1) & 0x3; // SW[2:1]

            if (isQuad) {
                // Quad mode: one quarter image received, process into 4 output slots
                memcpy(processingBuffer, source, QUAD_IMAGE_SIZE);

                process_flip(source,
                             processingBuffer + QUAD_IMAGE_SIZE,
                             QUAD_IMAGE_WIDTH, QUAD_IMAGE_HEIGHT, 1);

                box_blur(source,
                         processingBuffer + 2 * QUAD_IMAGE_SIZE,
                         QUAD_IMAGE_WIDTH, QUAD_IMAGE_HEIGHT);

                sobel_edge_detection(source,
                                     processingBuffer + 3 * QUAD_IMAGE_SIZE,
                                     QUAD_IMAGE_WIDTH, QUAD_IMAGE_HEIGHT);

                display_quad_image(processingBuffer, sharedDisplay->quadDisplayIndices[0], 0);
                display_quad_image(processingBuffer, sharedDisplay->quadDisplayIndices[1], 1);
                display_quad_image(processingBuffer, sharedDisplay->quadDisplayIndices[2], 2);
                display_quad_image(processingBuffer, sharedDisplay->quadDisplayIndices[3], 3);
            } else {
                // Full mode: SW[2:1] selects the processing operation
                switch (processMode) {
                    case PROC_FLIP:
                        process_flip(source, processingBuffer,
                                     IMAGE_WIDTH, IMAGE_HEIGHT, 1);
                        display_full_image(processingBuffer);
                        break;
                    case PROC_BLUR:
                        box_blur(source, processingBuffer,
                                 IMAGE_WIDTH, IMAGE_HEIGHT);
                        display_full_image(processingBuffer);
                        break;
                    case PROC_EDGE: {
                        // uint32_t start = IORD(USEC_COUNTER_BASE, 0);
                        
                        sobel_edge_detection(source, processingBuffer,
                                             IMAGE_WIDTH, IMAGE_HEIGHT);

                        // uint32_t end = IORD(USEC_COUNTER_BASE, 0);
                        // printf("Sobel time: %lu us\n", (end - start));

                        display_full_image(processingBuffer);
                        break;
                    }
                    
                    default: // PROC_RAW
                        display_full_image(source);
                        break;
                }
            }
        }
    }
    return 0;
}
