// CPU 2: main.c (Communications)
#include <stdint.h>
#include "system.h"
#include "altera_avalon_spi.h"
#include "altera_avalon_mutex.h"
#include "accelerometer.h"

// Define a hardcoded address in SDRAM for the shared buffers.
// Ensure this address is outside the linker regions of BOTH processors.
#define SHARED_MEM_BASE (SDRAM_CONTROL_BASE + 0x01000000)
#define IMAGE_SIZE 76800

// Set up two ping-pong buffers in the shared memory space
uint8_t* buffer_A = (uint8_t*)(SHARED_MEM_BASE);
uint8_t* buffer_B = (uint8_t*)(SHARED_MEM_BASE + IMAGE_SIZE);

// Shared variable to tell CPU 1 which buffer has the newest full frame (0 = A, 1 = B)
volatile int* latest_ready_buffer = (volatile int*)(SHARED_MEM_BASE + (2 * IMAGE_SIZE));

int main() {
    // 1. Initialize Comms
    accel_setup();

    // 2. Open the Hardware Mutex
    alt_mutex_dev* mutex = altera_avalon_mutex_open("/dev/mutex_0");

    int current_write_buffer = 0; // Start by writing to Buffer A
    uint8_t* target_buffer;

    while(1) {
        // Point to the correct buffer
        target_buffer = (current_write_buffer == 0) ? buffer_A : buffer_B;

        // --- Fetch Frame via SPI ---
        uint8_t cmd = 0x10;
        alt_avalon_spi_command(SPI_0_BASE, 0, 1, &cmd, IMAGE_SIZE, target_buffer, 0);

        // --- Fetch Accelerometer Data ---
        accel_update();

        // --- Handshake with CPU 1 ---
        // Try to lock the mutex to safely update the shared pointer
        altera_avalon_mutex_lock(mutex, 1);

        *latest_ready_buffer = current_write_buffer; // Tell CPU 1 this buffer is ready
        current_write_buffer = !current_write_buffer; // Swap our write target for the next loop

        altera_avalon_mutex_unlock(mutex);
    }
    return 0;
}
