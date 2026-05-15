// Communications Nios II core — receives camera frames via SPI, shares accelerometer
// data, and forwards frame buffer tokens to the image processing core via mailbox.
// Created By: Nathan Morris       (32532601)
//             Ritwam Shohaum      (33156816)
//             Shuk Kan LUI        (33891885)
//             Evgeny Solomin      (34977260)
// Created Date: 2026
// version = '1.0'

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "system.h"
#include "io.h"
#include "sys/alt_irq.h"
#include "altera_avalon_spi.h"
#include "accelerometer.h"

#include "common_defs.h"
#include "memory_addresses.h"

// Mailbox register offsets
#define MAILBOX_STATUS_FULL 0x02  // bit in the STATUS register indicating mailbox is full
#define MAILBOX_IRQ_ENABLE  0x01  // value written to the IRQ enable register

// Camera command byte bit masks
#define CAM_QUAD_MASK  0x02
#define CAM_WRITE_MASK 0x10

// Minimum tilt required on a single axis to count as a directional double-tap
#define GYRO_THRESH_SINGLE 60
// Minimum combined tilt required to count as a directional double-tap
#define GYRO_THRESH_DOUBLE 90

// Triple buffer base addresses in SDRAM
uint8_t* buffers[3] = {
    (uint8_t*)(IMAGE_READ_BUF_A),
    (uint8_t*)(IMAGE_READ_BUF_B),
    (uint8_t*)(IMAGE_READ_BUF_C)
};

// Shared structs mapped to uncached SDRAM aliases so both cores see writes immediately
SharedAccelData* sharedAccel = (SharedAccelData*)(SHARED_ACCEL_DATA | UNCACHED_MEM_MASK);
SharedDisplayState* sharedDisplay = (SharedDisplayState*)(SHARED_DISPLAY_STATE | UNCACHED_MEM_MASK);

// 1 = buffer is free for writing, 0 = buffer is locked by the image processing core
volatile int freeBuffers[3] = {1, 1, 1};

// Tracks the last camera configuration byte sent so we only re-send on change
uint8_t gCamLastConfig = 0;

// ISR: called when the image processing core returns a buffer token via ACK mailbox
static void mailbox_ack_isr(void* context) {
    uint32_t returnedBuf = IORD(ACK_MAILBOX_BASE, 0);
    if (returnedBuf < 3) {
        freeBuffers[returnedBuf] = 1;
    }
}

// Called on a detected double-tap: cycles the quad display index for the tilt direction.
void doubletap_handler() {
    if (!sharedDisplay->isQuad) {
        return;
    }

    DeviceRotation rot = accel_get_device_rotation();

    if ((abs(rot.xAxis) < GYRO_THRESH_SINGLE
            && abs(rot.yAxis) < GYRO_THRESH_SINGLE)
            || abs(rot.xAxis) + abs(rot.yAxis) < GYRO_THRESH_DOUBLE) {
        return;
    }

    uint8_t imageX = rot.xAxis > 0 ? 0 : 1;
    uint8_t imageY = rot.yAxis < 0 ? 0 : 1;
    uint8_t idx = (imageY << 1) | imageX;

    sharedDisplay->quadDisplayIndices[idx] = (sharedDisplay->quadDisplayIndices[idx] + 1) % 4;

    printf("Quad display: [%lu, %lu, %lu, %lu]\n",
           sharedDisplay->quadDisplayIndices[0], sharedDisplay->quadDisplayIndices[1],
           sharedDisplay->quadDisplayIndices[2], sharedDisplay->quadDisplayIndices[3]);
}

// Main loop: acquires camera frames into a free triple buffer slot and forwards
// the slot index to the image processing core via mailbox.
// @return 0 (loop runs indefinitely)
int main() {
    accel_setup();
    gyro_set_dtap_callback(&doubletap_handler);

    alt_ic_isr_register(
        ACK_MAILBOX_IRQ_INTERRUPT_CONTROLLER_ID,
        ACK_MAILBOX_IRQ,
        mailbox_ack_isr,
        NULL,
        NULL
    );

    IOWR(ACK_MAILBOX_BASE, 3, MAILBOX_IRQ_ENABLE);

    // Initialise shared display state
    sharedDisplay->isQuad = 0;
    sharedDisplay->quadDisplayIndices[0] = 0;
    sharedDisplay->quadDisplayIndices[1] = 1;
    sharedDisplay->quadDisplayIndices[2] = 2;
    sharedDisplay->quadDisplayIndices[3] = 3;

    // Send initial camera command to put it into a known state
    uint8_t startupCmd = CAM_WRITE_MASK;
    alt_avalon_spi_command(SPI_0_BASE, 0, 1, &startupCmd, 0, NULL, 0);

    while (1) {
        // Find a free, unused buffer slot
        int writeTarget = -1;
        for (int i = 0; i < 3; i++) {
            if (freeBuffers[i]) {
                writeTarget = i;
                break;
            }
        }

        if (writeTarget == -1) {
            continue;
        }

        // Spin until the ESP-CAM signals it is ready (GPIO[2] high)
        while (IORD(CAM_REDY_BASE, 0) == 0);

        freeBuffers[writeTarget] = 0; // lock buffer while receiving frame

        // Check quad mode (set by the image processing core reading SW[0])
        bool isQuad = sharedDisplay->isQuad;

        // Build camera command — only set WRITE_MASK when the config has changed
        uint8_t cmd = isQuad ? CAM_QUAD_MASK : 0x00;
        if (cmd != gCamLastConfig) {
            gCamLastConfig = cmd;
            cmd |= CAM_WRITE_MASK;
        }

        uint32_t readSize = isQuad ? QUAD_IMAGE_SIZE : IMAGE_SIZE;
        uint8_t* destPtr = (uint8_t*)((uint32_t)buffers[writeTarget] | UNCACHED_MEM_MASK);

        alt_avalon_spi_command(
            SPI_0_BASE,
            0,
            1,
            &cmd,
            readSize,
            destPtr,
            0
        );

        // Fetch and share fresh accelerometer data with the image processing core
        accel_update();
        DeviceRotation currentRot = accel_get_device_rotation();

        sharedAccel->x = currentRot.xAxis;
        sharedAccel->y = currentRot.yAxis;
        sharedAccel->z = currentRot.zAxis;

        // Send the completed buffer index to the image processing core
        while (IORD(DATA_MAILBOX_BASE, 2) & MAILBOX_STATUS_FULL);
        IOWR(DATA_MAILBOX_BASE, 0, writeTarget);
    }
    return 0;
}
