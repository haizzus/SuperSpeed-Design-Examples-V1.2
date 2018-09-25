`timescale 1ns / 1ps

module FifoSlaveCounter(
    input PCLK,
    input WR, 
	 input RD,
	 output LastRDData,
	 input OE,
	 output WR_FIFO_Full,
	 output RD_FIFO_Empty,
	 input PushButton,
    input RESET,
    inout [31:0] DQ,
    output [7:0] LED,
	 output [7:0] User
    );

// Define our counter which will provide data
reg [31:0] Counter;
assign DQ = OE ? Counter : 32'hzzzzzzzz;
// Display data movement during transfers
assign LED = ~DQ[31:24];

// Generate a RUN signal from the PushButton; PushButton presses toggle RUN
// Note that this creates a different clock domain so must be synchronized below
reg RUN;
always @ (negedge PushButton or posedge RESET) begin
	if (RESET) RUN<=0; else RUN <= ~RUN;
	end

reg RUN_Sync;
reg RUN_Sync0;
// Define a State Machine for CPLD as FIFO Master, use one hot encoding
reg [4:0] CurrentState, NextState;
parameter IDLE 		= 5'b00001;
parameter WAIT4RDWR 	= 5'b00010;
parameter WRITE 		= 5'b00100;
parameter READ 		= 5'b01000;
parameter STOP 		= 5'b10000;
// Display internal variables on User port for debug
assign User = { CurrentState, RUN, RUN_Sync0, RUN_Sync };	
// Output signals are dependent upon the state machine
assign RD_FIFO_Empty = (CurrentState == IDLE);
assign WR_FIFO_Full = (CurrentState == IDLE);
assign LastRDData = (CurrentState == STOP) | (CurrentState == IDLE);

always @ (posedge PCLK or posedge RESET)
begin
	if (RESET) begin
		CurrentState <= IDLE;
		// Start at -1 so that the first counter value is 0
		Counter <= 32'hFFFFFFFF;
		RUN_Sync <= 0;
		RUN_Sync0 <= 0;
		end
	else begin
		CurrentState <= NextState;
		if (RD) Counter <= Counter + 1;
		// Synchronize RUN to PCLK domain before using in PCLK domain
		RUN_Sync0 <= RUN;
		RUN_Sync <= RUN_Sync0;
		end
end

// Calculate next state using combinational logic
always @ (*) begin
	// Default is to stay in the current state
	NextState = CurrentState;
	case (CurrentState)
	IDLE:			if (RUN_Sync) NextState = WAIT4RDWR;
	WAIT4RDWR:	if (RD) NextState = READ; else
					if (WR) NextState = WRITE; else
					if (!RUN_Sync) NextState = IDLE;
	WRITE:		if (!RUN_Sync) NextState = STOP; else
					if (!WR) NextState = WAIT4RDWR;
	READ:			if (!RD) NextState = WAIT4RDWR; else
					if (!RUN_Sync) NextState = STOP;
	STOP:			if (!RUN_Sync) NextState = IDLE;
	default:		NextState = IDLE;		// Should never get here
	endcase
end
		
endmodule
