// Common project-wide type definitions and constants shared between both Nios II cores.
// Created By: Nathan Morris       (32532601)
//             Ritwam Shohaum      (33156816)
//             Shuk Kan LUI        (33891885)
//             Evgeny Solomin      (34977260)
// Created Date: 2026
// version = '1.0'

#ifndef COMMON_DEFS_H
#define COMMON_DEFS_H

#include "stdint.h"

#define IMAGE_WIDTH 320
#define IMAGE_HEIGHT 240
#define IMAGE_SIZE (IMAGE_WIDTH * IMAGE_HEIGHT)

#define QUAD_IMAGE_WIDTH 160
#define QUAD_IMAGE_HEIGHT 120
#define QUAD_IMAGE_SIZE (QUAD_IMAGE_WIDTH * QUAD_IMAGE_HEIGHT)

// Bit mask applied to a cached SDRAM address to obtain the uncached alias
#define UNCACHED_MEM_MASK 0x80000000

// 7-segment display value for a blank (off) digit
#define HEX_BLANK 0xFF

// Struct to hold the accelerometer data safely across cores
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

#endif
