#include <stdint.h>
#include "system.h"
#include "io.h"
#include "sys/alt_irq.h"
#include <stdlib.h>

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

// task 2.2 functions

void convolve(uint8_t *inputImage, uint8_t *outputImage, int *kernel, int width, int height) {
	// iterate through every row and column skipping the borders
	for (int i = 1; i < height - 1; i++) {
		for (int j = 1; j < width - 1; j++) {
			// get the weighted pixel sum and kernel weight (normalisation)
			int sum = 0;
			int total_weight = 0;

			// looping over 3x3 neighbourhood around pixel (i,j)
			for (int ki = -1; ki <= 1; ki++) {
				for (int kj = -1; kj <= 1; kj++) {
						// get neighbouring pixel value
						int pixel = inputImage[(i + ki) * width + (j + kj)];
						// get matching kern3l weight (stored in row-major:index
						int weight = kernel[(ki + 1) * 3 + (kj + 1)];
						sum += pixel * weight;
						total_weight += weight;
					}
				}
				// avoid dividing by zero if kernel weights = 0
				if (total_weight == 0)total_weight = 1;
				// normalise
				int result = sum / total_weight;
				// restrict result to valid pixel range [0, 255]
				if (result < 0) result = 0;
				if (result > 255) result = 255;
				// write the result to the output buffer
				outputImage[i * width + j] = (uint8_t)result;
			}
		}
		// border pixels zero, skipped by convolution loop, setting to them to black
		// top, bottom, left , right
		for (int x = 0; x < width; x++) outputImage[x] = 0;
		for (int x = 0; x < width; x++) outputImage[(height-1)*width + x] = 0;
		for (int y = 0; y < height; y++) outputImage[y*width] = 0;
		for (int y = 0; y < height; y++) outputImage[y*width + (width - 1)] = 0;

}

void box_blur(uint8_t *input, uint8_t *output, int width, int height) {
	// 3x3 box kernel : all values = 1
	// convolve will divide by total_weight =9, giving average
	int blur_kernel[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};

	// run a pass of convolution with box kernel
	convolve(input, output, blur_kernel, width, height);
}

void sobel_edge_detection(uint8_t *input, uint8_t *output, int width, int height) {
	// detecting vertical (Gx) abd horizontal (Gy) edges (gradient in each direction)
	int kernel_x[9] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
	int kernel_y[9] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};

	// pixels with magnitude below this value are set to black
	// increase fewer, stronger edges - decrease for more, weaker edges
	int threshold = 60;

	for (int i = 1; i < height - 1; i++) {
		for (int j = 1; j < width -1; j++) {

			// horizontal and vertical gradient at pixel
			int gx = 0;
			int gy = 0;

			// compute gx and gy at the same time looping over 3x3
			for (int ki = -1; ki <= 1; ki++) {
				for (int kj = -1; kj <= 1; kj++) {
					int pixel = input[(i + ki) * width + (j + kj)];
					gx += pixel * kernel_x[(ki+1)*3 + (kj+1)];
					gy += pixel * kernel_y[(ki+1)*3 + (kj+1)];

				}
			}
			// combine gradients use magnitude values of gx and gy
			int magnitude = (gx < 0 ? -gx : gx) + (gy < 0 ? -gy : gy);

			// restrict magnitude to 255
			if (magnitude > 255) magnitude = 255;

			// apply threshold, keep pixel if strong, otherwise black
			output[i * width + j] = (magnitude >= threshold) ? (uint8_t)magnitude : 0;



		}
	}

	// zero border pixels
	for (int x = 0; x < width; x++) output[x] = 0;
	for (int x = 0; x < width; x++) output[(height -1)*width + x] = 0;
	for (int y = 0; y < height; y++) output[y*width] = 0;
	for (int y = 0; y < height; y++) output[y*width + (width-1)] = 0;

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

            // task 2.2 implement
            if (currently_displaying == 1) {
            	box_blur(buffers[0], buffers[1], 320, 240);
            }

            if (currently_displaying == 2) {
            	sobel_edge_detection(buffers[0], buffers[2], 320, 240);
            }

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
