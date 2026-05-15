// ADXL345 accelerometer driver for the communications Nios II core.
// Handles SPI initialisation, double-tap interrupts, and axis data reads.
// Created By: Nathan Morris       (32532601)
//             Ritwam Shohaum      (33156816)
//             Shuk Kan LUI        (33891885)
//             Evgeny Solomin      (34977260)
// Created Date: 2026
// version = '1.0'

#include <stdio.h>
#include <stdint.h>
#include "system.h"
#include "io.h"

#include "altera_avalon_spi.h"
#include "altera_avalon_pio_regs.h"
#include "sys/alt_irq.h"

#include "accelerometer.h"

// Gyro write registers
#define BW_RATE         0x2c
#define POWER_CONTROL   0x2d
#define DATA_FORMAT     0x31
#define INT_ENABLE      0x2E
#define INT_MAP         0x2F
#define THRESH_ACT      0x24
#define THRESH_INACT    0x25
#define TIME_INACT      0x26
#define ACT_INACT_CTL   0x27
#define THRESH_FF       0x28
#define TIME_FF         0x29
#define TAP_AXES        0x2a
#define TAP_THRES       0x1d
#define LATENT          0x22
#define DUR             0x21
#define WINDOW          0x23

// Gyro read registers
#define INT_SOURCE  0x30
#define X_LB        0x32
#define X_HB        0x33
#define Y_LB        0x34
#define Y_HB        0x35
#define Z_LB        0x36
#define Z_HB        0x37

// Number of bytes in the gyro configuration array (address + value pairs)
#define CONFIG_LENGTH (16 * 2)

// Number of accel_update calls between consecutive axis prints
#define MAX_COUNT 10

// Bit masks for reading multiple bytes: bit 7 = read, bit 6 = multi-byte
#define READ_MULTI_BYTE_MASK 0xC0

#define READ_X_AXIS (READ_MULTI_BYTE_MASK | X_LB)
#define READ_Y_AXIS (READ_MULTI_BYTE_MASK | Y_LB)
#define READ_Z_AXIS (READ_MULTI_BYTE_MASK | Z_LB)

// Configuration array: each pair is (register address, value to write)
alt_u8 gyroConfig[CONFIG_LENGTH] = {
    DATA_FORMAT, 0x0b,      // 4-wire SPI, full resolution, +/- 16g
    THRESH_ACT, 0x04,
    THRESH_INACT, 0x02,
    TIME_INACT, 0x02,
    ACT_INACT_CTL, 0xff,
    THRESH_FF, 0x09,
    TIME_FF, 0x46,

    // double tap settings
    TAP_THRES, 0x1f,
    TAP_AXES, 0x07,
    LATENT, 0x85,
    DUR, 0x40,
    WINDOW, 0xc0,

    // interrupt configuration
    BW_RATE, 0x0a,
    INT_ENABLE, 0x20,
    INT_MAP, 0x00,
    POWER_CONTROL, 0x08
};

static int gPrintCounter = 0;

// Flag set by the ISR when a double-tap interrupt fires
volatile int flagTapDouble = 0;

// User-supplied callback invoked on double-tap; defaults to a debug print
void (*gDoubleTapCallback)() = 0;

// Default double-tap callback — prints a notification to the JTAG UART
void doubletap_callback_default() {
    printf("Detected double tap\n");
}

// ISR triggered by the GSENSOR_INT PIO edge-capture interrupt
static void gsens_isr(void* context) {
    // Re-arm the edge capture register so the next tap fires again
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(GSENS_INT_BASE, 0x3);
    flagTapDouble = 1;
}

// Initialises the ADXL345 over SPI and registers the interrupt handler.
// @return 0 on success
int accel_setup() {
    // Clear any pending edge captures before enabling the interrupt
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(GSENS_INT_BASE, 0x1);

    // Enable the PIO to generate interrupts on pin edge
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(GSENS_INT_BASE, 0x1);

    // Connect the hardware IRQ line to the gsens_isr handler
    alt_ic_isr_register(GSENS_INT_IRQ_INTERRUPT_CONTROLLER_ID, GSENS_INT_IRQ, gsens_isr, NULL, 0);

    alt_irq_cpu_enable_interrupts();

    // Write all configuration register pairs to the accelerometer (slave 1)
    alt_u8 gyroDataOut;
    for (int i = 0; i < CONFIG_LENGTH; i += 2) {
        alt_avalon_spi_command(SPI_0_BASE, 1, 2, gyroConfig + i, 0, &gyroDataOut, 0);
    }

    gyro_set_dtap_callback(&doubletap_callback_default);

    printf("Accelerometer initialised\n");

    // Reading INT_SOURCE clears the latched interrupt on the ADXL345
    alt_u8 clearReq = 0x80 | INT_SOURCE;
    alt_u8 responseDiscard;
    alt_avalon_spi_command(SPI_0_BASE, 1, 1, &clearReq, 1, &responseDiscard, 0);

    return 0;
}

// Checks for a pending double-tap interrupt and updates the print counter.
// @return 0 on success
int accel_update() {
    if (flagTapDouble == 1) {
        flagTapDouble = 0;

        // Reading INT_SOURCE clears the interrupt latch on the ADXL345
        alt_u8 initialClearCmd = INT_SOURCE | 0x80;
        alt_u8 initialSourceVal;
        alt_avalon_spi_command(SPI_0_BASE, 1, 1, &initialClearCmd, 1, &initialSourceVal, 0);

        if (gDoubleTapCallback) {
            gDoubleTapCallback();
        }
    }

    gPrintCounter++;
    if (gPrintCounter >= MAX_COUNT) {
        DeviceRotation rotation = accel_get_device_rotation();
        accel_print_device_rotation("X axis: %4d\t Y axis: %4d\t Z axis %4d\n", &rotation);
        gPrintCounter = 0;
    }

    return 0;
}

// Reads all three axes from the ADXL345 in a single multi-byte SPI transaction.
// @return DeviceRotation struct with current xAxis, yAxis, zAxis values
DeviceRotation accel_get_device_rotation() {
    DeviceRotation result;

    // 6-byte buffer: low then high byte for each axis (X, Y, Z)
    alt_u8 axisData[6];

    // Bit 7 = read, bit 6 = multi-byte; start read from X_LB
    alt_u8 readCmd = READ_MULTI_BYTE_MASK | X_LB;

    alt_avalon_spi_command(SPI_0_BASE, 1, 1, &readCmd, 6, axisData, 0);

    // Combine low and high bytes into signed 16-bit values
    result.xAxis = (((int16_t)(axisData[1]) << 8) | axisData[0]);
    result.yAxis = (((int16_t)(axisData[3]) << 8) | axisData[2]);
    result.zAxis = (((int16_t)(axisData[5]) << 8) | axisData[4]);

    return result;
}

// Prints axis rotation data using the provided format string.
// @param message  printf-style format string expecting three int arguments (x, y, z)
// @param rotation Pointer to a DeviceRotation struct to print
void accel_print_device_rotation(const char *message, const DeviceRotation *rotation) {
    printf(message, rotation->xAxis, rotation->yAxis, rotation->zAxis);
}

// Sets the callback function to invoke when a double-tap interrupt occurs.
// @param callback Function pointer to call on double-tap
void gyro_set_dtap_callback(void (*callback)()) {
    gDoubleTapCallback = callback;
}
