// Microsecond counter: outputs a free-running 32-bit count of elapsed microseconds.
// Created By: Nathan Morris       (32532601)
//             Ritwam Shohaum      (33156816)
//             Shuk Kan LUI        (33891885)
//             Evgeny Solomin      (34977260)
// Created Date: 2026
// version = '1.0'
//
// Note: asynchronous reset is used intentionally so that the counter clears
// immediately on reset without waiting for the next clock edge.

module usec_counter (
    input clk,              // 50 MHz clock
    input reset_n,          // active-low reset
    output [31:0] usec_out  // current microsecond count
);

    reg [5:0]  count;        // counts 0 to 49 (50 cycles = 1 usec at 50 MHz)
    reg [31:0] usec_counter;

    always @(posedge clk or negedge reset_n) begin
        if (!reset_n) begin
            count <= 0;
            usec_counter <= 0;
        end else begin
            if (count == 6'd49) begin
                count <= 0;
                usec_counter <= usec_counter + 1;
            end else begin
                count <= count + 1;
            end
        end
    end

    assign usec_out = usec_counter;

endmodule
