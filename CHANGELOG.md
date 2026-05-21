# Changelog

All notable changes to this project are documented in this file.

## [v2.0] - 2026-05-21

### Added

- **Three-core Nios II architecture** - Split single-core system into Communications (Core 0), Image Processing (Core 1), and Display (Core 2) for parallel operation. Led by Evgeny Solomin.
- **Display parallelism (Task 4A)** - Dedicated display core writes to the VGA pixel buffer independently, decoupling it from the processing pipeline. Double-buffered processing output prevents contention. Led by Evgeny Solomin.
- **RGB colour support** - Upgraded from greyscale to 16-bit RGB pixel format throughout the pipeline, including updated display functions and pixel buffer. Led by Ritwam Shohaum.
- **Quad-image display mode** - All four processing modes (raw, flip, blur, edge) displayed simultaneously in quadrants at quarter resolution. Led by Evgeny Solomin.
- **Accelerometer quad-image control** - Tilt the board to rearrange quad-image quadrant positions using gyroscope data with configurable sensitivity and thresholds. Led by Evgeny Solomin.
- **Accelerometer double-tap detection** - Double-tap the board to toggle between single and quad display modes. Led by Evgeny Solomin.
- **Image flip processing** - 180-degree image rotation selectable via SW[2:1]. Led by Nathan Morris.
- **Image convolution framework** - Generic 3x3 convolution function supporting arbitrary kernels with automatic normalisation. Led by Nathan Morris.
- **Box blur filter** - 3x3 uniform average blur using the convolution framework. Led by Nathan Morris.
- **Sobel edge detection** - Gradient-based edge detection with thresholding, using horizontal and vertical Sobel kernels. Led by Nathan Morris and Ritwam Shohaum.
- **Convolution optimisation (Task 4D)** - Optimised edge detection for improved performance on Nios II. Led by Shuk Kan LUI.
- **Image processing implementation (Task 2.2)** - Core image processing pipeline setup. Led by Shuk Kan LUI.
- **Microsecond counter** - Custom Verilog hardware timer (usec_counter.v) providing 32-bit microsecond-precision timestamps for benchmarking. Led by Nathan Morris.
- **FPS display on HEX** - Real-time frames-per-second shown on 7-segment displays with one decimal place. Led by Nathan Morris.
- **Per-step benchmarking** - Timing breakdown for each processing stage (flip, blur, edge, display) printed over JTAG UART every 20 frames. Led by Nathan Morris.
- **Shared project commons** - Common header files (common_defs.h, memory_addresses.h) shared across all cores to maintain consistent type definitions and memory layout. Led by Evgeny Solomin.
- **Software-side quad downscaling** - Quad mode now downscales images in software rather than relying on hardware. Led by Ritwam Shohaum.

### Changed

- **Dual-core to three-core system** - Migrated from the M1 dual-core architecture to three dedicated cores with mailbox-based inter-core communication. Led by Evgeny Solomin and Ritwam Shohaum.
- **Pixel buffer updated for RGB** - Modified pixel_buffer.v and VGA controller to handle 16-bit colour data instead of 4-bit greyscale. Led by Ritwam Shohaum.
- **Display functions take buffer parameter** - Refactored display_full_image and display_quad_image to accept a buffer pointer, allowing processed images to be passed without code duplication. Led by Nathan Morris.
- **Memory map reorganised** - Shared memory layout updated with triple buffers, processed image double buffers, shared accelerometer data, display state, and timing data all mapped into SDRAM. Led by Evgeny Solomin.
- **Processing mode selection** - Switch-based selection (SW[2:1]) for choosing between raw, flip, blur, and edge detection in single-image mode. Led by Nathan Morris.

### Fixed

- **Corrupted images on startup** - Fixed initial frame corruption when the processing pipeline starts before the first complete frame arrives. Led by Evgeny Solomin.
- **Image shifted one pixel** - Corrected off-by-one error in pixel buffer addressing. Led by Evgeny Solomin.
- **FPS timing accuracy** - Fixed FPS calculation to include frame read time, not just display time. Led by Evgeny Solomin.
- **GPIO logic and FPS display** - Fixed GPIO-related issues and FPS display rendering. Led by Ritwam Shohaum.
- **usec_out wire duplication** - Resolved duplicate wire assignment caused by incorrect merge conflict resolution. Led by Nathan Morris.
- **PIO connection** - Minor Platform Designer connection fix for peripheral mapping. Led by Ritwam Shohaum.

### Contributors

- **Evgeny Solomin** (sowngryphon) - Display parallelism, multi-core architecture, accelerometer control, quad-image mode, shared commons
- **Nathan Morris** - Image processing functions, benchmarking infrastructure, microsecond counter, convolution framework
- **Ritwam Shohaum** (ritwam-s) - RGB support, dual-core implementation, GPIO/display fixes, Sobel improvements, software downscaling
- **Shuk Kan LUI** (slui0008) - Image processing implementation (Task 2.2), convolution optimisation (Task 4D), documentation updates
