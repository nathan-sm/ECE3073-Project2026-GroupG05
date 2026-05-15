/*
 * system.h - SOPC Builder system and BSP software package information
 *
 * Machine generated for CPU 'image_proc' in SOPC Builder design 'NiosSystem'
 * SOPC Builder design path: ../../NiosSystem.sopcinfo
 *
 * Generated: Fri May 15 23:14:24 EST 2026
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
#define ALT_CPU_CPU_ID_VALUE 0x00000002
#define ALT_CPU_CPU_IMPLEMENTATION "fast"
#define ALT_CPU_DATA_ADDR_WIDTH 0x1b
#define ALT_CPU_DCACHE_BYPASS_MASK 0x80000000
#define ALT_CPU_DCACHE_LINE_SIZE 32
#define ALT_CPU_DCACHE_LINE_SIZE_LOG2 5
#define ALT_CPU_DCACHE_SIZE 2048
#define ALT_CPU_EXCEPTION_ADDR 0x00020020
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
#define ALT_CPU_NAME "image_proc"
#define ALT_CPU_NUM_OF_SHADOW_REG_SETS 0
#define ALT_CPU_OCI_VERSION 1
#define ALT_CPU_RESET_ADDR 0x00020000


/*
 * CPU configuration (with legacy prefix - don't use these anymore)
 *
 */

#define NIOS2_BIG_ENDIAN 0
#define NIOS2_BREAK_ADDR 0x04000820
#define NIOS2_CPU_ARCH_NIOS2_R1
#define NIOS2_CPU_FREQ 50000000u
#define NIOS2_CPU_ID_SIZE 1
#define NIOS2_CPU_ID_VALUE 0x00000002
#define NIOS2_CPU_IMPLEMENTATION "fast"
#define NIOS2_DATA_ADDR_WIDTH 0x1b
#define NIOS2_DCACHE_BYPASS_MASK 0x80000000
#define NIOS2_DCACHE_LINE_SIZE 32
#define NIOS2_DCACHE_LINE_SIZE_LOG2 5
#define NIOS2_DCACHE_SIZE 2048
#define NIOS2_EXCEPTION_ADDR 0x00020020
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
#define NIOS2_RESET_ADDR 0x00020000


/*
 * Define for each module class mastered by the CPU
 *
 */

#define __ALTERA_AVALON_JTAG_UART
#define __ALTERA_AVALON_MAILBOX_SIMPLE
#define __ALTERA_AVALON_NEW_SDRAM_CONTROLLER
#define __ALTERA_AVALON_PIO
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
#define ALT_STDERR "/dev/image_jtag"
#define ALT_STDERR_BASE 0x40010a0
#define ALT_STDERR_DEV image_jtag
#define ALT_STDERR_IS_JTAG_UART
#define ALT_STDERR_PRESENT
#define ALT_STDERR_TYPE "altera_avalon_jtag_uart"
#define ALT_STDIN "/dev/image_jtag"
#define ALT_STDIN_BASE 0x40010a0
#define ALT_STDIN_DEV image_jtag
#define ALT_STDIN_IS_JTAG_UART
#define ALT_STDIN_PRESENT
#define ALT_STDIN_TYPE "altera_avalon_jtag_uart"
#define ALT_STDOUT "/dev/image_jtag"
#define ALT_STDOUT_BASE 0x40010a0
#define ALT_STDOUT_DEV image_jtag
#define ALT_STDOUT_IS_JTAG_UART
#define ALT_STDOUT_PRESENT
#define ALT_STDOUT_TYPE "altera_avalon_jtag_uart"
#define ALT_SYSTEM_NAME "NiosSystem"


/*
 * ack_mailbox configuration
 *
 */

#define ACK_MAILBOX_BASE 0x4001080
#define ACK_MAILBOX_IRQ -1
#define ACK_MAILBOX_IRQ_INTERRUPT_CONTROLLER_ID -1
#define ACK_MAILBOX_NAME "/dev/ack_mailbox"
#define ACK_MAILBOX_SPAN 16
#define ACK_MAILBOX_TYPE "altera_avalon_mailbox_simple"
#define ALT_MODULE_CLASS_ack_mailbox altera_avalon_mailbox_simple


/*
 * altpll_0 configuration
 *
 */

#define ALTPLL_0_BASE 0x4001060
#define ALTPLL_0_IRQ -1
#define ALTPLL_0_IRQ_INTERRUPT_CONTROLLER_ID -1
#define ALTPLL_0_NAME "/dev/altpll_0"
#define ALTPLL_0_SPAN 16
#define ALTPLL_0_TYPE "altpll"
#define ALT_MODULE_CLASS_altpll_0 altpll


/*
 * data_mailbox configuration
 *
 */

#define ALT_MODULE_CLASS_data_mailbox altera_avalon_mailbox_simple
#define DATA_MAILBOX_BASE 0x4001090
#define DATA_MAILBOX_IRQ 0
#define DATA_MAILBOX_IRQ_INTERRUPT_CONTROLLER_ID 0
#define DATA_MAILBOX_NAME "/dev/data_mailbox"
#define DATA_MAILBOX_SPAN 16
#define DATA_MAILBOX_TYPE "altera_avalon_mailbox_simple"


/*
 * display_frame_mailbox configuration
 *
 */

#define ALT_MODULE_CLASS_display_frame_mailbox altera_avalon_mailbox_simple
#define DISPLAY_FRAME_MAILBOX_BASE 0x4001070
#define DISPLAY_FRAME_MAILBOX_IRQ -1
#define DISPLAY_FRAME_MAILBOX_IRQ_INTERRUPT_CONTROLLER_ID -1
#define DISPLAY_FRAME_MAILBOX_NAME "/dev/display_frame_mailbox"
#define DISPLAY_FRAME_MAILBOX_SPAN 16
#define DISPLAY_FRAME_MAILBOX_TYPE "altera_avalon_mailbox_simple"


/*
 * hal configuration
 *
 */

#define ALT_INCLUDE_INSTRUCTION_RELATED_EXCEPTION_API
#define ALT_MAX_FD 4
#define ALT_SYS_CLK none
#define ALT_TIMESTAMP_CLK none


/*
 * hex20 configuration
 *
 */

#define ALT_MODULE_CLASS_hex20 altera_avalon_pio
#define HEX20_BASE 0x4001000
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
#define HEX53_BASE 0x4001010
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
 * image_jtag configuration
 *
 */

#define ALT_MODULE_CLASS_image_jtag altera_avalon_jtag_uart
#define IMAGE_JTAG_BASE 0x40010a0
#define IMAGE_JTAG_IRQ 1
#define IMAGE_JTAG_IRQ_INTERRUPT_CONTROLLER_ID 0
#define IMAGE_JTAG_NAME "/dev/image_jtag"
#define IMAGE_JTAG_READ_DEPTH 64
#define IMAGE_JTAG_READ_THRESHOLD 8
#define IMAGE_JTAG_SPAN 8
#define IMAGE_JTAG_TYPE "altera_avalon_jtag_uart"
#define IMAGE_JTAG_WRITE_DEPTH 64
#define IMAGE_JTAG_WRITE_THRESHOLD 8


/*
 * key configuration
 *
 */

#define ALT_MODULE_CLASS_key altera_avalon_pio
#define KEY_BASE 0x4001040
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
#define LEDR_BASE 0x4001050
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
 * sw configuration
 *
 */

#define ALT_MODULE_CLASS_sw altera_avalon_pio
#define SW_BASE 0x4001030
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
#define SW_IRQ 3
#define SW_IRQ_INTERRUPT_CONTROLLER_ID 0
#define SW_IRQ_TYPE "LEVEL"
#define SW_NAME "/dev/sw"
#define SW_RESET_VALUE 0
#define SW_SPAN 16
#define SW_TYPE "altera_avalon_pio"


/*
 * usec_counter configuration
 *
 */

#define ALT_MODULE_CLASS_usec_counter altera_avalon_pio
#define USEC_COUNTER_BASE 0x4001020
#define USEC_COUNTER_BIT_CLEARING_EDGE_REGISTER 0
#define USEC_COUNTER_BIT_MODIFYING_OUTPUT_REGISTER 0
#define USEC_COUNTER_CAPTURE 0
#define USEC_COUNTER_DATA_WIDTH 32
#define USEC_COUNTER_DO_TEST_BENCH_WIRING 0
#define USEC_COUNTER_DRIVEN_SIM_VALUE 0
#define USEC_COUNTER_EDGE_TYPE "NONE"
#define USEC_COUNTER_FREQ 50000000
#define USEC_COUNTER_HAS_IN 1
#define USEC_COUNTER_HAS_OUT 0
#define USEC_COUNTER_HAS_TRI 0
#define USEC_COUNTER_IRQ -1
#define USEC_COUNTER_IRQ_INTERRUPT_CONTROLLER_ID -1
#define USEC_COUNTER_IRQ_TYPE "NONE"
#define USEC_COUNTER_NAME "/dev/usec_counter"
#define USEC_COUNTER_RESET_VALUE 0
#define USEC_COUNTER_SPAN 16
#define USEC_COUNTER_TYPE "altera_avalon_pio"

#endif /* __SYSTEM_H_ */
