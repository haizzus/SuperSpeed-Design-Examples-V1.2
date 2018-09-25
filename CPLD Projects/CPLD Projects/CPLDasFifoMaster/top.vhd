`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////
module top(
	inout [12:0] CTRL,
	inout [31:0] DQ,
	input PCLK,
	inout I2C_SCL,
	inout I2C_SDA,
	input [7:0] Button,
	output [7:0] LED,
	inout [7:0] User,
	input SPI_SCK,
	input SPI_SSN,
	input RX_MOSI,
	output TX_MISO,
	output FlashCS_n,
	output INT,
	output GPIO45_n,
	input TP_2
    );

// Need to assign all the inputs else they get optimized away
	wire UnusedUser = User[0] & User[1] & User[2] & User[3] & User[4] & User[5] & User[6] & User[7];
	wire UnusedCtrl = CTRL[0] & CTRL[1] & CTRL[2] & CTRL[3] & CTRL[4] & CTRL[5] & CTRL[6] & CTRL[7] & CTRL[8] & CTRL[9] & CTRL[11] & CTRL[12];
	wire UnusedOther = RX_MOSI & SPI_SCK & SPI_SSN & TCI_2;
	assign TP_n = UnusedCtrl & UnusedOther & UnusedUser;
// Assign fixed outputs not used in this example
	assign FlashCS_n = 1'b1;
	assign INT = 1'b0;
	assign TX_MISO = 1'bZ;
// Using CTRL[10] to RESET the CPLD	
	assign RESET = CTRL[10];
// Both modules output to the LEDs, use Button[7] to select which module has control
	wire [7:0] I2C_LEDs;
	wire [7:0] Counter_LEDs;
	assign LED = Button[7] ? I2C_LEDs : Counter_LEDs;

// Include the Counter1 counter	
Counter1_Module Counter (
		.PCLK(PCLK),
		.RESET(RESET),
		.WR_n(CTRL[0]),
		.DQ(DQ),
		.LED(Counter_LEDs)
		);
		
// Include the I2C Port Expander module
   wire sda_out;
   wire out_en;
   wire ack_out;

   // tri-state output vs. drive 1 for logic 1 (i2c specs pullup)
   wire sda_in = I2C_SDA;
   assign I2C_SDA = (ack_out || (out_en & ~sda_out)) ? 1'b0 : 1'bz;
	
i2c_module i2c_slave (
	.scl(I2C_SCL), 
	.i2c_rst(RESET),
	.sda_in(sda_in),
	.gpio_input_pins(Button),
	.ack_out(ack_out),
	.out_en(out_en),
	.sda_out(sda_out),
	.gpio_output_pins(I2C_LEDs));
                         
defparam i2c_slave.my_i2c_addr = 7'b1010110;	// pass via parameter

//*********************************************************************
// Instantiate Pullups for Timing Simulation.  
// This allows for simulation of Open Drain (external pullup) behavior
// 
// NOTE:  In order for Timing Simulation to work, you must also append 
//	the top_timesim.v file with the following:  
//
//         X_PU X_PU1     	
//         (
//             	.O (I2C_SDA)
//    	);
//*********************************************************************

PULLUP PULLUP_inst (
	.O(I2C_SDA)
	);


endmodule

