#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

typedef struct
{
	alt_16 x_axis, y_axis, z_axis;
} DeviceRotation;

int accel_setup();
int accel_update();

DeviceRotation accel_get_device_rotation();
void accel_print_device_rotation(const char *message, const DeviceRotation *rotation);

#endif // ACCELEROMETER_H
