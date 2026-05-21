// Communications Nios II core -- receives camera frames via SPI, shares accelerometer
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

// Map the structs to SDRAM right after the 3 image buffers
SharedAccelData* shared_accel = (SharedAccelData*)(SHARED_ACCEL_DATA);
SharedDisplayState* shared_display = (SharedDisplayState*)(SHARED_DISPLAY_STATE);
SharedTimingData *sharedTimingData = (SharedTimingData*)(SHARED_TIMING_DATA);

// Global flags
volatile int free_buffers[3] = {1, 1, 1}; // 1 = free, 0 = in use

// Camera config tracking
#define CAM_RGB_MASK   0x01
#define CAM_QUAD_MASK  0x02
#define CAM_PACK_MASK  0x04
#define CAM_WRITE_MASK 0x10
uint8_t g_camLastConfig = 0x0;

// ISR: called when the image processing core returns a buffer token via ACK mailbox
static void mailbox_ack_isr(void* context) {
    uint32_t returnedBuf = IORD(ACK_MAILBOX_BASE, 0);
    if (returnedBuf < 3) {
        free_buffers[returnedBuf] = 1;
    }
}

// Called on a detected double-tap: cycles the quad display index for the tilt direction.
void doubletap_handler() {
    if (!shared_display->isQuad) {
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

    shared_display->quadDisplayIndices[idx] = (shared_display->quadDisplayIndices[idx] + 1) % 4;

    printf("Quad display: [%lu, %lu, %lu, %lu]\n",
           shared_display->quadDisplayIndices[0], shared_display->quadDisplayIndices[1],
           shared_display->quadDisplayIndices[2], shared_display->quadDisplayIndices[3]);
}

// Main loop: acquires camera frames into a free triple buffer slot and forwards
// the slot index to the image processing core via mailbox.
// @return 0 (loop runs indefinitely)
int main() {
	printf("Comms main\n");

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

    // Initialize shared display state
    shared_display->isQuad = 0;
    shared_display->quadDisplayIndices[0] = 0;
    shared_display->quadDisplayIndices[1] = 1;
    shared_display->quadDisplayIndices[2] = 2;
    shared_display->quadDisplayIndices[3] = 3;

    while (!IORD(CAM_REDY_BASE, 0));

    // Initial camera setup command
    uint8_t startup_cmd = CAM_RGB_MASK | CAM_PACK_MASK | CAM_WRITE_MASK;
    alt_avalon_spi_command(SPI_0_BASE, 0, 1, &startup_cmd, IMAGE_BYTES, buffers[0], 0);

    sharedTimingData->frameReadTime = 0;

    printf("Comms init\n");

    while(1) {
        // Spin here and wait for the ESP-CAM to pull GPIO[2] high.
        while (IORD(CAM_REDY_BASE, 0) == 0);

        // 1. Find a safe, unused buffer
        int write_target = -1;
        for (int i = 0; i < 3; i++) {
            if (free_buffers[i]) {
                write_target = i;
                break;
            }
        }

        if (write_target == -1) {
            continue;
        }

        // Update shared quad mode flag from switches each iteration
        // Core 0 reads this to know what frame size to request
        shared_display->isQuad = (IORD(SW_BASE, 0) & 0x1) ? 1 : 0;

        free_buffers[write_target] = 0; // lock buffer while receiving frame

        bool isQuad = shared_display->isQuad;

		// ALWAYS request a full RGB Packed frame. Do not trust the ESP-CAM to downsample.
		uint8_t cmd = CAM_RGB_MASK | CAM_PACK_MASK;
		if (cmd != g_camLastConfig) {
			g_camLastConfig = cmd;
			cmd |= CAM_WRITE_MASK;
			alt_avalon_spi_command(SPI_0_BASE, 0, 1, &cmd, 0, NULL, 0);
			free_buffers[write_target] = 1;
			continue;
		}

		// ALWAYS read the full 320x240 frame (115,200 bytes)
		uint32_t readSize = IMAGE_BYTES;
		uint8_t* dest_ptr = (uint8_t*)((uint32_t)buffers[write_target]);

		uint32_t frameReadBeginTime = IORD(USEC_COUNTER_BASE, 0);
		alt_avalon_spi_command(SPI_0_BASE, 0, 0, NULL, readSize, dest_ptr, 0);

        uint32_t frameReadEndTime = IORD(USEC_COUNTER_BASE, 0);
        sharedTimingData->frameReadTime = frameReadEndTime - frameReadBeginTime;

        // Fetch and share fresh accelerometer data with the image processing core
        accel_update();
        DeviceRotation currentRot = accel_get_device_rotation();

        shared_accel->x = currentRot.xAxis;
        shared_accel->y = currentRot.yAxis;
        shared_accel->z = currentRot.zAxis;

//        printf("Sent frame to image proc at addr %d\n", (int)dest_ptr);
//		printf("Pixel value at 1 0: %d\n", dest_ptr[1]);

        // Send the completed buffer index to the image processing core
        while (IORD(DATA_MAILBOX_BASE, 2) & MAILBOX_STATUS_FULL);
        IOWR(DATA_MAILBOX_BASE, 0, write_target);
    }
    return 0;
}
