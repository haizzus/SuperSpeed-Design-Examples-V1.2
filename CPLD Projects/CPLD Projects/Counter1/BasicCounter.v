`timescale 1ns / 1ps

// This counter counts when RESET is released and drives WR_n low to indicate 'Valid Count'

module BasicCounter(
    input PCLK,
    input RESET,
    output reg WR_n,
    output [31:0] DQ,
    output [7:0] LED
    );

reg [31:0] Counter;

// Put key signals on LEDs for DEBUG, ~ since LED is ON when signal is low
assign LED = ~Counter[31:24];
assign DQ = Counter;

always @ (posedge PCLK or posedge RESET) begin
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
