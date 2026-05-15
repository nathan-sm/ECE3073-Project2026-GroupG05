// Shared SDRAM memory map for the dual-core Nios II system.
// Created By: Nathan Morris       (32532601)
//             Ritwam Shohaum      (33156816)
//             Shuk Kan LUI        (33891885)
//             Evgeny Solomin      (34977260)
// Created Date: 2026
// version = '1.0'

#ifndef MEMORY_ADDRESSES_H
#define MEMORY_ADDRESSES_H

#include "common_defs.h"

#define SHARED_MEM_BASE 0x1000000

#define IMAGE_READ_BUF_A SHARED_MEM_BASE
#define IMAGE_READ_BUF_B (IMAGE_READ_BUF_A + IMAGE_SIZE)
#define IMAGE_READ_BUF_C (IMAGE_READ_BUF_B + IMAGE_SIZE)

#define SHARED_ACCEL_DATA (IMAGE_READ_BUF_C + IMAGE_SIZE)

// 8-byte alignment padding inserted after SharedAccelData before SharedDisplayState
#define SHARED_ACCEL_PADDING 8
#define SHARED_DISPLAY_STATE (SHARED_ACCEL_DATA + sizeof(SharedAccelData) + SHARED_ACCEL_PADDING)

#endif
