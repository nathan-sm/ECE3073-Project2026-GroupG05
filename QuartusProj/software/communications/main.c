#include <stdint.h>
#include "system.h"
#include "altera_avalon_spi.h"
#include "accelerometer.h"
#include "io.h"

#define SHARED_MEM_BASE (SDRAM_CONTROL_BASE + 0x01000000)
#define IMAGE_SIZE 76800

// --- Mailbox Helper Macros ---
#define MB_STATUS_PENDING 0x01
#define MB_STATUS_FULL    0x02

#define MAILBOX_C2I_BASE DATA_MAILBOX_BASE
#define MAILBOX_I2C_BASE ACK_MAILBOX_BASE

// Write data to command register (Blocks if FIFO is full)
void mailbox_send(int base, uint32_t data) {
    while (IORD(base, 2) & MB_STATUS_FULL);
    IOWR(base, 0, data);
}

// Read from command register (Non-blocking, returns 1 if successful)
int mailbox_try_read(int base, uint32_t* data) {
    if (IORD(base, 2) & MB_STATUS_PENDING) {
        *data = IORD(base, 0); // Reading the command register pops the FIFO
        return 1;
    }
    return 0;
}

uint8_t* buffers[3] = {
    (uint8_t*)(SHARED_MEM_BASE),
    (uint8_t*)(SHARED_MEM_BASE + IMAGE_SIZE),
    (uint8_t*)(SHARED_MEM_BASE + (2 * IMAGE_SIZE))
};

int main() {
    accel_setup();

    // Token system: 1 = Comms owns it (free), 0 = Image owns it (in use)
    int free_buffers[3] = {1, 1, 1};
    int write_target = 0;
    uint32_t returned_buf;

    while(1) {
        // 1. Reclaim any returned buffers from the Image CPU
        while (mailbox_try_read(MAILBOX_I2C_BASE, &returned_buf)) {
            if (returned_buf < 3) {
                free_buffers[returned_buf] = 1;
            }
        }

        // 2. Find a unused buffer
        write_target = -1;
        for (int i = 0; i < 3; i++) {
            if (free_buffers[i]) {
                write_target = i;
                break;
            }
        }

        // If no buffers are free, skip this cycle and keep polling the mailbox
        if (write_target == -1) {
            continue;
        }

        // Mark buffer as in-use
        free_buffers[write_target] = 0;

        // 3. Fetch Frame via SPI into the buffer
        uint8_t cmd = 0x10;
        alt_avalon_spi_command(SPI_0_BASE, 0, 1, &cmd, IMAGE_SIZE, buffers[write_target], 0);

        // 4. Fetch Accelerometer Data
        accel_update();

        // 5. Send the finished buffer token to the Image CPU
        mailbox_send(MAILBOX_C2I_BASE, write_target);
    }
    return 0;
}
