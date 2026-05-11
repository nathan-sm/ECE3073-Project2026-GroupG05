#include <stdint.h>
#include "system.h"
#include "io.h"
#include "sys/alt_irq.h"

#define SHARED_MEM_BASE (SDRAM_CONTROL_BASE + 0x01000000)
#define IMAGE_SIZE 76800

uint8_t* buffers[3] = {
    (uint8_t*)(SHARED_MEM_BASE),
    (uint8_t*)(SHARED_MEM_BASE + IMAGE_SIZE),
    (uint8_t*)(SHARED_MEM_BASE + (2 * IMAGE_SIZE))
};

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} SharedAccelData;

SharedAccelData* shared_accel = (SharedAccelData*)((SHARED_MEM_BASE + (3 * IMAGE_SIZE)) | 0x80000000);

volatile int new_frame_ready = 0;
volatile uint32_t current_frame_index = 0;

static void mailbox_rx_isr(void* context) {
    current_frame_index = IORD(DATA_MAILBOX_BASE, 0);
    new_frame_ready = 1;
}

// FPS DISPLAY FUNCTION
void display_fps(uint32_t elapsed) {
    if (elapsed == 0) return; // Prevent divide by zero on first frame

    uint32_t fps_100 = 100000000 / elapsed;

    const uint8_t SEG[10] = {
        0xC0, 0xF9, 0xA4, 0xB0, 0x99,
        0x92, 0x82, 0xF8, 0x80, 0x90
    };

    int d3 = (fps_100 / 1000) % 10;
    int d2 = (fps_100 / 100)  % 10;
    int d1 = (fps_100 / 10)   % 10;
    int d0 =  fps_100         % 10;

    uint8_t hex3 = SEG[d3];
    uint8_t hex2 = SEG[d2] & ~(1 << 7);
    uint8_t hex1 = SEG[d1];
    uint8_t hex0 = SEG[d0];

    uint32_t hex0_2 = ((uint32_t)hex2 << 16) | ((uint32_t)hex1 << 8) | hex0;
    uint32_t hex3_5 = (0xFF << 16) | (0xFF << 8) | hex3;

    IOWR(HEX20_BASE, 0, hex0_2);
    IOWR(HEX53_BASE, 0, hex3_5);
}

int main() {
    int currently_displaying = -1;
    uint32_t last_frame_time = 0;

    alt_ic_isr_register(
        DATA_MAILBOX_IRQ_INTERRUPT_CONTROLLER_ID,
        DATA_MAILBOX_IRQ,
        mailbox_rx_isr,
        NULL,
        NULL
    );

    IOWR(DATA_MAILBOX_BASE, 3, 0x01);

    volatile int* const pixel_dat_ptr = (volatile int*)(PIXEL_DAT_BASE | 0x80000000);
    volatile int* const img_addy_ptr  = (volatile int*)(IMG_ADDY_BASE  | 0x80000000);

    while(1) {
        if (new_frame_ready) {
            new_frame_ready = 0;

            // Calculate the time it took since the last frame was delivered
            uint32_t current_time = IORD(USEC_COUNTER_BASE, 0);
            if (last_frame_time != 0) {
                uint32_t frameTime = current_time - last_frame_time;
                display_fps(frameTime);
            }
            last_frame_time = current_time;

            if (currently_displaying != -1) {
                while (IORD(ACK_MAILBOX_BASE, 2) & 0x02);
                IOWR(ACK_MAILBOX_BASE, 0, currently_displaying);
            }

            currently_displaying = current_frame_index;

            // Shared accel data, for quad image display.
            int16_t x_val = shared_accel->x;
            int16_t y_val = shared_accel->y;

            uint8_t* source_buffer = buffers[currently_displaying];
            uint32_t* src_ptr32 = (uint32_t*)((uint32_t)source_buffer | 0x80000000);
            uint32_t* const src_end32 = (uint32_t*)(((uint32_t)source_buffer + IMAGE_SIZE) | 0x80000000);

            int addr = 0;

            while (src_ptr32 < src_end32) {
                uint32_t block = *src_ptr32++;

                *img_addy_ptr = addr++;
                *pixel_dat_ptr = (block & 0xFF) >> 4;

                *img_addy_ptr = addr++;
                *pixel_dat_ptr = ((block >> 8) & 0xFF) >> 4;

                *img_addy_ptr = addr++;
                *pixel_dat_ptr = ((block >> 16) & 0xFF) >> 4;

                *img_addy_ptr = addr++;
                *pixel_dat_ptr = ((block >> 24) & 0xFF) >> 4;
            }
        }
    }
    return 0;
}
