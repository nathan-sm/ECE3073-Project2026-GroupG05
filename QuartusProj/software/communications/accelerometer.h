// ADXL345 accelerometer interface declarations for the communications Nios II core.
// Created By: Nathan Morris       (32532601)
//             Ritwam Shohaum      (33156816)
//             Shuk Kan LUI        (33891885)
//             Evgeny Solomin      (34977260)
// Created Date: 2026
// version = '1.0'

#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

// Holds raw signed 16-bit axis readings from the ADXL345
typedef struct {
    alt_16 xAxis, yAxis, zAxis;
} DeviceRotation;

// Initialises the ADXL345 over SPI and registers the interrupt handler.
// @return 0 on success
int accel_setup();

// Checks for a pending double-tap interrupt and updates the print counter.
// @return 0 on success
int accel_update();

// Reads all three axes from the ADXL345 in a single multi-byte SPI transaction.
// @return DeviceRotation struct with current xAxis, yAxis, zAxis values
DeviceRotation accel_get_device_rotation();

// Prints axis rotation data using the provided format string.
// @param message  printf-style format string expecting three int arguments (x, y, z)
// @param rotation Pointer to a DeviceRotation struct to print
void accel_print_device_rotation(const char *message, const DeviceRotation *rotation);

// Sets the callback function to invoke when a double-tap interrupt occurs.
// @param callback Function pointer to call on double-tap
void gyro_set_dtap_callback(void (*callback)());

#endif // ACCELEROMETER_H
