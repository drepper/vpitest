module two #(
    parameter DIVIDE2 = 7500
) (
    input clk,
    output reg[1:0] leds,
    input[31:0] count
);

    reg[10:0] s = 0;

    always @(posedge clk) begin
        if (s == DIVIDE2[10:0]) begin
            leds <= count[20:19];
            s <= 0;
        end else
            s <= s + 1;
    end

endmodule
