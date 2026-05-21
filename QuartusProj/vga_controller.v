// *********************************
//
// VGA Controller Module
// Made for students to use during the ECE3073 Project
//
// Author: ECE3073 Teaching Team
// Last Edited: 05/02/2026
//
// Copyright © 2026 Copyright, Monash University
//
// *********************************


module vga_controller (
	// 25MHz clock derived from CLOCK_50
	input VGA_CLK,
	
	// Controller interface with the pixel buffer
	input [11:0] VGA_DATA,
	output [18:0] VGA_ADDR, // This address may vary, having it longer that what you use wont be an issue
	
	// Pixel data output
	output [3:0] VGA_R,
	output [3:0] VGA_G,
	output [3:0] VGA_B,
	
	
	// VGA timing output
	output VGA_HS,
	output VGA_VS
	
);
    parameter H_SYNC  = 96;
    parameter H_BACK  = 48;
    parameter H_FRONT = 16;
    parameter H_CYCLE = 800;

    parameter V_SYNC  = 2;
    parameter V_BACK  = 33;
    parameter V_FRONT = 10;
    parameter V_CYCLE = 525;


    reg [18:0] hAddr, vAddr;

    always @(posedge VGA_CLK) begin // horizontal counter
        if (hAddr < H_CYCLE - 1) begin
            hAddr <= hAddr + 1;
        end else begin
            hAddr <= 0;
        end
    end

    always @(posedge VGA_CLK) begin // vertical counter
        if (hAddr == H_CYCLE - 1) begin
            if (vAddr < V_CYCLE - 1) begin
                vAddr <= vAddr + 1;
            end else begin
                vAddr <= 0;
            end
        end
    end

    assign VGA_HS = ~(hAddr < H_SYNC); // hsync high for 96 counts
    assign VGA_VS = ~(vAddr < V_SYNC); // vsync high for 2 counts

	assign pixelValid = (H_ADDR > (H_SYNC + H_BACK) && H_ADDR < (H_CYCLE - H_FRONT)) && (V_ADDR > (V_SYNC + V_BACK) && V_ADDR < (V_CYCLE - V_FRONT));
	
	assign VGA_R = pixelValid ? VGA_DATA[11:8] : 4'd0;
   assign VGA_G = pixelValid ? VGA_DATA[7:4]  : 4'd0;
   assign VGA_B = pixelValid ? VGA_DATA[3:0]  : 4'd0;
	
	wire [18:0] H_MEM_ADDR, V_MEM_ADDR;	
	
	assign H_MEM_ADDR = H_ADDR - (H_SYNC + H_BACK) + 1;
	assign V_MEM_ADDR = V_ADDR - (V_SYNC + V_BACK);
	
//	// Scale the addresses to 320X240 instead of 640x480. 
	assign VGA_ADDR = ((H_MEM_ADDR >> 1) + ((V_MEM_ADDR >> 1) * 320));
					
	
endmodule
