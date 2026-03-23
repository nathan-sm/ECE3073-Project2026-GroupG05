	NiosSystem u0 (
		.clk_clk             (<connected-to-clk_clk>),             //           clk.clk
		.reset_reset_n       (<connected-to-reset_reset_n>),       //         reset.reset_n
		.ledr_export         (<connected-to-ledr_export>),         //          ledr.export
		.sw_export           (<connected-to-sw_export>),           //            sw.export
		.key_export          (<connected-to-key_export>),          //           key.export
		.hex20_export        (<connected-to-hex20_export>),        //         hex20.export
		.hex53_export        (<connected-to-hex53_export>),        //         hex53.export
		.pixel_dat_export    (<connected-to-pixel_dat_export>),    //     pixel_dat.export
		.cam_redy_export     (<connected-to-cam_redy_export>),     //      cam_redy.export
		.img_addy_export     (<connected-to-img_addy_export>),     //      img_addy.export
		.sdram_control_addr  (<connected-to-sdram_control_addr>),  // sdram_control.addr
		.sdram_control_ba    (<connected-to-sdram_control_ba>),    //              .ba
		.sdram_control_cas_n (<connected-to-sdram_control_cas_n>), //              .cas_n
		.sdram_control_cke   (<connected-to-sdram_control_cke>),   //              .cke
		.sdram_control_cs_n  (<connected-to-sdram_control_cs_n>),  //              .cs_n
		.sdram_control_dq    (<connected-to-sdram_control_dq>),    //              .dq
		.sdram_control_dqm   (<connected-to-sdram_control_dqm>),   //              .dqm
		.sdram_control_ras_n (<connected-to-sdram_control_ras_n>), //              .ras_n
		.sdram_control_we_n  (<connected-to-sdram_control_we_n>)   //              .we_n
	);

