`timescale 1ns / 1ps
// First declare all the connections for the CPLD board
module top(
	inout [12:0] CTRL,
	inout [31:0] DQ,
	input PCLK,
	inout I2C_SCL,
	inout I2C_SDA,
	input GPIO45_n,
	input [7:0] Button,
	output [7:0] LED,
	inout [7:0] User,
	input SPI_SCK,
	input SPI_SSN,
	input RX_MOSI,
	output TX_MISO,
	output FlashCS_n,
	output INT,
	output TP_2
    );

// Assign fixed outputs not used in this example
	assign FlashCS_n = 1'b1;
	assign INT = 1'b0;
	assign TX_MISO = 1'bZ;
	assign TP_2 = 1'b1;

// Include the Counter that does the work
// The following connections should match the GPIF II Designer Interface Definition
// WR is connected to CTRL[0] = GPIO17
// DMA0_Ready is connected to CTRL[4] = GPIO21
// DMA1_Ready is connected to CTRL[6] = GPIO23	
// RESET is connected to CTRL[10] = GPIO27
// SelectDMA is connected to CTRL[12] = GPIO29
ControlledCounter Counter (
	.PCLK(PCLK),
	.WR_n(CTRL[0]),
	.DMA0_Ready(CTRL[4]),
	.DMA1_Ready(CTRL[6]),
	.RESET(CTRL[10]),
	.SelectDMA(CTRL[12]),
	.DQ(DQ),
	.LED(LED)
	);


endmodule
