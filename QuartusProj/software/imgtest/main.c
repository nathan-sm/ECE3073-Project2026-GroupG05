#include <stdio.h>
#include "system.h"
#include "altera_avalon_pio_regs.h"

// Define image dimensions based on your VGA setup
#define IMG_WIDTH 320
#define IMG_HEIGHT 240
#define TOTAL_PIXELS (IMG_WIDTH * IMG_HEIGHT)

void write_test_pattern(int pattern_type) {
    int row, col;
    int address;
    int color_val;

    for (row = 0; row < IMG_HEIGHT; row++) {
        for (col = 0; col < IMG_WIDTH; col++) {

            // Calculate the 1D address: Address = Column + (Row * Width)
            address = col + (row * IMG_WIDTH);

            // Determine what color to write based on the pattern
            if (pattern_type == 0) {
                // Pattern 0: Solid White (Max 4-bit value is 15 or 0xF)
                color_val = 0xF;
            }
            else if (pattern_type == 1) {
                // Pattern 1: Solid Dark Gray
                color_val = 0x4;
            }
            else if (pattern_type == 2) {
                // Pattern 2: Vertical Stripes
                // Alternate between white (0xF) and black (0x0) every 16 pixels
                color_val = (col / 16) % 2 == 0 ? 0xF : 0x0;
            }
            else if (pattern_type == 3) {
                // Pattern 3: Gradient / Grayscale bars
                color_val = (col / 20) % 16;
            }

            // 1. Output the address to the Address PIO
            IOWR_ALTERA_AVALON_PIO_DATA(IMG_ADDY_BASE, address);

            // 2. Output the color data to the Pixel Data PIO
            IOWR_ALTERA_AVALON_PIO_DATA(PIXEL_DAT_BASE, color_val);
        }
    }
}

int main() {
    printf("Starting VGA Test...\n");

    // Draw vertical stripes to test the display
    write_test_pattern(3);

    printf("Test pattern written to buffer.\n");
    return 0;
}
