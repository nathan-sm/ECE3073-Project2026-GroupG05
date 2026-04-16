// Code adapted from Workshop4_SPI.c file

#include <stdio.h>
#include <stdint.h>
#include "system.h"
#include "io.h"

#include "altera_avalon_spi.h"
#include "altera_avalon_pio_regs.h"
#include "sys/alt_irq.h"

// Gyro write Registers
#define BW_RATE 0x2c
#define POWER_CONTROL 0x2d
#define DATA_FORMAT 0x31
#define INT_ENABLE 0x2E
#define INT_MAP 0x2F
#define THRESH_ACT 0x24
#define THRESH_INACT 0x25
#define TIME_INACT 0x26
#define ACT_INACT_CTL 0x27
#define THRESH_FF 0x28
#define TIME_FF 0x29
#define TAP_AXES 0x2a
#define TAP_THRES 0x1d
#define LATENT 0x22
#define DUR 0x21
#define WINDOW 0x23

// Gyro read Registers
#define INT_SOURCE 0x30
#define X_LB 0x32
#define X_HB 0x33
#define Y_LB 0x34
#define Y_HB 0x35
#define Z_LB 0x36
#define Z_HB 0x37

// Gyro read Registers
#define INT_SOURCE 0x30
#define X_LB 0x32
#define X_HB 0x33
#define Y_LB 0x34
#define Y_HB 0x35
#define Z_LB 0x36
#define Z_HB 0x37

#define CONFIG_LENGHT 16 * 2 // number of config details to send to initialise gyro

// flag for interrupt
volatile int flag_tap_double = 0;

#define MAX_COUNT 500000 // cycles to count to before printing

#define READ_X_AXIS 0xc0 | X_LB // enable read bit and multi byte
#define READ_Y_AXIS 0xc0 | Y_LB // enable read bit and multi byte
#define READ_Z_AXIS 0xc0 | Z_LB // enable read bit and multi byte

// define desired setup settings with write location then value to write
alt_u8 gyro_config[CONFIG_LENGHT] = {
    DATA_FORMAT, 0x0b, // 4-wire SPI, full resolution, +/- 16g
    THRESH_ACT, 0x04,
    THRESH_INACT, 0x02,
    TIME_INACT, 0x02,
	ACT_INACT_CTL, 0xff,
    THRESH_FF, 0x09,
    TIME_FF, 0x46,

	// double tap setting
    TAP_THRES, 0x1f,
    TAP_AXES, 0x07,
    LATENT, 0x85,
    DUR, 0x40,
    WINDOW, 0xc0,

	// interrupt
    BW_RATE, 0x0a,
    INT_ENABLE, 0x20,
    INT_MAP, 0x00,
    POWER_CONTROL, 0x08};


// ISR
static void gsens_isr(void* context){
	// edge capture reg on the Nios PIO to fire again
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(GSENS_INT_BASE, 0x3);

	// flag set up to let main know the interrupt occurred
	flag_tap_double = 1;
}



int main()
{
    printf("Start accel.\n"); // basic print statement to ensure UART is working

    // clear pending edge captures
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(GSENS_INT_BASE, 0x1);

    // enable PIO to send interrupts
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(GSENS_INT_BASE, 0x1);

    // reg interrupt - hardware IRQ line connection to gsens_isr function
    alt_ic_isr_register(GSENS_INT_IRQ_INTERRUPT_CONTROLLER_ID, GSENS_INT_IRQ, gsens_isr, NULL, 0);

    alt_irq_cpu_enable_interrupts();

    // accel. initialise
    alt_u8 gyro_data_out;

    // accel. select slave 1 (second argument)
    for (int i = 0; i < CONFIG_LENGHT; i +=2) {
    	alt_avalon_spi_command(SPI_0_BASE, 1, 2, gyro_config + i, 0, &gyro_data_out, 0);
    }

    printf("Accel. initialised\n");


    // clears the interrupt latency on teh accelerometer, when the reg is read
    alt_u8 clear_req = 0x80 | INT_SOURCE;
    alt_u8 response_discard;
    alt_avalon_spi_command(SPI_0_BASE, 1, 1, &clear_req, 1, &response_discard, 0);

    int print_counter = 0;

    // loop
    while(1){
    	// detect if interrupt has fired
    	if (flag_tap_double == 1) {
    		printf("Detected double tap\n");

			// reset flag
    		flag_tap_double = 0;

    		// read the int_source reg to clear interrupt on ADKL345
    		// read bit mask = 0x08
    		alt_u8 initial_clear_cmd = INT_SOURCE | 0x80;
    		alt_u8 initial_source_val;
    		alt_avalon_spi_command(SPI_0_BASE , 1, 1, &initial_clear_cmd, 1, &initial_source_val, 0);

    	}
    	// increment counter
    	print_counter++;
    	if (print_counter >=MAX_COUNT) {

    		// capture array for low and high byte from the SPI read
    		alt_u8 axis_data[6];
    		int16_t x_val, y_val, z_val;


    		// enabling both read bit (bit 7) and multi-byte bit (bit 6) with 0xC0
    		alt_u8 read_cmd = 0xC0 | X_LB;

    		alt_avalon_spi_command(SPI_0_BASE, 1, 1, &read_cmd, 6, axis_data, 0);

    		// 16 bit integers from 6 byte array, bit shifting

    		x_val = (int16_t)((axis_data[1] << 8) | axis_data[0]);
    		y_val = (int16_t)((axis_data[3] << 8) | axis_data[2]);
    		z_val = (int16_t)((axis_data[5] << 8) | axis_data[4]);


    		printf("X axis: %4d\t Y axis: %4d\t Z axis %4d\n",x_val,y_val,z_val);
    		print_counter = 0;

    	}


    }

    return 0;
}







