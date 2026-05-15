// Image system Nios II core (single-processor version) — fetches camera frames,
// applies processing, drives the VGA pixel buffer, and handles accelerometer input.
// Created By: Nathan Morris       (32532601)
//             Ritwam Shohaum      (33156816)
//             Shuk Kan LUI        (33891885)
//             Evgeny Solomin      (34977260)
// Created Date: 2026
// version = '1.0'

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "system.h"
#include "io.h"
#include "altera_avalon_spi.h"

#include "accelerometer.h"

// SPI slave select indices
#define ESP_CAM_SS  0   // spi_ss_n[0]
#define ACCEL_SS    1   // spi_ss_n[1]

// Image dimensions
#define DISPLAY_WIDTH       320
#define DISPLAY_HEIGHT      240
#define FULL_IMAGE_WIDTH    320
#define FULL_IMAGE_HEIGHT   240
#define FULL_IMAGE_SIZE     (FULL_IMAGE_WIDTH * FULL_IMAGE_HEIGHT)
#define QUAD_IMAGE_WIDTH    (FULL_IMAGE_WIDTH / 2)
#define QUAD_IMAGE_HEIGHT   (FULL_IMAGE_HEIGHT / 2)
#define QUAD_IMAGE_SIZE     (QUAD_IMAGE_WIDTH * QUAD_IMAGE_HEIGHT)
#define QUAD_IMAGE_BUF_SIZE (QUAD_IMAGE_SIZE * 4)

// ESP-CAM command byte bit masks
// Bit 0: 0=Greyscale,  1=4-bit RGB
// Bit 1: 0=320x240,    1=160x120
// Bit 2: 0=Raw stream, 1=Packed stream
// Bit 3: 0=Flash off,  1=Flash on
// Bit 4: 0=No change,  1=Apply settings (must be set for bits 0–3 to register)
#define CAM_GREY_MASK   0x1
#define CAM_QUAD_MASK   0x2
#define CAM_PACK_MASK   0x4
#define CAM_FLASH_MASK  0x8
#define CAM_WRITE_MASK  0x10
#define CAM_CMD_DEFAULT 0x0   // greyscale, 320x240

// Processing mode codes selected by SW[2:1]
#define PROC_RAW    0
#define PROC_FLIP   1
#define PROC_BLUR   2
#define PROC_EDGE   3

// Threshold used by Sobel edge detection to binarise gradient magnitude
#define SOBEL_EDGE_THRESHOLD 60

// 7-segment display value for a blank (off) digit
#define HEX_BLANK 0xFF

// Tracks the last camera configuration sent so we only re-send on change
uint8_t gCamLastConfig = 0x0;

// Frame buffers in SDRAM
uint8_t fullImageBuffer[FULL_IMAGE_SIZE];
uint8_t quadImageBuffer[QUAD_IMAGE_BUF_SIZE];

// Processed image output buffers
uint8_t processedFull[FULL_IMAGE_SIZE];
uint8_t processedQuad[QUAD_IMAGE_BUF_SIZE];

// Array controlling which sub-image appears in each quadrant of the quad display
uint32_t gQuadDisplayIndices[4] = {0, 1, 2, 3};

// Minimum tilt on a single axis required to count as a directional double-tap
#define GYRO_CONTROL_THRESH_SINGLE 60
// Minimum combined tilt required to count as a directional double-tap
#define GYRO_CONTROL_THRESH_DOUBLE 90

// Flips the image 180 degrees (reverses pixel order across both axes).
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

// Writes the current FPS (derived from elapsed microseconds) to the 7-segment displays.
// @param elapsed Microseconds elapsed since the start of the previous frame
void display_fps(uint32_t elapsed) {
    // Store 100x FPS so we can use integer arithmetic for the decimal digit
    uint32_t fps100 = 100000000 / elapsed;

    const uint8_t SEG[10] = {
        0xC0, // 0
        0xF9, // 1
        0xA4, // 2
        0xB0, // 3
        0x99, // 4
        0x92, // 5
        0x82, // 6
        0xF8, // 7
        0x80, // 8
        0x90  // 9
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

// Sends the appropriate SPI command to the ESP-CAM and reads one frame into SDRAM.
// @param isQuad true to request a 160x120 quarter frame, false for 320x240 full frame
void receive_frame(bool isQuad) {
    uint8_t camCmd = gCamLastConfig;
    uint8_t camFlag = isQuad ? CAM_QUAD_MASK : 0x0;
    if ((gCamLastConfig & CAM_QUAD_MASK) != camFlag) {
        if (isQuad) {
            camCmd |= CAM_QUAD_MASK;
        } else {
            camCmd &= ~CAM_QUAD_MASK;
        }

        gCamLastConfig = camCmd;
        camCmd |= CAM_WRITE_MASK;
    }

    uint32_t readSize = isQuad ? QUAD_IMAGE_SIZE : FULL_IMAGE_SIZE;
    uint8_t *readDest = isQuad ? quadImageBuffer : fullImageBuffer;
    alt_avalon_spi_command(SPI_0_BASE, ESP_CAM_SS,
                           1, &camCmd,
                           readSize, readDest,
                           0);
}

// Writes the full 320x240 image to the VGA pixel buffer.
// @param buffer Pointer to a FULL_IMAGE_SIZE greyscale image
void display_full_image(uint8_t *buffer) {
    for (uint32_t i = 0; i < FULL_IMAGE_SIZE; i++) {
        IOWR(IMG_ADDY_BASE,  0, i);
        IOWR(PIXEL_DAT_BASE, 0, buffer[i] >> 4);
    }
}

// Writes one quarter-size image to a specific quadrant of the VGA pixel buffer.
// @param buffer       Pointer to a buffer holding 4 packed quarter images
// @param imageIndex   Which of the 4 sub-images to use as the source (0–3)
// @param displayIndex Which screen quadrant to target (0=TL, 1=TR, 2=BL, 3=BR)
void display_quad_image(uint8_t *buffer, uint32_t imageIndex, uint32_t displayIndex) {
    uint32_t imgAddr = imageIndex * QUAD_IMAGE_SIZE;
    uint32_t pixelBufferAddr = 0;

    // Move destination right by half the display width for right-side quadrants
    if (displayIndex & 0x1) {
        pixelBufferAddr += QUAD_IMAGE_WIDTH;
    }

    // Move destination down by half the display height for bottom quadrants
    if (displayIndex & 0x2) {
        pixelBufferAddr += QUAD_IMAGE_SIZE * 2;
    }

    for (uint32_t i = 0; i < QUAD_IMAGE_HEIGHT; i++) {
        for (uint32_t j = 0; j < QUAD_IMAGE_WIDTH; j++) {
            IOWR(IMG_ADDY_BASE,  0, pixelBufferAddr);
            IOWR(PIXEL_DAT_BASE, 0, buffer[imgAddr] >> 4);

            imgAddr++;
            pixelBufferAddr++;
        }

        // Advance destination to the next row of the full-width display
        pixelBufferAddr += FULL_IMAGE_WIDTH - QUAD_IMAGE_WIDTH;
    }
}

// Called on double-tap: cycles the quad display index for the quadrant matching the tilt.
void doubletap_callback() {
    uint16_t swStatus = IORD(SW_BASE, 0);
    bool isQuad = swStatus & 0x1;

    if (!isQuad) {
        return;
    }

    DeviceRotation deviceRotation = accel_get_device_rotation();

    if ((abs(deviceRotation.xAxis) < GYRO_CONTROL_THRESH_SINGLE
            && abs(deviceRotation.yAxis) < GYRO_CONTROL_THRESH_SINGLE)
            || abs(deviceRotation.xAxis) + abs(deviceRotation.yAxis) < GYRO_CONTROL_THRESH_DOUBLE) {
        return;
    }

    uint8_t imageX = deviceRotation.xAxis > 0 ? 0 : 1;
    uint8_t imageY = deviceRotation.yAxis < 0 ? 0 : 1;

    uint8_t imageIndex = (imageY << 1) | imageX;

    gQuadDisplayIndices[imageIndex] = (gQuadDisplayIndices[imageIndex] + 1) % 4;

    printf("Changed image display: [%lu, %lu, %lu, %lu]\n",
           gQuadDisplayIndices[0], gQuadDisplayIndices[1],
           gQuadDisplayIndices[2], gQuadDisplayIndices[3]);
}

// Main loop: polls for a camera ready signal, fetches a frame, processes it,
// displays it, and updates the FPS counter.
// @return 0 (loop runs indefinitely)
int main(void) {
    printf("ESP-CAM SPI initialised\n");

    // Send a startup command to put the camera into a known state
    uint8_t startupCmd = CAM_CMD_DEFAULT | CAM_WRITE_MASK;

    alt_avalon_spi_command(SPI_0_BASE, ESP_CAM_SS,
                           1, &startupCmd,
                           0, NULL, 0);

    // Clear frame buffers to black
    for (int i = 0; i < FULL_IMAGE_SIZE; i++) {
        fullImageBuffer[i] = 0x00;
    }

    for (int i = 0; i < QUAD_IMAGE_BUF_SIZE; i++) {
        quadImageBuffer[i] = 0x00;
    }

    if (accel_setup()) {
        printf("Gyro init failed.\n");
        return 1;
    }

    gyro_set_dtap_callback(&doubletap_callback);

    while (1) {
        // Poll for CAM_READY signal (GPIO[2] via cam_redy PIO)
        if (IORD(CAM_REDY_BASE, 0) == 1) {
            uint16_t swStatus = IORD(SW_BASE, 0);
            bool isQuad = swStatus & 0x1;
            int processMode = (swStatus >> 1) & 0x3;  // SW[2:1]

            receive_frame(isQuad);

            // Time the processing and display pipeline
            unsigned int tStart = IORD(USEC_COUNTER_BASE, 0);

            if (isQuad) {
                // slot 0: raw
                memcpy(processedQuad, quadImageBuffer, QUAD_IMAGE_SIZE);

                // slot 1: flip
                process_flip(quadImageBuffer,
                             processedQuad + QUAD_IMAGE_SIZE,
                             QUAD_IMAGE_WIDTH, QUAD_IMAGE_HEIGHT, 1);

                // slot 2: blur (placeholder — copies raw for now)
                memcpy(processedQuad + 2 * QUAD_IMAGE_SIZE,
                       quadImageBuffer, QUAD_IMAGE_SIZE);

                // slot 3: edge (placeholder — copies raw for now)
                memcpy(processedQuad + 3 * QUAD_IMAGE_SIZE,
                       quadImageBuffer, QUAD_IMAGE_SIZE);

                display_quad_image(processedQuad, gQuadDisplayIndices[0], 0);
                display_quad_image(processedQuad, gQuadDisplayIndices[1], 1);
                display_quad_image(processedQuad, gQuadDisplayIndices[2], 2);
                display_quad_image(processedQuad, gQuadDisplayIndices[3], 3);
            } else {
                // Single mode: SW[2:1] selects the processing operation
                if (processMode == PROC_FLIP) {
                    process_flip(fullImageBuffer, processedFull,
                                 FULL_IMAGE_WIDTH, FULL_IMAGE_HEIGHT, 1);
                } else {
                    // Raw (and placeholders for blur/edge)
                    memcpy(processedFull, fullImageBuffer, FULL_IMAGE_SIZE);
                }
                display_full_image(processedFull);
            }

            display_fps(IORD(USEC_COUNTER_BASE, 0) - tStart);
        }

        if (accel_update()) {
            printf("Gyro read failed\n");
        }
    }

    return 0;
}
