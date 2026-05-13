// outputs 32-bit microsecond counter
module usec_counter (
    input clk,              // 50 MHz clock
    input reset_n,          // active-low reset
    output [31:0] usec_out  // current microsecond count
);

    reg [5:0] count;         // counts 0 to 49 (50 cycles = 1 usec at 50MHz)
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
