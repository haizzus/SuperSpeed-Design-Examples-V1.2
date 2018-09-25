`timescale 1ns / 1ps

module FifoMasterCounter(
    input PCLK,
    output WR, 
	 output RD,
	 output LastWRData,
	 input LastRDData, 
	 input DMA0_Ready,
	 input DMA0_Watermark,
	 input DMA1_Ready, 
	 input DMA1_Watermark, 
	 input PushButton,
    input RESET,
    inout [31:0] DQ,
    output [7:0] LED,
	 input Select,
	 output [7:0] User
    );

// Define our counter which will provide data
reg [31:0] Counter;
// Display data movement during transfers
assign LED = ~DQ[31:24];

// Generate a RUN signal from the PushButton; PushButton presses toggle RUN
// Note that this creates a different clock domain so must be synchronized below
reg RUN;
always @ (negedge PushButton or posedge RESET) begin
	if (RESET) RUN<=0; else RUN <= ~RUN;
	end

// Define a State Machine for CPLD as FIFO Master, use one hot encoding
reg [7:0] CurrentState, NextState;
parameter IDLE 		= 8'b00000001;
parameter WAIT4DMA 	= 8'b00000010;
parameter DRIVE		= 8'b00000100;
parameter WRITE 		= 8'b00001000;
parameter PAUSE_W 	= 8'b00010000;
parameter READ 		= 8'b00100000;
parameter PAUSE_R 	= 8'b01000000;
parameter STOP 		= 8'b10000000;
// Display internal variables on User port for debug
assign User = { CurrentState[6:0], RUN };	
// Output signals are dependent upon the state machine
assign OE = (CurrentState == WRITE) | (CurrentState == DRIVE);
assign DQ = OE ? Counter : 32'hzzzzzzzz;
assign WR = (CurrentState == WRITE);
assign LastWRData = (CurrentState == STOP) | (CurrentState == IDLE);
assign RD = (CurrentState == READ);

reg RUN_Sync;
reg RUN_Sync0;
always @ (posedge PCLK or posedge RESET)
begin
	if (RESET) begin
		CurrentState <= IDLE;
		RUN_Sync <= 0;
		RUN_Sync0 <= 0;
		end
	else begin
		CurrentState <= NextState;
		if (NextState == IDLE) Counter <= 0;
		if (WR) Counter <= Counter + 1;
		RUN_Sync0 <= RUN;
		RUN_Sync <= RUN_Sync0;
		end
end

// Calculate next state using combinational logic
always @ (*) begin
	// Default is to stay in the current state
	NextState = CurrentState;
	case (CurrentState)
	IDLE:			if (RUN_Sync) NextState = WAIT4DMA;
	WAIT4DMA:	if (DMA0_Ready & !Select) NextState = DRIVE; else
					if (DMA1_Ready & Select) NextState = READ; else
					if (~RUN_Sync) NextState = STOP;
	DRIVE:		NextState = WRITE;
	WRITE:		if (~RUN_Sync) NextState = STOP; else
					if (DMA0_Watermark) NextState = PAUSE_W;
	PAUSE_W:		if (~DMA0_Ready) NextState = WAIT4DMA;
	READ:			if (~RUN_Sync) NextState = STOP; else
					if (DMA1_Watermark) NextState = PAUSE_R;
	PAUSE_R:		if (~DMA1_Ready) NextState = WAIT4DMA;
	STOP:			if (~RUN_Sync) NextState = IDLE;
	default:		NextState = IDLE;		// Should never get here
	endcase
end
		
endmodule
