#include <stdbool.h>
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
#define DISPLAY_WIDTH		320
#define DISPLAY_HEIGHT		240
#define FULL_IMAGE_WIDTH    320
#define FULL_IMAGE_HEIGHT   240
#define FULL_IMAGE_SIZE     (FULL_IMAGE_WIDTH * FULL_IMAGE_HEIGHT)    // 76,800 pixels
#define QUAD_IMAGE_WIDTH	(FULL_IMAGE_WIDTH/2)
#define QUAD_IMAGE_HEIGHT	(FULL_IMAGE_HEIGHT/2)
#define QUAD_IMAGE_SIZE		(QUAD_IMAGE_WIDTH * QUAD_IMAGE_HEIGHT)
#define QUAD_IMAGE_BUF_SIZE (QUAD_IMAGE_SIZE * 4)

// ESP-CAM Command Byte
// Bit 0: 0 = Greyscale,   1 = 4-bit RGB
// Bit 1: 0 = 320x240,     1 = 160x120
// Bit 2: 0 = Raw stream,  1 = Packed stream
// Bit 3: 0 = Flash off,   1 = Flash on
// Bit 4: 0 = No change,   1 = Apply settings (must be set for bits 0-3 to register)
#define CAM_GREY_MASK 0x1
#define CAM_QUAD_MASK 0x2
#define CAM_PACK_MASK 0x4
#define CAM_FLASH_MASK 0x8
#define CAM_WRITE_MASK 0x10;
#define CAM_CMD_DEFAULT 0x0    // Greyscale
uint8_t g_camLastConfig = 0x0;

// Global buffer placed in SDRAM
uint8_t full_image_buffer[FULL_IMAGE_SIZE];
uint8_t quad_image_buffer[QUAD_IMAGE_BUF_SIZE];

// Array of which image to display in each position in quad display mode
uint32_t g_quadDisplayIndices[4] = { 0, 1, 2, 3 };

/** Display an FPS value on the 7-segment displays
 *
 * @param elapsed Time taken for the frame, in ms.
 */
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

/** Write a frame from the SPI-CAM to SDRAM
 *
 * 	@param isQuad Boolean value for whether to receive a quarter size frame
 */
void receive_frame(bool isQuad)
{
	uint8_t camCmd = g_camLastConfig;
	uint8_t camFlag = isQuad ? CAM_QUAD_MASK : 0x0;
	if ((g_camLastConfig & CAM_QUAD_MASK) != camFlag)
	{
		if (isQuad)
		{
			camCmd |= CAM_QUAD_MASK;
		}
		else
		{
			camCmd &= ~CAM_QUAD_MASK;
		}

		g_camLastConfig = camCmd;

		camCmd |= CAM_WRITE_MASK;
	}

	uint32_t readSize = isQuad ? QUAD_IMAGE_SIZE : FULL_IMAGE_SIZE;
	uint8_t *readDest = isQuad ? quad_image_buffer : full_image_buffer;
    alt_avalon_spi_command(SPI_0_BASE, ESP_CAM_SS,
                           1, &camCmd,
                           readSize, readDest,
                           0);
}

/** Display the full image currently in the full_image_buffer.
 *
 * @returns Time taken to transfer image, in ms.
 */
uint32_t display_full_image()
{
	unsigned int t_start = IORD(USEC_COUNTER_BASE, 0);

    for (uint32_t i = 0; i < FULL_IMAGE_SIZE; i++) {
        IOWR(IMG_ADDY_BASE,  0, i);
        IOWR(PIXEL_DAT_BASE, 0, full_image_buffer[i] >> 4);
    }

    return IORD(USEC_COUNTER_BASE, 0) - t_start;
}

/** Display one of the images in quad_display_buffer on the screen.
 *
 * @param imageIndex Which image to display
 * @param displayIndex Where on the display to place the image (0 - top left, 1 - top right, 2 - bottom left, 3 - bottom right)
 *
 * @returns Time taken to transfer images, in ms.
 */
uint32_t display_quad_image(uint32_t imageIndex, uint32_t displayIndex)
{
    unsigned int t_start = IORD(USEC_COUNTER_BASE, 0);

	uint32_t imgAddr = imageIndex * QUAD_IMAGE_SIZE;
	uint32_t pixelBufferAddr = 0;

	// If on the right, move destination over by half the display width
	if (displayIndex & 0x1)
	{
		pixelBufferAddr += QUAD_IMAGE_WIDTH;
	}

	// If on the bottom, move destination down by half the display height
	if (displayIndex & 0x2)
	{
		pixelBufferAddr += QUAD_IMAGE_SIZE * 2;
	}

	// Double loop over quad-size image, by lines then pixels
	for (uint32_t i = 0; i < QUAD_IMAGE_HEIGHT; i++)
	{
		for (uint32_t j = 0; j < QUAD_IMAGE_WIDTH; j++)
		{
			IOWR(IMG_ADDY_BASE,  0, pixelBufferAddr);
			IOWR(PIXEL_DAT_BASE, 0, quad_image_buffer[imgAddr] >> 4);

			imgAddr++;
			pixelBufferAddr++;
		}

		// At the end of each line, move the destination to the next line of the display
		pixelBufferAddr += FULL_IMAGE_WIDTH;
	}

    return IORD(USEC_COUNTER_BASE, 0) - t_start;
}

void doubletap_callback()
{
	uint16_t swStatus = IORD(SW_BASE, 0);
	bool isQuad = swStatus & 0x1;

	if (!isQuad)
		return;

	DeviceRotation deviceRotation = accel_get_device_rotation();

	uint8_t imageX = deviceRotation.x_axis < 0 ? 0 : 1;
	uint8_t imageY = deviceRotation.y_axis < 0 ? 0 : 1;

	uint8_t imageIndex = (imageY << 1) | imageX;

	g_quadDisplayIndices[imageIndex] = (g_quadDisplayIndices[imageIndex] + 1) % 4;
}

int main(void) {
    printf("ESP-CAM SPI initialised\n");

    // Send startup command to put camera into a known state
    uint8_t startup_cmd = CAM_CMD_DEFAULT | CAM_WRITE_MASK;

    alt_avalon_spi_command(SPI_0_BASE, ESP_CAM_SS,
                           1, &startup_cmd,
                           0, NULL, 0);

    // Initialize full image buffer
    for (int i = 0; i < FULL_IMAGE_SIZE; i++)
    {
    	full_image_buffer[i] = 0x00;
    }

    // Initialize quad image buffer
    for (int i = 0; i < QUAD_IMAGE_BUF_SIZE; i++)
    {
    	quad_image_buffer[i] = 0x00;
    }

    for (int i = 0; i < FULL_IMAGE_SIZE; i++)
    {
    	full_image_buffer[i] = 0x00;
    }

    for (int i = 0; i < QUAD_IMAGE_BUF_SIZE; i++)
    {
    	quad_image_buffer[i] = 0xff;
    }

    if (accel_setup())
    {
    	printf("Gyro init failed.\n");
    	return 1;
    }

    while (1) {
        // Wait for CAM_READY signal (GPIO[2] via cam_redy PIO)
        if (IORD(CAM_REDY_BASE, 0) == 1)
        {
        	uint16_t swStatus = IORD(SW_BASE, 0);
        	bool isQuad = swStatus & 0x1;

			// Fetch and display the frame
			receive_frame(isQuad);

			uint32_t frameWriteTime = 0;
			if (isQuad)
			{
				frameWriteTime += display_quad_image(g_quadDisplayIndices[0], 0);
				frameWriteTime += display_quad_image(g_quadDisplayIndices[1], 0);
				frameWriteTime += display_quad_image(g_quadDisplayIndices[2], 0);
				frameWriteTime += display_quad_image(g_quadDisplayIndices[3], 0);
			}
			else
			{
				frameWriteTime = display_full_image();
			}

			display_fps(frameWriteTime);
        }

//        printf("Frame written to pixel buffer\n");

        if (accel_update())
        {
        	printf("Gyro read failed\n");
        }
    }

    return 0;
}
