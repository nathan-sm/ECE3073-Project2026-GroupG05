#include <stdio.h>
#include <stdint.h>
#include "system.h"
#include "io.h"
#include "altera_avalon_spi.h"

// SPI Slave Indices
#define ESP_CAM_SS      0       // spi_ss_n[0]
#define ACCEL_SS        1       // spi_ss_n[1]

// Image Parameters
#define IMAGE_WIDTH     320
#define IMAGE_HEIGHT    240
#define IMAGE_SIZE      (IMAGE_WIDTH * IMAGE_HEIGHT)    // 76,800 pixels

// ESP-CAM Command Byte
// Bit 0: 0 = Greyscale,   1 = 4-bit RGB
// Bit 1: 0 = 320x240,     1 = 160x120
// Bit 2: 0 = Raw stream,  1 = Packed stream
// Bit 3: 0 = Flash off,   1 = Flash on
// Bit 4: 0 = No change,   1 = Apply settings (must be set for bits 0-3 to register)
#define CAM_CMD_DEFAULT 0x10    // Greyscale

// Global buffer placed in SDRAM
uint8_t image_buffer[IMAGE_SIZE];

// Send command byte then receive a full 320x240 frame into the pixel buffer
void receive_frame(uint8_t cmd) {

    alt_avalon_spi_command(SPI_0_BASE, ESP_CAM_SS,
                           1, &cmd,
                           IMAGE_SIZE, image_buffer,
                           0);

    for (uint32_t i = 0; i < IMAGE_SIZE; i++) {
        IOWR(PIXEL_DAT_BASE, 0, image_buffer[i] >> 4);
        IOWR(IMG_ADDY_BASE,  0, i);
    }
}

int main(void) {
    printf("ESP-CAM SPI initialised\n");

    // Send startup command to put camera into a known state
    uint8_t startup_cmd = CAM_CMD_DEFAULT;

    alt_avalon_spi_command(SPI_0_BASE, ESP_CAM_SS,
                           1, &startup_cmd,
                           0, NULL, 0);

    while (1) {
        // Wait for CAM_READY signal (GPIO[2] via cam_redy PIO)
        while (IORD(CAM_REDY_BASE, 0) == 0);

        // Fetch and display the frame
        receive_frame(CAM_CMD_DEFAULT);

        printf("Frame written to pixel buffer\n");
    }

    return 0;
}
