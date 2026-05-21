# ECE3073 Project 2026 - Group G05

Real-time image processing system on the DE10-Lite FPGA using a multi-core Nios II architecture with live ESP-CAM video capture and VGA display.

## Team

| Name | Student ID | Email |
|------|-----------|-------|
| Nathan Morris | 32532601 | nmor0021@student.monash.edu |
| Ritwam Shohaum | 33156816 | rsho0005@student.monash.edu |
| Evgeny Solomin | 34977260 | esol0009@student.monash.edu |
| Shuk Kan LUI | 33891885 | slui0008@student.monash.edu |

## Overview

This project implements a video processing pipeline on the DE10-Lite FPGA (MAX 10). An ESP32-CAM captures 320x240 frames over SPI, which are processed through selectable image filters and displayed on a VGA monitor. An onboard accelerometer provides tilt-based control and double-tap interaction.

## Milestone 1 Baseline

M1 delivered a single-core Nios II system with full functionality across all tasks:

- **Task 1** - VGA display hardware: custom pixel buffer (dual-port RAM) interfaced with the provided VGA controller, driven by a 25 MHz PLL clock
- **Task 2** - Nios II processor setup in Platform Designer with SDRAM, PIO ports (LEDs, switches, keys, HEX displays, pixel data, image address, camera-ready pin), and JTAG UART. Test images generated and displayed from the processor.
- **Task 3** - SPI integration: ESP32-CAM streaming live greyscale video frames into SDRAM via the Qsys SPI controller, and onboard accelerometer reading rotational position and triggering interrupts on double-tap. Both peripherals share the same SPI bus.
- **Task 4** - Processor benchmarking: custom Verilog microsecond counter providing 32-bit timestamps, with FPS calculated and displayed on HEX[3:0] with two decimal places of precision.

## Changes Since Milestone 1

### Architecture: Single-Core to Three-Core

The M1 system used a single Nios II core handling communications, processing, and display. In M2 the system was restructured into three dedicated cores:

- **Core 0 (Communications)** - Receives camera frames via SPI, reads accelerometer data, and forwards frame buffer tokens to the processing core via mailbox.
- **Core 1 (Image Processing)** - Applies the selected processing mode (raw, flip, blur, edge detection) and passes processed frames to the display core via a second mailbox.
- **Core 2 (Display)** - Writes pixel data to the VGA pixel buffer and drives the 7-segment FPS counter.

Separating processing from display allows the two stages to run in parallel, improving throughput. Inter-core communication uses Altera Avalon mailbox peripherals, with a triple-buffer scheme for camera frames and a double-buffered processing output to prevent tearing and contention.

### RGB Colour Support

The pixel pipeline was upgraded from 4-bit greyscale to 16-bit RGB. This required changes to the VGA controller, pixel buffer Verilog, and all display/processing functions in software.

### Image Processing Modes

Four processing modes are selectable via SW[2:1] in single-image mode:

- **Raw** - unprocessed passthrough
- **Flip** - 180-degree image rotation
- **Box blur** - 3x3 uniform average convolution
- **Sobel edge detection** - gradient-based edge detection with thresholding

A generic 3x3 convolution function was implemented to support both blur and edge detection, with optimised variants (kernel unrolling, multiply-shift division replacement, zero-multiplication Sobel) for improved performance on the Nios II.

### Quad-Image Display

SW[0] toggles a quad-image mode that displays all four processing modes simultaneously at quarter resolution. The accelerometer controls which processing result appears in which quadrant (tilt to rearrange), and double-tap toggles between single and quad mode. Quad downscaling is handled in software.

### Benchmarking Enhancements

The M1 microsecond counter was extended with per-step timing breakdowns (flip, blur, edge, display) printed over JTAG UART every 20 frames, enabling before/after comparison for optimisation work.

## Repo Structure

```
QuartusProj/
├── esp32cam_top_level.v       # Top-level Verilog module
├── vga_controller.v           # VGA timing and sync generation
├── pixel_buffer.v             # Dual-port pixel buffer
├── usec_counter.v             # Microsecond precision timer
├── NiosSystem.qsys            # Platform Designer system definition
└── software/
    ├── communications/        # Core 0: SPI + accelerometer
    ├── communications_bsp/
    ├── image_processing/      # Core 1: image processing pipeline
    ├── image_processing_bsp/
    ├── display_proc/          # Core 2: VGA display driver
    ├── display_proc_bsp/
    └── project_commons/       # Shared headers (common_defs.h, memory_addresses.h)
```
