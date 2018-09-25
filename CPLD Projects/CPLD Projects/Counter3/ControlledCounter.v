`timescale 1ns / 1ps

module ControlledCounter(
	input PCLK,
	output reg WR_n,
	input DMA0_Ready,
	input DMA1_Ready,
	input RESET,
	output SelectDMA,
	output [31:0] DQ,
	output [7:0] LED
	);

reg [31:0] Counter;

assign DQ = Counter;
// Give visual feeback on LEDs
assign LED = ~DQ[31:24];
// Swap DMA Channels (actually hardware threads) in sync with DMA Buffers filling
assign SelectDMA = Counter[12];

wire ChannelReady = SelectDMA ? !DMA0_Ready : !DMA1_Ready;

reg CurrentState, NextState;
parameter START = 1'b0;
parameter COUNT = 1'b1;

always @ (posedge PCLK or posedge RESET) begin
	if (RESET) begin
		WR_n <= 1;		// Disable writes
		Counter <= 0;	// 
		CurrentState <= START;
	end
	else begin
		CurrentState <= NextState;
// When the selected DMA channel is Ready then enable Writes
		if (WR_n && (SelectDMA ? DMA1_Ready : DMA0_Ready)) WR_n <= 0;
		if (CurrentState == COUNT) begin
// Increment the counter after START delay
			Counter <= Counter + 1;
// If I am just about to switch DMA channels and the target DMA channel is not ready then disable writes
			if ((Counter[11:0] == 4095) && ChannelReady) WR_n <= 1;
		end
	end
end

always @ (*) begin
// Add a one clock delay on start so that the SelectDMA signal is stable at 0 before counting begins
// GPIF sees the first WR_n and starts IN_DATA on the second WR_n
	case (CurrentState)
	START:	NextState = COUNT;
	COUNT:	NextState = COUNT;
	default:	NextState = START;	// Should not get here
	endcase
end

endmodule
