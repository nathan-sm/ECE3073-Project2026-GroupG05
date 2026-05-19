#include <stdint.h>
#include <string.h>
#include "system.h"
#include "io.h"
#include "sys/alt_irq.h"
#include <stdlib.h>

#define SHARED_MEM_BASE (SDRAM_CONTROL_BASE + 0x01000000)

#define IMAGE_PIXELS 76800
#define IMAGE_BYTES  115200
#define QUAD_IMAGE_WIDTH  160
#define QUAD_IMAGE_HEIGHT 120
#define QUAD_PIXELS  19200
#define QUAD_BYTES   28800
#define FULL_IMAGE_WIDTH  320
#define FULL_IMAGE_HEIGHT 240

// Processing modes (selected by SW[2:1])
#define PROC_RAW   0
#define PROC_FLIP  1
#define PROC_BLUR  2
#define PROC_EDGE  3

uint8_t* buffers[3] = {
    (uint8_t*)(SHARED_MEM_BASE),
    (uint8_t*)(SHARED_MEM_BASE + IMAGE_BYTES),
    (uint8_t*)(SHARED_MEM_BASE + (2 * IMAGE_BYTES))
};

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} SharedAccelData;

typedef struct {
    volatile uint8_t isQuad;
    volatile uint8_t _pad[3];
    volatile uint32_t quadDisplayIndices[4];
} SharedDisplayState;

SharedAccelData* shared_accel = (SharedAccelData*)((SHARED_MEM_BASE + (3 * IMAGE_BYTES)) | 0x80000000);
SharedDisplayState* shared_display = (SharedDisplayState*)((SHARED_MEM_BASE + (3 * IMAGE_BYTES) + sizeof(SharedAccelData) + 8) | 0x80000000);

// Global arrays for unpacking and processing
uint16_t unpacked_rgb[IMAGE_PIXELS];
uint16_t processing_buffer[IMAGE_PIXELS];

// Temporary buffers for RGB split/grayscale processing
uint8_t grayscale_buffer[IMAGE_PIXELS];
uint8_t edge_buffer[IMAGE_PIXELS];
uint8_t r_chan[IMAGE_PIXELS], g_chan[IMAGE_PIXELS], b_chan[IMAGE_PIXELS];
uint8_t r_blur[IMAGE_PIXELS], g_blur[IMAGE_PIXELS], b_blur[IMAGE_PIXELS];

volatile int new_frame_ready = 0;
volatile uint32_t current_frame_index = 0;

static void mailbox_rx_isr(void* context) {
    current_frame_index = IORD(DATA_MAILBOX_BASE, 0);
    new_frame_ready = 1;
}

void display_fps(uint32_t elapsed) {
    if (elapsed == 0) return;
    uint32_t fps_100 = 100000000 / elapsed;
    const uint8_t SEG[10] = { 0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90 };

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

// ---- Image Processing Functions ----
void process_flip(uint16_t *input, uint16_t *output, int width, int height) {
    int totalPixels = width * height;
    for (int i = 0; i < totalPixels; i++) {
        output[i] = input[totalPixels - 1 - i];
    }
}

void convolve(uint8_t *inputImage, uint8_t *outputImage, int *kernel, int width, int height) {
    for (int i = 1; i < height - 1; i++) {
        for (int j = 1; j < width - 1; j++) {
            int sum = 0;
            int total_weight = 0;
            for (int ki = -1; ki <= 1; ki++) {
                for (int kj = -1; kj <= 1; kj++) {
                    int pixel = inputImage[(i + ki) * width + (j + kj)];
                    int weight = kernel[(ki + 1) * 3 + (kj + 1)];
                    sum += pixel * weight;
                    total_weight += weight;
                }
            }
            if (total_weight == 0) total_weight = 1;
            int result = sum / total_weight;
            if (result < 0) result = 0;
            if (result > 255) result = 255;
            outputImage[i * width + j] = (uint8_t)result;
        }
    }
    for (int x = 0; x < width; x++) outputImage[x] = 0;
    for (int x = 0; x < width; x++) outputImage[(height-1)*width + x] = 0;
    for (int y = 0; y < height; y++) outputImage[y*width] = 0;
    for (int y = 0; y < height; y++) outputImage[y*width + (width - 1)] = 0;
}

void sobel_edge_detection(uint8_t *input, uint8_t *output, int width, int height) {
    int kernel_x[9] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
    int kernel_y[9] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};
    int threshold = 60;

    for (int i = 1; i < height - 1; i++) {
        for (int j = 1; j < width -1; j++) {
            int gx = 0;
            int gy = 0;
            for (int ki = -1; ki <= 1; ki++) {
                for (int kj = -1; kj <= 1; kj++) {
                    int pixel = input[(i + ki) * width + (j + kj)];
                    gx += pixel * kernel_x[(ki+1)*3 + (kj+1)];
                    gy += pixel * kernel_y[(ki+1)*3 + (kj+1)];
                }
            }
            int magnitude = (gx < 0 ? -gx : gx) + (gy < 0 ? -gy : gy);
            if (magnitude > 255) magnitude = 255;
            output[i * width + j] = (magnitude >= threshold) ? (uint8_t)magnitude : 0;
        }
    }
    for (int x = 0; x < width; x++) output[x] = 0;
    for (int x = 0; x < width; x++) output[(height -1)*width + x] = 0;
    for (int y = 0; y < height; y++) output[y*width] = 0;
    for (int y = 0; y < height; y++) output[y*width + (width-1)] = 0;
}

// --- RGB Processing Wrappers ---
void process_rgb_blur(uint16_t* rgb_in, uint16_t* rgb_out, int width, int height) {
    int total = width * height;
    int blur_kernel[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};

    // Split
    for(int i = 0; i < total; i++) {
        r_chan[i] = (rgb_in[i] >> 8) & 0xF;
        g_chan[i] = (rgb_in[i] >> 4) & 0xF;
        b_chan[i] = rgb_in[i] & 0xF;
    }

    // Blur separately
    convolve(r_chan, r_blur, blur_kernel, width, height);
    convolve(g_chan, g_blur, blur_kernel, width, height);
    convolve(b_chan, b_blur, blur_kernel, width, height);

    // Recombine back to 12-bit
    for(int i = 0; i < total; i++) {
        rgb_out[i] = ((r_blur[i] & 0xF) << 8) | ((g_blur[i] & 0xF) << 4) | (b_blur[i] & 0xF);
    }
}

void process_rgb_edges(uint16_t* rgb_in, uint16_t* rgb_out, int width, int height) {
    int total = width * height;

    // 1. Convert to Grayscale using milestone formula
    for(int i = 0; i < total; i++) {
        uint16_t p = rgb_in[i];
        uint8_t r = ((p >> 8) & 0xF) * 17;
        uint8_t g = ((p >> 4) & 0xF) * 17;
        uint8_t b = (p & 0xF) * 17;
        grayscale_buffer[i] = (r * 76 + g * 150 + b * 29) >> 8;
    }

    // 2. Run Edge Detection
    sobel_edge_detection(grayscale_buffer, edge_buffer, width, height);

    // 3. Map 8-bit edge back to 12-bit RGB (White or Black)
    for(int i = 0; i < total; i++) {
        uint8_t val = edge_buffer[i] >> 4;
        rgb_out[i] = (val << 8) | (val << 4) | val;
    }
}

// ---- Display Functions ----
void display_full_image(uint16_t *buffer) {
    volatile int* const pixel_dat_ptr = (volatile int*)(PIXEL_DAT_BASE | 0x80000000);
    volatile int* const img_addy_ptr  = (volatile int*)(IMG_ADDY_BASE  | 0x80000000);

    uint32_t* src_ptr32 = (uint32_t*)((uint32_t)buffer | 0x80000000);
    uint32_t* const src_end32 = (uint32_t*)(((uint32_t)buffer + (IMAGE_PIXELS * 2)) | 0x80000000);

    int addr = 0;

    while (src_ptr32 < src_end32) {
        uint32_t block = *src_ptr32++;

        *img_addy_ptr = addr++;
        *pixel_dat_ptr = block & 0x0FFF; // Bottom 12 bits

        *img_addy_ptr = addr++;
        *pixel_dat_ptr = (block >> 16) & 0x0FFF; // Top 12 bits
    }
}

void display_quad_image(uint16_t *buffer, uint32_t imageIndex, uint32_t displayIndex) {
    volatile int* const pixel_dat_ptr = (volatile int*)(PIXEL_DAT_BASE | 0x80000000);
    volatile int* const img_addy_ptr  = (volatile int*)(IMG_ADDY_BASE  | 0x80000000);

    uint16_t* src = (uint16_t*)((uint32_t)buffer | 0x80000000);

    uint32_t imgAddr = imageIndex * QUAD_PIXELS;
    uint32_t pixelBufferAddr = 0;

    if (displayIndex & 0x1) pixelBufferAddr += QUAD_IMAGE_WIDTH;
    if (displayIndex & 0x2) pixelBufferAddr += QUAD_PIXELS * 2;

    for (uint32_t i = 0; i < QUAD_IMAGE_HEIGHT; i++) {
        for (uint32_t j = 0; j < QUAD_IMAGE_WIDTH; j++) {
            *img_addy_ptr  = pixelBufferAddr;
            *pixel_dat_ptr = src[imgAddr]; // No shift needed
            imgAddr++;
            pixelBufferAddr++;
        }
        pixelBufferAddr += FULL_IMAGE_WIDTH - QUAD_IMAGE_WIDTH;
    }
}

int main() {
    int currently_displaying = -1;
    uint32_t last_frame_time = 0;

    alt_ic_isr_register(DATA_MAILBOX_IRQ_INTERRUPT_CONTROLLER_ID, DATA_MAILBOX_IRQ, mailbox_rx_isr, NULL, NULL);
    IOWR(DATA_MAILBOX_BASE, 3, 0x01);

    while(1) {
        shared_display->isQuad = (IORD(SW_BASE, 0) & 0x1) ? 1 : 0;

        if (new_frame_ready) {
            new_frame_ready = 0;

            uint32_t current_time = IORD(USEC_COUNTER_BASE, 0);
            if (last_frame_time != 0) {
                display_fps(current_time - last_frame_time);
            }
            last_frame_time = current_time;

            if (currently_displaying != -1) {
                while (IORD(ACK_MAILBOX_BASE, 2) & 0x02);
                IOWR(ACK_MAILBOX_BASE, 0, currently_displaying);
            }

            currently_displaying = current_frame_index;

            // 1. Always grab the full frame buffer
            uint8_t* source_packed = (uint8_t*)((uint32_t)buffers[currently_displaying] | 0x80000000);

            uint8_t isQuad = shared_display->isQuad;
            int processMode = (IORD(SW_BASE, 0) >> 1) & 0x3;

            // ALWAYS execute unpacking across the full 320x240 data payload
            int num_pixels = IMAGE_PIXELS;

            int byte_idx = 0;
            for (int i = 0; i < num_pixels; i += 2) {
                uint8_t b0 = source_packed[byte_idx++];
                uint8_t b1 = source_packed[byte_idx++];
                uint8_t b2 = source_packed[byte_idx++];

                uint16_t p0_raw = (b0 << 4) | (b1 >> 4);
                uint16_t p1_raw = ((b1 & 0x0F) << 8) | b2;

                unpacked_rgb[i]   = (((p0_raw >> 4) & 0xF) << 8) | ((p0_raw & 0xF) << 4) | ((p0_raw >> 8) & 0xF);
                unpacked_rgb[i+1] = (((p1_raw >> 4) & 0xF) << 8) | ((p1_raw & 0xF) << 4) | ((p1_raw >> 8) & 0xF);
            }

            if (isQuad) {
                // SOFTWARE DOWNSAMPLING: Manually scale the full 320x240 image down into a 160x120 array
                uint16_t* quad_source = processing_buffer; // Draw straight into the first quadrant slot
                for (int y = 0; y < QUAD_IMAGE_HEIGHT; y++) {
                    for (int x = 0; x < QUAD_IMAGE_WIDTH; x++) {
                        quad_source[y * QUAD_IMAGE_WIDTH + x] = unpacked_rgb[(y * 2) * FULL_IMAGE_WIDTH + (x * 2)];
                    }
                }

                // Slot 1: Flip (processed from the clean quad_source variable)
                process_flip(quad_source, processing_buffer + QUAD_PIXELS, QUAD_IMAGE_WIDTH, QUAD_IMAGE_HEIGHT);

                // Slot 2: Blur
                process_rgb_blur(quad_source, processing_buffer + (2 * QUAD_PIXELS), QUAD_IMAGE_WIDTH, QUAD_IMAGE_HEIGHT);

                // Slot 3: Edge
                process_rgb_edges(quad_source, processing_buffer + (3 * QUAD_PIXELS), QUAD_IMAGE_WIDTH, QUAD_IMAGE_HEIGHT);

                display_quad_image(processing_buffer, shared_display->quadDisplayIndices[0], 0);
                display_quad_image(processing_buffer, shared_display->quadDisplayIndices[1], 1);
                display_quad_image(processing_buffer, shared_display->quadDisplayIndices[2], 2);
                display_quad_image(processing_buffer, shared_display->quadDisplayIndices[3], 3);
            }
            else {
                switch (processMode) {
                    case PROC_FLIP:
                        process_flip(unpacked_rgb, processing_buffer, FULL_IMAGE_WIDTH, FULL_IMAGE_HEIGHT);
                        display_full_image(processing_buffer);
                        break;
                    case PROC_BLUR:
                        process_rgb_blur(unpacked_rgb, processing_buffer, FULL_IMAGE_WIDTH, FULL_IMAGE_HEIGHT);
                        display_full_image(processing_buffer);
                        break;
                    case PROC_EDGE:
                        process_rgb_edges(unpacked_rgb, processing_buffer, FULL_IMAGE_WIDTH, FULL_IMAGE_HEIGHT);
                        display_full_image(processing_buffer);
                        break;
                    default:
                        display_full_image(unpacked_rgb);
                        break;
                }
            }
        }
    }
    return 0;
}
