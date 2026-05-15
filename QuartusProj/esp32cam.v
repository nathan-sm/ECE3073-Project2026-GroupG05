// ESP32-CAM standalone test top-level module.
// Connects the VGA PLL, pixel buffer, and VGA controller for display testing.
// Note: vga_in_data and vga_in_addr are currently undriven (see TODO below).
// Created By: Nathan Morris       (32532601)
//             Ritwam Shohaum      (33156816)
//             Shuk Kan LUI        (33891885)
//             Evgeny Solomin      (34977260)
// Created Date: 2026
// version = '1.0'

module esp32cam(
    CLOCK_50,

    VGA_R,
    VGA_G,
    VGA_B,
    VGA_VS,
    VGA_HS,

    KEY,
    SW
);
    // === IO ===

    input wire CLOCK_50;

    // VGA
    output wire [3:0] VGA_R;
    output wire [3:0] VGA_G;
    output wire [3:0] VGA_B;
    output wire VGA_VS, VGA_HS;

    input [1:0] KEY;
    input [9:0] SW;

    // === Auxiliary wires ===

    wire vga_clk;

    // Pixel buffer write enable
    wire pxb_wr_en;
    assign pxb_wr_en = 1'b1;

    // Data and address from NiosII to write to pixel buffer
    // TODO: Assign these from NiosII
    wire [3:0]  vga_in_data;
    wire [16:0] vga_in_addr;

    // Data and address to read from pixel buffer to VGA controller
    wire [3:0]  vga_out_data;
    wire [16:0] vga_out_addr;

    // === Module instantiation ===

    vga_pll vga_pll_inst(.inclk0(CLOCK_50), .c0(vga_clk));

    pixel_buffer pixel_buffer_inst(
        .clock     (vga_clk),
        .wren      (pxb_wr_en),
        .wraddress (vga_in_addr),
        .data      (vga_in_data),
        .rdaddress (vga_out_addr),
        .q         (vga_out_data)
    );

    vga_controller vga_controller_inst(
        .VGA_CLK  (vga_clk),
        .VGA_DATA (vga_out_data),
        .VGA_ADDR (vga_out_addr),
        .VGA_R    (VGA_R),
        .VGA_G    (VGA_G),
        .VGA_B    (VGA_B),
        .VGA_HS   (VGA_HS),
        .VGA_VS   (VGA_VS)
    );

endmodule
