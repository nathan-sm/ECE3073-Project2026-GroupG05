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
	
	// === Auxillary wires ===
	
	wire vga_clk;
	
	// Pixel buffer write enable
	wire pxb_wr_en;
	assign pxb_wr_en = 1'b1;
	
	// Data and address from NiosII to write to pixel buffer
	// TODO: Assign these from NiosII
	wire [3:0] vga_in_data;
	wire [16:0] vga_in_addr;
	
	// Data and address to read from pixel buffer to VGA controller
	wire [3:0] vga_out_data;
	wire [16:0] vga_out_addr;
	
	// === Module instantiation ===
	
	vga_pll vga_pll_inst(.inclk0(CLOCK_50), .c0(vga_clk));
	
	pixel_buffer pixel_buffer_inst(
		.clock(vga_clk),
		.wren(pxb_wr_en),
		.wraddress(vga_in_addr),
		.data(vga_in_data),
		.rdaddress(vga_out_addr),
		.q(vga_out_data)
	);
	
	vga_controller vga_controller_inst(
		.VGA_CLK(vga_clk),
		.VGA_DATA(vga_out_data),
		.VGA_ADDR(vga_out_addr),
		.VGA_R(VGA_R),
		.VGA_G(VGA_G),
		.VGA_B(VGA_B),
		.VGA_HS(VGA_HS),
		.VGA_VS(VGA_VS)
	);
	
	// === Debug ===
	
//	assign vga_in_data = { {2{KEY[1]}}, {2{KEY[0]}} };
//	assign vga_in_addr = { {9{SW[8]}}, {SW[7:0]} };
//	assign pxb_wr_en = SW[9];

//	assign vga_in_data = 4'hf;
//	assign pxb_wr_en = 1'b1;

//	reg [3:0] vga_debug_data = 4'h0;
//	reg [16:0] vga_debug_addr = 17'h00000;
	
//	assign vga_in_data = vga_debug_data;
//	assign vga_in_addr = vga_debug_addr;
	
//	always @(posedge vga_clk) begin
//		vga_debug_data <= vga_debug_data + 4'd1;
//		vga_debug_addr <= vga_debug_addr + 16'd1;
//		
//		if (vga_debug_addr == 76799) begin
//			vga_debug_data <= 4'h0;
//			vga_debug_addr <= 17'h00000;
//		end
//	end

endmodule
