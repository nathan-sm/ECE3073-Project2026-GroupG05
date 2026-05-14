#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "system.h"
#include "io.h"
#include "sys/alt_irq.h"
#include "altera_avalon_spi.h"
#include "accelerometer.h"

// Memory Config
#define SHARED_MEM_BASE (SDRAM_CONTROL_BASE + 0x01000000)

#define IMAGE_PIXELS 76800
#define IMAGE_BYTES  115200  // 76800 * 1.5
#define QUAD_PIXELS  19200
#define QUAD_BYTES   28800   // 19200 * 1.5

// Triple Buffer Pointers
uint8_t* buffers[3] = {
    (uint8_t*)(SHARED_MEM_BASE),
    (uint8_t*)(SHARED_MEM_BASE + IMAGE_BYTES),
    (uint8_t*)(SHARED_MEM_BASE + (2 * IMAGE_BYTES))
};

// Struct to hold the accelerometer data safely
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} SharedAccelData;

// Shared display state between cores
typedef struct {
    volatile uint8_t isQuad;  // Core 1 writes (from SW[0]), Core 0 reads
    volatile uint8_t _pad[3];
    volatile uint32_t quadDisplayIndices[4];  // Core 0 writes (double-tap), Core 1 reads
} SharedDisplayState;

// Struts to SDRAM
SharedAccelData* shared_accel = (SharedAccelData*)((SHARED_MEM_BASE + (3 * IMAGE_BYTES)) | 0x80000000);
SharedDisplayState* shared_display = (SharedDisplayState*)((SHARED_MEM_BASE + (3 * IMAGE_BYTES) + sizeof(SharedAccelData) + 8) | 0x80000000);

// Global flags
volatile int free_buffers[3] = {1, 1, 1}; // 1 = free, 0 = in use

// Camera config tracking
#define CAM_RGB_MASK   0x01
#define CAM_QUAD_MASK  0x02
#define CAM_PACK_MASK  0x04
#define CAM_WRITE_MASK 0x10
uint8_t g_camLastConfig = 0xFF; // Force initial update

// Gyro thresholds for quad display control
#define GYRO_THRESH_SINGLE 60
#define GYRO_THRESH_DOUBLE 90

// --- Interrupt Service Routine (ISR) ---
static void mailbox_ack_isr(void* context) {
    uint32_t returned_buf = IORD(ACK_MAILBOX_BASE, 0);
    if (returned_buf < 3) free_buffers[returned_buf] = 1;
}

// --- Double-tap callback: cycle quad display based on tilt ---
void doubletap_handler() {
    if (!shared_display->isQuad) return;

    DeviceRotation rot = accel_get_device_rotation();

    if ((abs(rot.x_axis) < GYRO_THRESH_SINGLE && abs(rot.y_axis) < GYRO_THRESH_SINGLE)
            || abs(rot.x_axis) + abs(rot.y_axis) < GYRO_THRESH_DOUBLE)
        return;

    uint8_t imageX = rot.x_axis > 0 ? 0 : 1;
    uint8_t imageY = rot.y_axis < 0 ? 0 : 1;
    uint8_t idx = (imageY << 1) | imageX;

    shared_display->quadDisplayIndices[idx] = (shared_display->quadDisplayIndices[idx] + 1) % 4;
}

int main() {
    accel_setup();
    gyro_set_dtap_callback(&doubletap_handler);

    alt_ic_isr_register(ACK_MAILBOX_IRQ_INTERRUPT_CONTROLLER_ID, ACK_MAILBOX_IRQ, mailbox_ack_isr, NULL, NULL);
    IOWR(ACK_MAILBOX_BASE, 3, 0x01);

    // Initialize shared display state
    shared_display->isQuad = 0;
    shared_display->quadDisplayIndices[0] = 0;
    shared_display->quadDisplayIndices[1] = 1;
    shared_display->quadDisplayIndices[2] = 2;
    shared_display->quadDisplayIndices[3] = 3;

    while(1) {
        int write_target = -1;
        for (int i = 0; i < 3; i++) {
            if (free_buffers[i]) {
                write_target = i;
                break;
            }
        }

        if (write_target == -1) continue;

        while (IORD(CAM_REDY_BASE, 0) == 0);

        free_buffers[write_target] = 0;
        bool isQuad = shared_display->isQuad;

        uint8_t desired_config = CAM_RGB_MASK | CAM_PACK_MASK | (isQuad ? CAM_QUAD_MASK : 0x00);

        if (desired_config != g_camLastConfig) {
            uint8_t write_cmd = desired_config | CAM_WRITE_MASK;
            alt_avalon_spi_command(SPI_0_BASE, 0, 1, &write_cmd, 0, NULL, 0);
            g_camLastConfig = desired_config;

            free_buffers[write_target] = 1; // Release buffer immediately
            continue; // Spin and wait for a clean frame next time
        }

        uint32_t readSize = isQuad ? QUAD_BYTES : IMAGE_BYTES;
        uint8_t* dest_ptr = (uint8_t*)((uint32_t)buffers[write_target] | 0x80000000);

        alt_avalon_spi_command(SPI_0_BASE, 0, 1, &desired_config, readSize, dest_ptr, 0);

        accel_update();
        DeviceRotation current_rot = accel_get_device_rotation();
        shared_accel->x = current_rot.x_axis;
        shared_accel->y = current_rot.y_axis;
        shared_accel->z = current_rot.z_axis;

        while (IORD(DATA_MAILBOX_BASE, 2) & 0x02);
        IOWR(DATA_MAILBOX_BASE, 0, write_target);
    }
    return 0;
}
