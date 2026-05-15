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

// Triple Buffer Pointers
uint8_t* buffers[3] = {
    (uint8_t*)(IMAGE_READ_BUF_A),
    (uint8_t*)(IMAGE_READ_BUF_B),
    (uint8_t*)(IMAGE_READ_BUF_C)
};

// Map the structs to SDRAM right after the 3 image buffers
SharedAccelData* shared_accel = (SharedAccelData*)(SHARED_ACCEL_DATA);
SharedDisplayState* shared_display = (SharedDisplayState*)(SHARED_DISPLAY_STATE);

// Global flags
volatile int free_buffers[3] = {1, 1, 1}; // 1 = free, 0 = in use

// Camera config tracking
#define CAM_QUAD_MASK  0x02
#define CAM_WRITE_MASK 0x10
uint8_t g_camLastConfig = 0x0;

// Gyro thresholds for quad display control
#define GYRO_THRESH_SINGLE 60
#define GYRO_THRESH_DOUBLE 90

// --- Interrupt Service Routine (ISR) ---
static void mailbox_ack_isr(void* context) {
    uint32_t returned_buf = IORD(ACK_MAILBOX_BASE, 0);
    if (returned_buf < 3) {
        free_buffers[returned_buf] = 1;
    }
}

// --- Double-tap callback: cycle quad display based on tilt ---
void doubletap_handler()
{
    if (!shared_display->isQuad)
        return;

    DeviceRotation rot = accel_get_device_rotation();

    if ((abs(rot.x_axis) < GYRO_THRESH_SINGLE
            && abs(rot.y_axis) < GYRO_THRESH_SINGLE)
            || abs(rot.x_axis) + abs(rot.y_axis) < GYRO_THRESH_DOUBLE)
        return;

    uint8_t imageX = rot.x_axis > 0 ? 0 : 1;
    uint8_t imageY = rot.y_axis < 0 ? 0 : 1;
    uint8_t idx = (imageY << 1) | imageX;

    shared_display->quadDisplayIndices[idx] = (shared_display->quadDisplayIndices[idx] + 1) % 4;

    printf("Quad display: [%lu, %lu, %lu, %lu]\n",
           shared_display->quadDisplayIndices[0], shared_display->quadDisplayIndices[1],
           shared_display->quadDisplayIndices[2], shared_display->quadDisplayIndices[3]);
}

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

    IOWR(ACK_MAILBOX_BASE, 3, 0x01);

    // Initialize shared display state
    shared_display->isQuad = 0;
    shared_display->quadDisplayIndices[0] = 0;
    shared_display->quadDisplayIndices[1] = 1;
    shared_display->quadDisplayIndices[2] = 2;
    shared_display->quadDisplayIndices[3] = 3;

    while (!IORD(CAM_REDY_BASE, 0));

    // Initial camera setup command
    uint8_t startup_cmd = CAM_WRITE_MASK;
    alt_avalon_spi_command(SPI_0_BASE, 0, 1, &startup_cmd, 0, NULL, 0);

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

        free_buffers[write_target] = 0; // Lock buffer

        bool isQuad = shared_display->isQuad;

        // 3. Build camera command only set WRITE_MASK when config changes
        uint8_t cmd = isQuad ? CAM_QUAD_MASK : 0x00;
        if (cmd != g_camLastConfig) {
            g_camLastConfig = cmd;
            cmd |= CAM_WRITE_MASK;
        }

        uint32_t readSize = isQuad ? QUAD_IMAGE_SIZE : IMAGE_SIZE;
        uint8_t* dest_ptr = (uint8_t*)((uint32_t)buffers[write_target]);

        alt_avalon_spi_command(
            SPI_0_BASE,
            0,
            1,
            &cmd,
            readSize,
            dest_ptr,
            0
        );

        // 4. Fetch and Share Accelerometer Data
        accel_update();
        DeviceRotation current_rot = accel_get_device_rotation();

        shared_accel->x = current_rot.x_axis;
        shared_accel->y = current_rot.y_axis;
        shared_accel->z = current_rot.z_axis;

//        printf("Sent frame to image proc at addr %d\n", (int)dest_ptr);
//		printf("Pixel value at 1 0: %d\n", dest_ptr[1]);

        // 5. Send the finished buffer token to the Image CPU
        while (IORD(DATA_MAILBOX_BASE, 2) & 0x02);
        IOWR(DATA_MAILBOX_BASE, 0, write_target);
    }
    return 0;
}
