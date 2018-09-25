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

// Need to assign inputs else they get optimized away
	assign TP_2 = RX_MOSI & SPI_SCK & SPI_SSN & I2C_SCL & I2C_SDA;

// Assign fixed outputs not used in this example
	assign FlashCS_n = 1'b1;
	assign INT = 1'b0;
	assign TX_MISO = 1'bZ;

// Include the Counter that does the work
// RESET is connected to CTRL[10] = GPIO27
// WR is connected to CTRL[0] = GPIO17	
BasicCounter Counter (
	.PCLK(PCLK),
	.RESET(CTRL[10]),
	.WR_n(CTRL[0]),
	.DQ(DQ),
	.LED(LED)
	);


endmodule
