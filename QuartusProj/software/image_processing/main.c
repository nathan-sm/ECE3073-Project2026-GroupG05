#include <stdint.h>
#include "system.h"
#include "io.h"

#define SHARED_MEM_BASE (SDRAM_CONTROL_BASE + 0x01000000)
#define IMAGE_SIZE 76800

// --- Mailbox Helper Macros ---
#define MB_STATUS_PENDING 0x01
#define MB_STATUS_FULL    0x02

#define MAILBOX_C2I_BASE DATA_MAILBOX_BASE
#define MAILBOX_I2C_BASE ACK_MAILBOX_BASE

void mailbox_send(int base, uint32_t data) {
    while (IORD(base, 2) & MB_STATUS_FULL);
    IOWR(base, 0, data);
}

int mailbox_try_read(int base, uint32_t* data) {
    if (IORD(base, 2) & MB_STATUS_PENDING) {
        *data = IORD(base, 0);
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
    int current_display = -1;
    uint32_t msg;

    while(1) {
        int new_frame = -1;

        // 1. Drain the mailbox
        while (mailbox_try_read(MAILBOX_C2I_BASE, &msg)) {
            if (new_frame != -1) {
                // We just found a newer frame in the queue.
                // Return the skipped frame back to Comms CPU.
                mailbox_send(MAILBOX_I2C_BASE, new_frame);
            }
            new_frame = msg;
        }

        // 2. Lock the new frame, return the old one
        if (new_frame != -1) {
            if (current_display != -1) {
                mailbox_send(MAILBOX_I2C_BASE, current_display);
            }
            current_display = new_frame;
        }

        // If we don't have any frame yet (startup phase), spin and wait
        if (current_display == -1) {
            continue;
        }

        uint8_t* source_buffer = buffers[current_display];

        // --- Write to VGA ---
        for (int i = 0; i < IMAGE_SIZE; i++) {
            IOWR(PIXEL_DAT_BASE, 0, source_buffer[i] >> 4);
            IOWR(IMG_ADDY_BASE, 0, i);
        }
    }
    return 0;
}
