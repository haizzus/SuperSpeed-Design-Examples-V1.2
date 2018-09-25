`timescale 1ns / 1ps

module Counter1(
    input PCLK,
    input RESET,
    output reg WR_n,
    output [31:0] DQ,
    output [7:0] LED
    );

reg [31:0] Counter;

assign LED = ~Counter[31:24];
assign DQ = Counter;

always @ (negedge PCLK or posedge RESET) begin
	if (RESET) begin
		WR_n <= 1;		// Disable writes
		Counter <= 0;
	end
	else begin
		WR_n <= 0;
		Counter <= Counter + 1;
		
	end
end

endmodule
