/*
 * system.h - SOPC Builder system and BSP software package information
 *
 * Machine generated for CPU 'nios2_gen2_0' in SOPC Builder design 'NiosSystem'
 * SOPC Builder design path: ../../NiosSystem.sopcinfo
 *
 * Generated: Mon Mar 30 08:28:53 EST 2026
 */

/*
 * DO NOT MODIFY THIS FILE
 *
 * Changing this file will have subtle consequences
 * which will almost certainly lead to a nonfunctioning
 * system. If you do modify this file, be aware that your
 * changes will be overwritten and lost when this file
 * is generated again.
 *
 * DO NOT MODIFY THIS FILE
 */

/*
 * License Agreement
 *
 * Copyright (c) 2008
 * Altera Corporation, San Jose, California, USA.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This agreement shall be governed in all respects by the laws of the State
 * of California and by the laws of the United States of America.
 */

#ifndef __SYSTEM_H_
#define __SYSTEM_H_

/* Include definitions from linker script generator */
#include "linker.h"


/*
 * CPU configuration
 *
 */

#define ALT_CPU_ARCHITECTURE "altera_nios2_gen2"
#define ALT_CPU_BIG_ENDIAN 0
#define ALT_CPU_BREAK_ADDR 0x04040820
#define ALT_CPU_CPU_ARCH_NIOS2_R1
#define ALT_CPU_CPU_FREQ 50000000u
#define ALT_CPU_CPU_ID_SIZE 1
#define ALT_CPU_CPU_ID_VALUE 0x00000000
#define ALT_CPU_CPU_IMPLEMENTATION "fast"
#define ALT_CPU_DATA_ADDR_WIDTH 0x1b
#define ALT_CPU_DCACHE_BYPASS_MASK 0x80000000
#define ALT_CPU_DCACHE_LINE_SIZE 32
#define ALT_CPU_DCACHE_LINE_SIZE_LOG2 5
#define ALT_CPU_DCACHE_SIZE 2048
#define ALT_CPU_EXCEPTION_ADDR 0x04020020
#define ALT_CPU_FLASH_ACCELERATOR_LINES 0
#define ALT_CPU_FLASH_ACCELERATOR_LINE_SIZE 0
#define ALT_CPU_FLUSHDA_SUPPORTED
#define ALT_CPU_FREQ 50000000
#define ALT_CPU_HARDWARE_DIVIDE_PRESENT 0
#define ALT_CPU_HARDWARE_MULTIPLY_PRESENT 1
#define ALT_CPU_HARDWARE_MULX_PRESENT 0
#define ALT_CPU_HAS_DEBUG_CORE 1
#define ALT_CPU_HAS_DEBUG_STUB
#define ALT_CPU_HAS_EXTRA_EXCEPTION_INFO
#define ALT_CPU_HAS_ILLEGAL_INSTRUCTION_EXCEPTION
#define ALT_CPU_HAS_JMPI_INSTRUCTION
#define ALT_CPU_ICACHE_LINE_SIZE 32
#define ALT_CPU_ICACHE_LINE_SIZE_LOG2 5
#define ALT_CPU_ICACHE_SIZE 4096
#define ALT_CPU_INITDA_SUPPORTED
#define ALT_CPU_INST_ADDR_WIDTH 0x1b
#define ALT_CPU_NAME "nios2_gen2_0"
#define ALT_CPU_NUM_OF_SHADOW_REG_SETS 0
#define ALT_CPU_OCI_VERSION 1
#define ALT_CPU_RESET_ADDR 0x04020000


/*
 * CPU configuration (with legacy prefix - don't use these anymore)
 *
 */

#define NIOS2_BIG_ENDIAN 0
#define NIOS2_BREAK_ADDR 0x04040820
#define NIOS2_CPU_ARCH_NIOS2_R1
#define NIOS2_CPU_FREQ 50000000u
#define NIOS2_CPU_ID_SIZE 1
#define NIOS2_CPU_ID_VALUE 0x00000000
#define NIOS2_CPU_IMPLEMENTATION "fast"
#define NIOS2_DATA_ADDR_WIDTH 0x1b
#define NIOS2_DCACHE_BYPASS_MASK 0x80000000
#define NIOS2_DCACHE_LINE_SIZE 32
#define NIOS2_DCACHE_LINE_SIZE_LOG2 5
#define NIOS2_DCACHE_SIZE 2048
#define NIOS2_EXCEPTION_ADDR 0x04020020
#define NIOS2_FLASH_ACCELERATOR_LINES 0
#define NIOS2_FLASH_ACCELERATOR_LINE_SIZE 0
#define NIOS2_FLUSHDA_SUPPORTED
#define NIOS2_HARDWARE_DIVIDE_PRESENT 0
#define NIOS2_HARDWARE_MULTIPLY_PRESENT 1
#define NIOS2_HARDWARE_MULX_PRESENT 0
#define NIOS2_HAS_DEBUG_CORE 1
#define NIOS2_HAS_DEBUG_STUB
#define NIOS2_HAS_EXTRA_EXCEPTION_INFO
#define NIOS2_HAS_ILLEGAL_INSTRUCTION_EXCEPTION
#define NIOS2_HAS_JMPI_INSTRUCTION
#define NIOS2_ICACHE_LINE_SIZE 32
#define NIOS2_ICACHE_LINE_SIZE_LOG2 5
#define NIOS2_ICACHE_SIZE 4096
#define NIOS2_INITDA_SUPPORTED
#define NIOS2_INST_ADDR_WIDTH 0x1b
#define NIOS2_NUM_OF_SHADOW_REG_SETS 0
#define NIOS2_OCI_VERSION 1
#define NIOS2_RESET_ADDR 0x04020000


/*
 * Define for each module class mastered by the CPU
 *
 */

#define __ALTERA_AVALON_JTAG_UART
#define __ALTERA_AVALON_NEW_SDRAM_CONTROLLER
#define __ALTERA_AVALON_ONCHIP_MEMORY2
#define __ALTERA_AVALON_PIO
#define __ALTERA_AVALON_SPI
#define __ALTERA_NIOS2_GEN2


/*
 * System configuration
 *
 */

#define ALT_DEVICE_FAMILY "MAX 10"
#define ALT_ENHANCED_INTERRUPT_API_PRESENT
#define ALT_IRQ_BASE NULL
#define ALT_LOG_PORT "/dev/null"
#define ALT_LOG_PORT_BASE 0x0
#define ALT_LOG_PORT_DEV null
#define ALT_LOG_PORT_TYPE ""
#define ALT_NUM_EXTERNAL_INTERRUPT_CONTROLLERS 0
#define ALT_NUM_INTERNAL_INTERRUPT_CONTROLLERS 1
#define ALT_NUM_INTERRUPT_CONTROLLERS 1
#define ALT_STDERR "/dev/jtag_uart_0"
#define ALT_STDERR_BASE 0x40410a0
#define ALT_STDERR_DEV jtag_uart_0
#define ALT_STDERR_IS_JTAG_UART
#define ALT_STDERR_PRESENT
#define ALT_STDERR_TYPE "altera_avalon_jtag_uart"
#define ALT_STDIN "/dev/jtag_uart_0"
#define ALT_STDIN_BASE 0x40410a0
#define ALT_STDIN_DEV jtag_uart_0
#define ALT_STDIN_IS_JTAG_UART
#define ALT_STDIN_PRESENT
#define ALT_STDIN_TYPE "altera_avalon_jtag_uart"
#define ALT_STDOUT "/dev/jtag_uart_0"
#define ALT_STDOUT_BASE 0x40410a0
#define ALT_STDOUT_DEV jtag_uart_0
#define ALT_STDOUT_IS_JTAG_UART
#define ALT_STDOUT_PRESENT
#define ALT_STDOUT_TYPE "altera_avalon_jtag_uart"
#define ALT_SYSTEM_NAME "NiosSystem"


/*
 * cam_redy configuration
 *
 */

#define ALT_MODULE_CLASS_cam_redy altera_avalon_pio
#define CAM_REDY_BASE 0x4041030
#define CAM_REDY_BIT_CLEARING_EDGE_REGISTER 0
#define CAM_REDY_BIT_MODIFYING_OUTPUT_REGISTER 0
#define CAM_REDY_CAPTURE 0
#define CAM_REDY_DATA_WIDTH 1
#define CAM_REDY_DO_TEST_BENCH_WIRING 0
#define CAM_REDY_DRIVEN_SIM_VALUE 0
#define CAM_REDY_EDGE_TYPE "NONE"
#define CAM_REDY_FREQ 50000000
#define CAM_REDY_HAS_IN 1
#define CAM_REDY_HAS_OUT 0
#define CAM_REDY_HAS_TRI 0
#define CAM_REDY_IRQ -1
#define CAM_REDY_IRQ_INTERRUPT_CONTROLLER_ID -1
#define CAM_REDY_IRQ_TYPE "NONE"
#define CAM_REDY_NAME "/dev/cam_redy"
#define CAM_REDY_RESET_VALUE 0
#define CAM_REDY_SPAN 16
#define CAM_REDY_TYPE "altera_avalon_pio"


/*
 * hal configuration
 *
 */

#define ALT_INCLUDE_INSTRUCTION_RELATED_EXCEPTION_API
#define ALT_MAX_FD 32
#define ALT_SYS_CLK none
#define ALT_TIMESTAMP_CLK none


/*
 * hex20 configuration
 *
 */

#define ALT_MODULE_CLASS_hex20 altera_avalon_pio
#define HEX20_BASE 0x4041060
#define HEX20_BIT_CLEARING_EDGE_REGISTER 0
#define HEX20_BIT_MODIFYING_OUTPUT_REGISTER 0
#define HEX20_CAPTURE 0
#define HEX20_DATA_WIDTH 24
#define HEX20_DO_TEST_BENCH_WIRING 0
#define HEX20_DRIVEN_SIM_VALUE 0
#define HEX20_EDGE_TYPE "NONE"
#define HEX20_FREQ 50000000
#define HEX20_HAS_IN 0
#define HEX20_HAS_OUT 1
#define HEX20_HAS_TRI 0
#define HEX20_IRQ -1
#define HEX20_IRQ_INTERRUPT_CONTROLLER_ID -1
#define HEX20_IRQ_TYPE "NONE"
#define HEX20_NAME "/dev/hex20"
#define HEX20_RESET_VALUE 0
#define HEX20_SPAN 16
#define HEX20_TYPE "altera_avalon_pio"


/*
 * hex53 configuration
 *
 */

#define ALT_MODULE_CLASS_hex53 altera_avalon_pio
#define HEX53_BASE 0x4041050
#define HEX53_BIT_CLEARING_EDGE_REGISTER 0
#define HEX53_BIT_MODIFYING_OUTPUT_REGISTER 0
#define HEX53_CAPTURE 0
#define HEX53_DATA_WIDTH 24
#define HEX53_DO_TEST_BENCH_WIRING 0
#define HEX53_DRIVEN_SIM_VALUE 0
#define HEX53_EDGE_TYPE "NONE"
#define HEX53_FREQ 50000000
#define HEX53_HAS_IN 0
#define HEX53_HAS_OUT 1
#define HEX53_HAS_TRI 0
#define HEX53_IRQ -1
#define HEX53_IRQ_INTERRUPT_CONTROLLER_ID -1
#define HEX53_IRQ_TYPE "NONE"
#define HEX53_NAME "/dev/hex53"
#define HEX53_RESET_VALUE 0
#define HEX53_SPAN 16
#define HEX53_TYPE "altera_avalon_pio"


/*
 * img_addy configuration
 *
 */

#define ALT_MODULE_CLASS_img_addy altera_avalon_pio
#define IMG_ADDY_BASE 0x4041020
#define IMG_ADDY_BIT_CLEARING_EDGE_REGISTER 0
#define IMG_ADDY_BIT_MODIFYING_OUTPUT_REGISTER 0
#define IMG_ADDY_CAPTURE 0
#define IMG_ADDY_DATA_WIDTH 17
#define IMG_ADDY_DO_TEST_BENCH_WIRING 0
#define IMG_ADDY_DRIVEN_SIM_VALUE 0
#define IMG_ADDY_EDGE_TYPE "NONE"
#define IMG_ADDY_FREQ 50000000
#define IMG_ADDY_HAS_IN 0
#define IMG_ADDY_HAS_OUT 1
#define IMG_ADDY_HAS_TRI 0
#define IMG_ADDY_IRQ -1
#define IMG_ADDY_IRQ_INTERRUPT_CONTROLLER_ID -1
#define IMG_ADDY_IRQ_TYPE "NONE"
#define IMG_ADDY_NAME "/dev/img_addy"
#define IMG_ADDY_RESET_VALUE 0
#define IMG_ADDY_SPAN 16
#define IMG_ADDY_TYPE "altera_avalon_pio"


/*
 * jtag_uart_0 configuration
 *
 */

#define ALT_MODULE_CLASS_jtag_uart_0 altera_avalon_jtag_uart
#define JTAG_UART_0_BASE 0x40410a0
#define JTAG_UART_0_IRQ 0
#define JTAG_UART_0_IRQ_INTERRUPT_CONTROLLER_ID 0
#define JTAG_UART_0_NAME "/dev/jtag_uart_0"
#define JTAG_UART_0_READ_DEPTH 64
#define JTAG_UART_0_READ_THRESHOLD 8
#define JTAG_UART_0_SPAN 8
#define JTAG_UART_0_TYPE "altera_avalon_jtag_uart"
#define JTAG_UART_0_WRITE_DEPTH 64
#define JTAG_UART_0_WRITE_THRESHOLD 8


/*
 * key configuration
 *
 */

#define ALT_MODULE_CLASS_key altera_avalon_pio
#define KEY_BASE 0x4041070
#define KEY_BIT_CLEARING_EDGE_REGISTER 0
#define KEY_BIT_MODIFYING_OUTPUT_REGISTER 0
#define KEY_CAPTURE 0
#define KEY_DATA_WIDTH 2
#define KEY_DO_TEST_BENCH_WIRING 0
#define KEY_DRIVEN_SIM_VALUE 0
#define KEY_EDGE_TYPE "NONE"
#define KEY_FREQ 50000000
#define KEY_HAS_IN 1
#define KEY_HAS_OUT 0
#define KEY_HAS_TRI 0
#define KEY_IRQ 2
#define KEY_IRQ_INTERRUPT_CONTROLLER_ID 0
#define KEY_IRQ_TYPE "LEVEL"
#define KEY_NAME "/dev/key"
#define KEY_RESET_VALUE 0
#define KEY_SPAN 16
#define KEY_TYPE "altera_avalon_pio"


/*
 * ledr configuration
 *
 */

#define ALT_MODULE_CLASS_ledr altera_avalon_pio
#define LEDR_BASE 0x4041090
#define LEDR_BIT_CLEARING_EDGE_REGISTER 0
#define LEDR_BIT_MODIFYING_OUTPUT_REGISTER 0
#define LEDR_CAPTURE 0
#define LEDR_DATA_WIDTH 10
#define LEDR_DO_TEST_BENCH_WIRING 0
#define LEDR_DRIVEN_SIM_VALUE 0
#define LEDR_EDGE_TYPE "NONE"
#define LEDR_FREQ 50000000
#define LEDR_HAS_IN 0
#define LEDR_HAS_OUT 1
#define LEDR_HAS_TRI 0
#define LEDR_IRQ -1
#define LEDR_IRQ_INTERRUPT_CONTROLLER_ID -1
#define LEDR_IRQ_TYPE "NONE"
#define LEDR_NAME "/dev/ledr"
#define LEDR_RESET_VALUE 0
#define LEDR_SPAN 16
#define LEDR_TYPE "altera_avalon_pio"


/*
 * pixel_dat configuration
 *
 */

#define ALT_MODULE_CLASS_pixel_dat altera_avalon_pio
#define PIXEL_DAT_BASE 0x4041040
#define PIXEL_DAT_BIT_CLEARING_EDGE_REGISTER 0
#define PIXEL_DAT_BIT_MODIFYING_OUTPUT_REGISTER 0
#define PIXEL_DAT_CAPTURE 0
#define PIXEL_DAT_DATA_WIDTH 4
#define PIXEL_DAT_DO_TEST_BENCH_WIRING 0
#define PIXEL_DAT_DRIVEN_SIM_VALUE 0
#define PIXEL_DAT_EDGE_TYPE "NONE"
#define PIXEL_DAT_FREQ 50000000
#define PIXEL_DAT_HAS_IN 0
#define PIXEL_DAT_HAS_OUT 1
#define PIXEL_DAT_HAS_TRI 0
#define PIXEL_DAT_IRQ -1
#define PIXEL_DAT_IRQ_INTERRUPT_CONTROLLER_ID -1
#define PIXEL_DAT_IRQ_TYPE "NONE"
#define PIXEL_DAT_NAME "/dev/pixel_dat"
#define PIXEL_DAT_RESET_VALUE 0
#define PIXEL_DAT_SPAN 16
#define PIXEL_DAT_TYPE "altera_avalon_pio"


/*
 * ram configuration
 *
 */

#define ALT_MODULE_CLASS_ram altera_avalon_onchip_memory2
#define RAM_ALLOW_IN_SYSTEM_MEMORY_CONTENT_EDITOR 0
#define RAM_ALLOW_MRAM_SIM_CONTENTS_ONLY_FILE 0
#define RAM_BASE 0x4020000
#define RAM_CONTENTS_INFO ""
#define RAM_DUAL_PORT 0
#define RAM_GUI_RAM_BLOCK_TYPE "AUTO"
#define RAM_INIT_CONTENTS_FILE "NiosSystem_ram"
#define RAM_INIT_MEM_CONTENT 1
#define RAM_INSTANCE_ID "NONE"
#define RAM_IRQ -1
#define RAM_IRQ_INTERRUPT_CONTROLLER_ID -1
#define RAM_NAME "/dev/ram"
#define RAM_NON_DEFAULT_INIT_FILE_ENABLED 0
#define RAM_RAM_BLOCK_TYPE "AUTO"
#define RAM_READ_DURING_WRITE_MODE "DONT_CARE"
#define RAM_SINGLE_CLOCK_OP 0
#define RAM_SIZE_MULTIPLE 1
#define RAM_SIZE_VALUE 102400
#define RAM_SPAN 102400
#define RAM_TYPE "altera_avalon_onchip_memory2"
#define RAM_WRITABLE 1


/*
 * sdram_control configuration
 *
 */

#define ALT_MODULE_CLASS_sdram_control altera_avalon_new_sdram_controller
#define SDRAM_CONTROL_BASE 0x0
#define SDRAM_CONTROL_CAS_LATENCY 3
#define SDRAM_CONTROL_CONTENTS_INFO
#define SDRAM_CONTROL_INIT_NOP_DELAY 0.0
#define SDRAM_CONTROL_INIT_REFRESH_COMMANDS 2
#define SDRAM_CONTROL_IRQ -1
#define SDRAM_CONTROL_IRQ_INTERRUPT_CONTROLLER_ID -1
#define SDRAM_CONTROL_IS_INITIALIZED 1
#define SDRAM_CONTROL_NAME "/dev/sdram_control"
#define SDRAM_CONTROL_POWERUP_DELAY 100.0
#define SDRAM_CONTROL_REFRESH_PERIOD 15.625
#define SDRAM_CONTROL_REGISTER_DATA_IN 1
#define SDRAM_CONTROL_SDRAM_ADDR_WIDTH 0x19
#define SDRAM_CONTROL_SDRAM_BANK_WIDTH 2
#define SDRAM_CONTROL_SDRAM_COL_WIDTH 10
#define SDRAM_CONTROL_SDRAM_DATA_WIDTH 16
#define SDRAM_CONTROL_SDRAM_NUM_BANKS 4
#define SDRAM_CONTROL_SDRAM_NUM_CHIPSELECTS 1
#define SDRAM_CONTROL_SDRAM_ROW_WIDTH 13
#define SDRAM_CONTROL_SHARED_DATA 0
#define SDRAM_CONTROL_SIM_MODEL_BASE 0
#define SDRAM_CONTROL_SPAN 67108864
#define SDRAM_CONTROL_STARVATION_INDICATOR 0
#define SDRAM_CONTROL_TRISTATE_BRIDGE_SLAVE ""
#define SDRAM_CONTROL_TYPE "altera_avalon_new_sdram_controller"
#define SDRAM_CONTROL_T_AC 5.5
#define SDRAM_CONTROL_T_MRD 3
#define SDRAM_CONTROL_T_RCD 20.0
#define SDRAM_CONTROL_T_RFC 70.0
#define SDRAM_CONTROL_T_RP 20.0
#define SDRAM_CONTROL_T_WR 14.0


/*
 * spi_0 configuration
 *
 */

#define ALT_MODULE_CLASS_spi_0 altera_avalon_spi
#define SPI_0_BASE 0x4041000
#define SPI_0_CLOCKMULT 1
#define SPI_0_CLOCKPHASE 1
#define SPI_0_CLOCKPOLARITY 1
#define SPI_0_CLOCKUNITS "Hz"
#define SPI_0_DATABITS 8
#define SPI_0_DATAWIDTH 16
#define SPI_0_DELAYMULT "1.0E-9"
#define SPI_0_DELAYUNITS "ns"
#define SPI_0_EXTRADELAY 0
#define SPI_0_INSERT_SYNC 0
#define SPI_0_IRQ 3
#define SPI_0_IRQ_INTERRUPT_CONTROLLER_ID 0
#define SPI_0_ISMASTER 1
#define SPI_0_LSBFIRST 0
#define SPI_0_NAME "/dev/spi_0"
#define SPI_0_NUMSLAVES 2
#define SPI_0_PREFIX "spi_"
#define SPI_0_SPAN 32
#define SPI_0_SYNC_REG_DEPTH 2
#define SPI_0_TARGETCLOCK 5000000u
#define SPI_0_TARGETSSDELAY "0.0"
#define SPI_0_TYPE "altera_avalon_spi"


/*
 * sw configuration
 *
 */

#define ALT_MODULE_CLASS_sw altera_avalon_pio
#define SW_BASE 0x4041080
#define SW_BIT_CLEARING_EDGE_REGISTER 0
#define SW_BIT_MODIFYING_OUTPUT_REGISTER 0
#define SW_CAPTURE 0
#define SW_DATA_WIDTH 10
#define SW_DO_TEST_BENCH_WIRING 0
#define SW_DRIVEN_SIM_VALUE 0
#define SW_EDGE_TYPE "NONE"
#define SW_FREQ 50000000
#define SW_HAS_IN 1
#define SW_HAS_OUT 0
#define SW_HAS_TRI 0
#define SW_IRQ 1
#define SW_IRQ_INTERRUPT_CONTROLLER_ID 0
#define SW_IRQ_TYPE "LEVEL"
#define SW_NAME "/dev/sw"
#define SW_RESET_VALUE 0
#define SW_SPAN 16
#define SW_TYPE "altera_avalon_pio"

#endif /* __SYSTEM_H_ */
