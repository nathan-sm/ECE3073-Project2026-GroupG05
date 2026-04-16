#include <stdio.h>
#include <stdint.h>
#include "system.h"
#include "io.h"
#include "altera_avalon_spi.h"

#include "accelerometer.h"

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

// ---- TASK 4: FPS display ----
void display_fps(uint32_t elapsed) {
	// Stores 100 x FPS so that we can do integer division
    uint32_t fps_100 = 100000000 / elapsed;

//    printf("100x FPS: %lu, elapsed: %lu\n", fps_100, elapsed);

    const uint8_t SEG[10] = {
        0xC0, // 0
        0xF9, // 1
        0xA4, // 2
        0xB0, // 3
        0x99, // 4
        0x92, // 5
        0x82, // 6
        0xF8, // 7
        0x80, // 8
        0x90  // 9
    };

    int d3 = (fps_100 / 1000) % 10;
    int d2 = (fps_100 / 100)  % 10;
    int d1 = (fps_100 / 10)   % 10;
    int d0 =  fps_100         % 10;

    uint8_t hex3 = SEG[d3];
    uint8_t hex2 = SEG[d2] & ~(1 << 7); // decimal point on HEX2
    uint8_t hex1 = SEG[d1];
    uint8_t hex0 = SEG[d0];

    uint32_t hex0_2 = ((uint32_t)hex2 << 16) | ((uint32_t)hex1 << 8) | hex0;
    uint32_t hex3_5 = (0xFF << 16) | (0xFF << 8) | hex3;

    IOWR(HEX20_BASE, 0, hex0_2);
    IOWR(HEX53_BASE, 0, hex3_5);
}

// Send command byte then receive a full 320x240 frame into the pixel buffer
void receive_frame(uint8_t cmd)
{
    unsigned int t_start = IORD(USEC_COUNTER_BASE, 0); // TASK 4: start timer

    alt_avalon_spi_command(SPI_0_BASE, ESP_CAM_SS,
                           1, &cmd,
                           IMAGE_SIZE, image_buffer,
                           0);


    for (uint32_t i = 0; i < IMAGE_SIZE; i++) {
        IOWR(PIXEL_DAT_BASE, 0, image_buffer[i] >> 4);
        IOWR(IMG_ADDY_BASE,  0, i);
    }

//    for (uint32_t i = 0; i < 0xfffff; i++);

    display_fps(IORD(USEC_COUNTER_BASE, 0) - t_start); // TASK 4: display FPS
}

int main(void) {
    printf("ESP-CAM SPI initialised\n");

    // Send startup command to put camera into a known state
    uint8_t startup_cmd = CAM_CMD_DEFAULT;

    alt_avalon_spi_command(SPI_0_BASE, ESP_CAM_SS,
                           1, &startup_cmd,
                           0, NULL, 0);

    if (setup_gyro())
    {
    	printf("Gyro init failed.\n");
    	return 1;
    }

    while (1) {
        // Wait for CAM_READY signal (GPIO[2] via cam_redy PIO)
        if (IORD(CAM_REDY_BASE, 0) == 1)
        {
			// Fetch and display the frame
			receive_frame(CAM_CMD_DEFAULT);
        }

//        printf("Frame written to pixel buffer\n");

        if (update_gyro())
        {
        	printf("Gyro read failed\n");
        	return 2;
        }
    }

    return 0;
}
