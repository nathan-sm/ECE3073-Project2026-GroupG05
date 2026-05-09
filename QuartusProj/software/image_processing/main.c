// CPU 1: main.c (Image Processing & Display)
#include <stdint.h>
#include "system.h"
#include "io.h"
#include "altera_avalon_mutex.h"

#define SHARED_MEM_BASE (SDRAM_CONTROL_BASE + 0x01000000)
#define IMAGE_SIZE 76800

// Map the exact same shared memory addresses as CPU 2
uint8_t* buffer_A = (uint8_t*)(SHARED_MEM_BASE);
uint8_t* buffer_B = (uint8_t*)(SHARED_MEM_BASE + IMAGE_SIZE);
volatile int* latest_ready_buffer = (volatile int*)(SHARED_MEM_BASE + (2 * IMAGE_SIZE));

int main() {
    alt_mutex_dev* mutex = altera_avalon_mutex_open("/dev/mutex_0");

    int buffer_to_process = 0;
    uint8_t* source_buffer;

    while(1) {
        // --- Check for new data ---
        altera_avalon_mutex_lock(mutex, 1);
        buffer_to_process = *latest_ready_buffer;
        altera_avalon_mutex_unlock(mutex);

        source_buffer = (buffer_to_process == 0) ? buffer_A : buffer_B;

        // --- Milestone 2 Task: Image Processing ---
        // (Insert your Convolution/Edge Detection code here, reading from source_buffer)

        // --- Write to VGA ---
        for (int i = 0; i < IMAGE_SIZE; i++) {
            IOWR(PIXEL_DAT_BASE, 0, source_buffer[i] >> 4);
            IOWR(IMG_ADDY_BASE, 0, i);
        }
    }
    return 0;
}
