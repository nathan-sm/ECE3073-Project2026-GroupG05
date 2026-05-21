/*
 * system.h - SOPC Builder system and BSP software package information
 *
 * Machine generated for CPU 'comms' in SOPC Builder design 'NiosSystem'
 * SOPC Builder design path: ../../NiosSystem.sopcinfo
 *
 * Generated: Fri May 15 14:06:16 EST 2026
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
#define ALT_CPU_BREAK_ADDR 0x04000820
#define ALT_CPU_CPU_ARCH_NIOS2_R1
#define ALT_CPU_CPU_FREQ 50000000u
#define ALT_CPU_CPU_ID_SIZE 1
#define ALT_CPU_CPU_ID_VALUE 0x00000001
#define ALT_CPU_CPU_IMPLEMENTATION "fast"
#define ALT_CPU_DATA_ADDR_WIDTH 0x1b
#define ALT_CPU_DCACHE_BYPASS_MASK 0x80000000
#define ALT_CPU_DCACHE_LINE_SIZE 32
#define ALT_CPU_DCACHE_LINE_SIZE_LOG2 5
#define ALT_CPU_DCACHE_SIZE 2048
#define ALT_CPU_EXCEPTION_ADDR 0x00800020
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
#define ALT_CPU_NAME "comms"
#define ALT_CPU_NUM_OF_SHADOW_REG_SETS 0
#define ALT_CPU_OCI_VERSION 1
#define ALT_CPU_RESET_ADDR 0x00800000


/*
 * CPU configuration (with legacy prefix - don't use these anymore)
 *
 */

#define NIOS2_BIG_ENDIAN 0
#define NIOS2_BREAK_ADDR 0x04000820
#define NIOS2_CPU_ARCH_NIOS2_R1
#define NIOS2_CPU_FREQ 50000000u
#define NIOS2_CPU_ID_SIZE 1
#define NIOS2_CPU_ID_VALUE 0x00000001
#define NIOS2_CPU_IMPLEMENTATION "fast"
#define NIOS2_DATA_ADDR_WIDTH 0x1b
#define NIOS2_DCACHE_BYPASS_MASK 0x80000000
#define NIOS2_DCACHE_LINE_SIZE 32
#define NIOS2_DCACHE_LINE_SIZE_LOG2 5
#define NIOS2_DCACHE_SIZE 2048
#define NIOS2_EXCEPTION_ADDR 0x00800020
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
#define NIOS2_RESET_ADDR 0x00800000


/*
 * Define for each module class mastered by the CPU
 *
 */

#define __ALTERA_AVALON_JTAG_UART
#define __ALTERA_AVALON_MAILBOX_SIMPLE
#define __ALTERA_AVALON_NEW_SDRAM_CONTROLLER
#define __ALTERA_AVALON_PIO
#define __ALTERA_AVALON_SPI
#define __ALTERA_NIOS2_GEN2
#define __ALTPLL


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
#define ALT_STDERR "/dev/jtag_uart_1"
#define ALT_STDERR_BASE 0x4001078
#define ALT_STDERR_DEV jtag_uart_1
#define ALT_STDERR_IS_JTAG_UART
#define ALT_STDERR_PRESENT
#define ALT_STDERR_TYPE "altera_avalon_jtag_uart"
#define ALT_STDIN "/dev/jtag_uart_1"
#define ALT_STDIN_BASE 0x4001078
#define ALT_STDIN_DEV jtag_uart_1
#define ALT_STDIN_IS_JTAG_UART
#define ALT_STDIN_PRESENT
#define ALT_STDIN_TYPE "altera_avalon_jtag_uart"
#define ALT_STDOUT "/dev/jtag_uart_1"
#define ALT_STDOUT_BASE 0x4001078
#define ALT_STDOUT_DEV jtag_uart_1
#define ALT_STDOUT_IS_JTAG_UART
#define ALT_STDOUT_PRESENT
#define ALT_STDOUT_TYPE "altera_avalon_jtag_uart"
#define ALT_SYSTEM_NAME "NiosSystem"


/*
 * ack_mailbox configuration
 *
 */

#define ACK_MAILBOX_BASE 0x4001020
#define ACK_MAILBOX_IRQ 3
#define ACK_MAILBOX_IRQ_INTERRUPT_CONTROLLER_ID 0
#define ACK_MAILBOX_NAME "/dev/ack_mailbox"
#define ACK_MAILBOX_SPAN 16
#define ACK_MAILBOX_TYPE "altera_avalon_mailbox_simple"
#define ALT_MODULE_CLASS_ack_mailbox altera_avalon_mailbox_simple


/*
 * altpll_0 configuration
 *
 */

#define ALTPLL_0_BASE 0x4001050
#define ALTPLL_0_IRQ -1
#define ALTPLL_0_IRQ_INTERRUPT_CONTROLLER_ID -1
#define ALTPLL_0_NAME "/dev/altpll_0"
#define ALTPLL_0_SPAN 16
#define ALTPLL_0_TYPE "altpll"
#define ALT_MODULE_CLASS_altpll_0 altpll


/*
 * cam_redy configuration
 *
 */

#define ALT_MODULE_CLASS_cam_redy altera_avalon_pio
#define CAM_REDY_BASE 0x4001030
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
 * data_mailbox configuration
 *
 */

#define ALT_MODULE_CLASS_data_mailbox altera_avalon_mailbox_simple
#define DATA_MAILBOX_BASE 0x4001060
#define DATA_MAILBOX_IRQ -1
#define DATA_MAILBOX_IRQ_INTERRUPT_CONTROLLER_ID -1
#define DATA_MAILBOX_NAME "/dev/data_mailbox"
#define DATA_MAILBOX_SPAN 16
#define DATA_MAILBOX_TYPE "altera_avalon_mailbox_simple"


/*
 * gsens_int configuration
 *
 */

#define ALT_MODULE_CLASS_gsens_int altera_avalon_pio
#define GSENS_INT_BASE 0x4001040
#define GSENS_INT_BIT_CLEARING_EDGE_REGISTER 0
#define GSENS_INT_BIT_MODIFYING_OUTPUT_REGISTER 0
#define GSENS_INT_CAPTURE 1
#define GSENS_INT_DATA_WIDTH 2
#define GSENS_INT_DO_TEST_BENCH_WIRING 0
#define GSENS_INT_DRIVEN_SIM_VALUE 0
#define GSENS_INT_EDGE_TYPE "RISING"
#define GSENS_INT_FREQ 50000000
#define GSENS_INT_HAS_IN 1
#define GSENS_INT_HAS_OUT 0
#define GSENS_INT_HAS_TRI 0
#define GSENS_INT_IRQ 1
#define GSENS_INT_IRQ_INTERRUPT_CONTROLLER_ID 0
#define GSENS_INT_IRQ_TYPE "EDGE"
#define GSENS_INT_NAME "/dev/gsens_int"
#define GSENS_INT_RESET_VALUE 0
#define GSENS_INT_SPAN 16
#define GSENS_INT_TYPE "altera_avalon_pio"


/*
 * hal configuration
 *
 */

#define ALT_INCLUDE_INSTRUCTION_RELATED_EXCEPTION_API
#define ALT_MAX_FD 32
#define ALT_SYS_CLK none
#define ALT_TIMESTAMP_CLK none


/*
 * jtag_uart_1 configuration
 *
 */

#define ALT_MODULE_CLASS_jtag_uart_1 altera_avalon_jtag_uart
#define JTAG_UART_1_BASE 0x4001078
#define JTAG_UART_1_IRQ 2
#define JTAG_UART_1_IRQ_INTERRUPT_CONTROLLER_ID 0
#define JTAG_UART_1_NAME "/dev/jtag_uart_1"
#define JTAG_UART_1_READ_DEPTH 64
#define JTAG_UART_1_READ_THRESHOLD 8
#define JTAG_UART_1_SPAN 8
#define JTAG_UART_1_TYPE "altera_avalon_jtag_uart"
#define JTAG_UART_1_WRITE_DEPTH 64
#define JTAG_UART_1_WRITE_THRESHOLD 8


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
#define SPI_0_BASE 0x4001000
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
#define SPI_0_IRQ 0
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

#endif /* __SYSTEM_H_ */
