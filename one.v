module one #(
    parameter DIVIDE = 10000
) (
    input clk,
    output[3:0] leds
);

    reg[31:0] count = 0;
    reg[1:0] state /* veriXYZlator public_flat_rw */ = 2'd0;

    assign leds[1:0] = state;

    two obj (
        .clk(clk),
        .leds(leds[3:2]),
        .count(count)
    );

    always @(posedge clk) begin
        if (count == DIVIDE - 1) begin
            count <= 0;
            state[1:0] <= state[1:0] + 1;
        end else
            count <= count + 1;
    end

endmodule
