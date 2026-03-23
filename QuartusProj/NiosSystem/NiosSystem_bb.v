
module NiosSystem (
	clk_clk,
	reset_reset_n,
	ledr_export,
	sw_export,
	key_export,
	hex20_export,
	hex53_export,
	pixel_dat_export,
	cam_redy_export,
	img_addy_export,
	sdram_control_addr,
	sdram_control_ba,
	sdram_control_cas_n,
	sdram_control_cke,
	sdram_control_cs_n,
	sdram_control_dq,
	sdram_control_dqm,
	sdram_control_ras_n,
	sdram_control_we_n);	

	input		clk_clk;
	input		reset_reset_n;
	output	[9:0]	ledr_export;
	input	[9:0]	sw_export;
	input	[1:0]	key_export;
	output	[23:0]	hex20_export;
	output	[23:0]	hex53_export;
	output	[3:0]	pixel_dat_export;
	input		cam_redy_export;
	output	[16:0]	img_addy_export;
	output	[12:0]	sdram_control_addr;
	output	[1:0]	sdram_control_ba;
	output		sdram_control_cas_n;
	output		sdram_control_cke;
	output		sdram_control_cs_n;
	inout	[15:0]	sdram_control_dq;
	output	[1:0]	sdram_control_dqm;
	output		sdram_control_ras_n;
	output		sdram_control_we_n;
endmodule
