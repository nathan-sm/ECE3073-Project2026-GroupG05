#include <stdint.h>
#include "system.h"
#include "io.h"
#include "sys/alt_irq.h"
#include "altera_avalon_spi.h"
#include "accelerometer.h"

// --- Shared Memory Configuration ---
#define SHARED_MEM_BASE (SDRAM_CONTROL_BASE + 0x01000000)
#define IMAGE_SIZE 76800

// Triple Buffer Pointers
uint8_t* buffers[3] = {
    (uint8_t*)(SHARED_MEM_BASE),
    (uint8_t*)(SHARED_MEM_BASE + IMAGE_SIZE),
    (uint8_t*)(SHARED_MEM_BASE + (2 * IMAGE_SIZE))
};

// Struct to hold the accelerometer data safely
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} SharedAccelData;

// Map the struct to SDRAM right after the 3 image buffers
SharedAccelData* shared_accel = (SharedAccelData*)((SHARED_MEM_BASE + (3 * IMAGE_SIZE)) | 0x80000000);

// Global flags
volatile int free_buffers[3] = {1, 1, 1}; // 1 = free, 0 = in use

// --- Interrupt Service Routine (ISR) ---
static void mailbox_ack_isr(void* context) {
    uint32_t returned_buf = IORD(ACK_MAILBOX_BASE, 0);
    if (returned_buf < 3) {
        free_buffers[returned_buf] = 1;
    }
}

int main() {
    accel_setup();

    alt_ic_isr_register(
        ACK_MAILBOX_IRQ_INTERRUPT_CONTROLLER_ID,
        ACK_MAILBOX_IRQ,
        mailbox_ack_isr,
        NULL,
        NULL
    );

    IOWR(ACK_MAILBOX_BASE, 3, 0x01);

    // Initial camera setup command
    uint8_t startup_cmd = 0x10;
    alt_avalon_spi_command(SPI_0_BASE, 0, 1, &startup_cmd, 0, NULL, 0);

    while(1) {
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

        // Spin here and wait for the ESP-CAM to pull GPIO[2] high.
        // This ensures we only fetch complete frames and don't overwhelm the SPI/SDRAM bus.
        while (IORD(CAM_REDY_BASE, 0) == 0);

        free_buffers[write_target] = 0; // Lock buffer

        // 2. Fetch Frame via SPI into the targeted buffer
        uint8_t cmd = 0x10;
        uint8_t* dest_ptr = (uint8_t*)((uint32_t)buffers[write_target] | 0x80000000);

        alt_avalon_spi_command(
            SPI_0_BASE,
            0,
            1,
            &cmd,
            IMAGE_SIZE,
            dest_ptr,
            0
        );

        // 3. Fetch and Share Accelerometer Data
        accel_update();
        DeviceRotation current_rot = accel_get_device_rotation();

        shared_accel->x = current_rot.x_axis;
        shared_accel->y = current_rot.y_axis;
        shared_accel->z = current_rot.z_axis;

        // 4. Send the finished buffer token to the Image CPU
        while (IORD(DATA_MAILBOX_BASE, 2) & 0x02);
        IOWR(DATA_MAILBOX_BASE, 0, write_target);
    }
    return 0;
}
