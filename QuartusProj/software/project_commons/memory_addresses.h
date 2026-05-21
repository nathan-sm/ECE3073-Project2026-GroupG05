// Shared SDRAM memory map for the three-core Nios II system.
// Created By: Nathan Morris       (32532601)
//             Ritwam Shohaum      (33156816)
//             Shuk Kan LUI        (33891885)
//             Evgeny Solomin      (34977260)
// Created Date: 2026
// version = '1.0'

#ifndef MEMORY_ADDRESSES_H
#define MEMORY_ADDRESSES_H

#include "common_defs.h"

//#define SHARED_MEM_BASE (0x100000)
#define SHARED_MEM_BASE (0x100000 | 0x80000000)

#define IMAGE_READ_BUF_A SHARED_MEM_BASE
#define IMAGE_READ_BUF_B (IMAGE_READ_BUF_A + IMAGE_BYTES)
#define IMAGE_READ_BUF_C (IMAGE_READ_BUF_B + IMAGE_BYTES)

#define SHARED_ACCEL_DATA (IMAGE_READ_BUF_C + IMAGE_BYTES)
#define SHARED_DISPLAY_STATE (SHARED_ACCEL_DATA + sizeof(SharedAccelData))

#define PROCESSED_IMG_BUF_A (SHARED_DISPLAY_STATE + sizeof(SharedDisplayState))
#define PROCESSED_IMG_BUF_B (PROCESSED_IMG_BUF_A + (IMAGE_SIZE * 2))

#define SHARED_TIMING_DATA (PROCESSED_IMG_BUF_B + (IMAGE_SIZE * 2))

#endif
