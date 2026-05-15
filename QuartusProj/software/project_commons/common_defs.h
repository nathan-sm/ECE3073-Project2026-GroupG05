#ifndef COMMON_DEFS_H
#define COMMON_DEFS_H

#include "stdint.h"

#define IMAGE_WIDTH 320
#define IMAGE_HEIGHT 240
#define IMAGE_SIZE (IMAGE_WIDTH * IMAGE_HEIGHT)
#define IMAGE_BYTES (IMAGE_SIZE + (IMAGE_SIZE / 2)) // 115200 Bytes for 12-bit packed

#define QUAD_IMAGE_WIDTH 160
#define QUAD_IMAGE_HEIGHT 120
#define QUAD_IMAGE_SIZE (QUAD_IMAGE_WIDTH * QUAD_IMAGE_HEIGHT)
#define QUAD_IMAGE_BYTES (QUAD_IMAGE_SIZE + (QUAD_IMAGE_SIZE / 2)) // 28800 Bytes

// Struct to hold the accelerometer data safely
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} SharedAccelData;

typedef struct {
    volatile uint8_t isQuad;
    volatile uint8_t _pad[3];
    volatile uint32_t quadDisplayIndices[4];
} SharedDisplayState;

typedef struct {
	volatile uint32_t frameReadTime;

	volatile uint32_t noFilterTime;
	volatile uint32_t flipTime;
	volatile uint32_t blurTime;
	volatile uint32_t sobelTime;

	volatile uint32_t displayTime;

	volatile uint32_t frameTime;
} SharedTimingData;

#endif
