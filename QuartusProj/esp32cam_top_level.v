// ESP32-CAM top-level module for the ECE3073 dual-core Nios II image system.
// Instantiates the Qsys NiosSystem, VGA subsystem, pixel buffer, and usec counter.
// Created By: Nathan Morris       (32532601)
//             Ritwam Shohaum      (33156816)
//             Shuk Kan LUI        (33891885)
//             Evgeny Solomin      (34977260)
// Created Date: 2026
// version = '1.0'

module esp32cam_top_level (
    // ---- System Clocks ---- //
    input           CLOCK_50,

    // ---- Basic I/O ---- //
    input  [1:0]    KEY,
    input  [9:0]    SW,
    output [9:0]    LEDR,

    // ---- HEX Displays ---- //
    output [7:0]    HEX0,
    output [7:0]    HEX1,
    output [7:0]    HEX2,
    output [7:0]    HEX3,
    output [7:0]    HEX4,
    output [7:0]    HEX5,

    // ---- VGA Outputs ---- //
    output [3:0]    VGA_R,
    output [3:0]    VGA_G,
    output [3:0]    VGA_B,
    output          VGA_HS,
    output          VGA_VS,

    // ---- SDRAM Connections ---- //
    output [12:0]   DRAM_ADDR,
    output [1:0]    DRAM_BA,
    output          DRAM_CAS_N,
    output          DRAM_CKE,
    output          DRAM_CLK,
    output          DRAM_CS_N,
    inout  [15:0]   DRAM_DQ,
    output          DRAM_LDQM,
    output          DRAM_UDQM,
    output          DRAM_RAS_N,
    output          DRAM_WE_N,

    // ---- ESP-CAM / SPI (GPIO) ---- //
    inout  [35:0]   GPIO,

    // ---- Accelerometer ---- //
    input  [2:1]    GSENSOR_INT,

    // Accelerometer SPI //
    input           GSENSOR_SDO,
    output          GSENSOR_SDI,
    output          GSENSOR_SCLK,
    output          GSENSOR_CS_N
);

    // ||Internal Wires & Interconnects||

    // Clocks
    wire vga_clk_25MHz;

    // Nios Exports
    wire [23:0] hex20_wire;
    wire [23:0] hex53_wire;
	 
    wire [11:0]  pixel_data_write;
	 
    wire [16:0] pixel_addr_write;
    wire [9:0]  ledr_wire;        // LEDR routed through wire for hardware test

    // VGA Controller to RAM interconnects
    wire [18:0] vga_read_address;
    wire [11:0]  vga_read_data;
	 
	// SPI Bus
    wire        spi_pico;          // Controller Out, Peripheral In — shared to both devices
    wire        spi_poci;          // Controller In, Peripheral Out — muxed from active device
    wire        spi_sclk;          // Clock from Nios
    wire [1:0]  spi_ss_n;          // One-hot CS: [0]=ESP-CAM, [1]=GSENSOR
    wire [31:0] usec_out;

    // ||PLL||

    // VGA Clock: 25 MHz, 0 ns phase shift
    vga_pll pll1 (
        .inclk0 (CLOCK_50),
        .c0     (vga_clk_25MHz)
    );

    // Qsys (Nios II) Dual-Core System Instantiation
    NiosSystem u0 (
        // System Clocks and Resets
        .clk_clk                (CLOCK_50),
        .reset_reset_n          (KEY[0]),
        .sdram_clk_clk          (DRAM_CLK),             // Shifted clock routed directly to SDRAM pin

        // Basic IO
        .ledr_export            (ledr_wire),
        .sw_export              (SW),
        .key_export             (KEY),

        // HEX Displays (24-bit mapped to 3x8-bit physical pins)
        .hex20_export           (hex20_wire),
        .hex53_export           (hex53_wire),

        // Pixel Buffer Control
        .pixel_dat_export       (pixel_data_write),
        .img_addy_export        (pixel_addr_write),

        // ESP-CAM Ready Pin (GPIO[2])
        .cam_redy_export        (GPIO[2]),

        // SDRAM Control
        .sdram_control_addr     (DRAM_ADDR),
        .sdram_control_ba       (DRAM_BA),
        .sdram_control_cas_n    (DRAM_CAS_N),
        .sdram_control_cke      (DRAM_CKE),
        .sdram_control_cs_n     (DRAM_CS_N),
        .sdram_control_dq       (DRAM_DQ),
        .sdram_control_dqm      ({DRAM_UDQM, DRAM_LDQM}),
        .sdram_control_ras_n    (DRAM_RAS_N),
        .sdram_control_we_n     (DRAM_WE_N),

        // SPI Interfaces
        .spi_0_MISO             (spi_poci),
        .spi_0_MOSI             (spi_pico),
        .spi_0_SCLK             (spi_sclk),
        .spi_0_SS_n             (spi_ss_n),

        // Accelerometer Interrupt
        .gsens_int_export       (GSENSOR_INT),

        // Microsecond Counter Export
        .usec_counter_export    (usec_out)
    );

    // ||Hardware Display Subsystem||

    // 2-Port RAM (Pixel Buffer)
    // Write side driven by Nios, read side driven by VGA controller
    pixel_buffer buffer_inst (
        .clock      (vga_clk_25MHz),       // Both read and write triggered by VGA clock
        .data       (pixel_data_write),    // From Nios
        .rdaddress  (vga_read_address),    // From VGA Controller
        .wraddress  (pixel_addr_write),    // From Nios
        .wren       (1'b1),                // Tie high — Nios controls via address writes
        .q          (vga_read_data)        // To VGA Controller
    );

    // VGA Controller
    vga_controller vga_inst (
        .VGA_CLK  (vga_clk_25MHz),     // 25 MHz VGA clock
        .VGA_DATA (vga_read_data),     // Data read from the 2-Port RAM
        .VGA_ADDR (vga_read_address),  // Address requested by VGA controller
        .VGA_R    (VGA_R),
        .VGA_G    (VGA_G),
        .VGA_B    (VGA_B),
        .VGA_HS   (VGA_HS),
        .VGA_VS   (VGA_VS)
    );

    // Microsecond counter for benchmarking
    usec_counter u_usec (
        .clk      (CLOCK_50),
        .reset_n  (KEY[0]),
        .usec_out (usec_out)
    );

    // ||Physical Pin Assignments & Logic||

    // Map the 24-bit Nios PIO outputs to the physical 8-bit HEX displays
    assign HEX0 = hex20_wire[7:0];
    assign HEX1 = hex20_wire[15:8];
    assign HEX2 = hex20_wire[23:16];

    assign HEX3 = hex53_wire[7:0];
    assign HEX4 = hex53_wire[15:8];
    assign HEX5 = hex53_wire[23:16];

    // Hardware FPGA flash test: pressing KEY[1] lights LEDR[0] directly in hardware
    assign LEDR = ledr_wire | {9'b0, ~KEY[1]};

    // High-impedance unused GPIO pins to prevent camera interference
    assign GPIO[11:10] = 2'bzz;
    assign GPIO[6]     = 1'bz;
    assign GPIO[4:3]   = 2'bzz;
    assign GPIO[1:0]   = 2'bzz;

    // --- ESP-CAM SPI (GPIO pins) ---
    assign GPIO[8]   = spi_pico;    // PICO <- Controller Out to ESP-CAM
    assign GPIO[9]   = spi_sclk;    // SCLK <- clock from Nios
    assign GPIO[5]   = spi_ss_n[0]; // CS_N <- active-low select for ESP-CAM

    // --- Accelerometer SPI (GSENSOR dedicated pins) ---
    assign GSENSOR_SCLK = spi_sclk;
    assign GSENSOR_SDI  = spi_pico;
    assign GSENSOR_CS_N = spi_ss_n[1];

    // --- POCI mux: route the active slave's data line to Nios ---
    assign spi_poci = (~spi_ss_n[0]) ? GPIO[7]      // ESP-CAM POCI when its CS is low
                    : (~spi_ss_n[1]) ? GSENSOR_SDO  // Accel POCI when its CS is low
                    : 1'b0;

endmodule
