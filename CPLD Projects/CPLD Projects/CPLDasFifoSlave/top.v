`timescale 1ns / 1ps

module top(
	inout [12:0] CTRL,
	inout [31:0] DQ,
	input PCLK,
	inout I2C_SCL,
	inout I2C_SDA,
	inout GPIO45_n,
	input [7:0] Button,
	output [7:0] LED,
	inout [7:0] User,
	input SPI_SCK,
	inout SPI_SSN,			// This is also LED_CTS
	input RX_MOSI,
	output TX_MISO,
	output FlashCS_n,
	output INT,
	output TP_2
	);

// Need to assign inputs else they get optimized away
	assign TP_2 = GPIO45_n & RX_MOSI & SPI_SCK & I2C_SCL & I2C_SDA & CTRL[4];

// Assign fixed outputs not used in this example
	assign FlashCS_n = 1'b1;
	assign INT = 1'b0;
	assign TX_MISO = 1'bZ;
	assign GPIO45_n = 1'bZ;
	assign SPI_SSN = ~User[2];		// This is CPLD RUN/STOP, LED on = collecting data
	assign CTRL[7] = Button[6];	// 0 = WRITE, 1 = READ

// Include the Counter	
FifoSlaveCounter Counter (
		.PCLK(PCLK),
		.WR(CTRL[0]),
		.RD(CTRL[1]),
		.LastRDData(CTRL[3]),
		.OE(CTRL[4]),
		.WR_FIFO_Full(CTRL[5]),
		.RD_FIFO_Empty(CTRL[6]),
		.PushButton(CTRL[9]),
		.RESET(CTRL[10]),
		.DQ(DQ),
		.LED(LED),
		.User(User)
		);
		
endmodule
