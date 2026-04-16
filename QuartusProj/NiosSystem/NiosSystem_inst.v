	NiosSystem u0 (
		.cam_redy_export     (<connected-to-cam_redy_export>),     //      cam_redy.export
		.clk_clk             (<connected-to-clk_clk>),             //           clk.clk
		.gsens_int_export    (<connected-to-gsens_int_export>),    //     gsens_int.export
		.hex20_export        (<connected-to-hex20_export>),        //         hex20.export
		.hex53_export        (<connected-to-hex53_export>),        //         hex53.export
		.img_addy_export     (<connected-to-img_addy_export>),     //      img_addy.export
		.key_export          (<connected-to-key_export>),          //           key.export
		.ledr_export         (<connected-to-ledr_export>),         //          ledr.export
		.pixel_dat_export    (<connected-to-pixel_dat_export>),    //     pixel_dat.export
		.reset_reset_n       (<connected-to-reset_reset_n>),       //         reset.reset_n
		.sdram_control_addr  (<connected-to-sdram_control_addr>),  // sdram_control.addr
		.sdram_control_ba    (<connected-to-sdram_control_ba>),    //              .ba
		.sdram_control_cas_n (<connected-to-sdram_control_cas_n>), //              .cas_n
		.sdram_control_cke   (<connected-to-sdram_control_cke>),   //              .cke
		.sdram_control_cs_n  (<connected-to-sdram_control_cs_n>),  //              .cs_n
		.sdram_control_dq    (<connected-to-sdram_control_dq>),    //              .dq
		.sdram_control_dqm   (<connected-to-sdram_control_dqm>),   //              .dqm
		.sdram_control_ras_n (<connected-to-sdram_control_ras_n>), //              .ras_n
		.sdram_control_we_n  (<connected-to-sdram_control_we_n>),  //              .we_n
		.spi_0_MISO          (<connected-to-spi_0_MISO>),          //         spi_0.MISO
		.spi_0_MOSI          (<connected-to-spi_0_MOSI>),          //              .MOSI
		.spi_0_SCLK          (<connected-to-spi_0_SCLK>),          //              .SCLK
		.spi_0_SS_n          (<connected-to-spi_0_SS_n>),          //              .SS_n
		.sw_export           (<connected-to-sw_export>),           //            sw.export
		.usec_counter_export (<connected-to-usec_counter_export>)  //  usec_counter.export
	);

