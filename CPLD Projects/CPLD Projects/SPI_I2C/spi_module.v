
module spi_module (
        input SPICLK,         // Spi clock
        input SPIMOSI,        // Master out, slave in
        input SPICS_N,        // Spi chip select (active low)

        input [1:0] SPIADDR,  // Selects Spi target
        output SPIMISO,       // Master in, slave out (for button & user) 
        output MISO_ENA,      // Drive MISO
        output FLASHCSN,      // Flash arget of Spi (active low)

        input [7:0] BUTTON,   // Input switches
        inout [7:0] USER_PIN, // User defined IO
        output [7:0] LED,     // Spi drives leds

        input RESET
        );


//==============================================================================
// Set this parameter to determine the direction of the individual USER_PIN
parameter  user_output = 8'b11111111;   // 1 = output

//==============================================================================
// Declarations
wire SWLED_CS;				// Switch/Led register selected
wire USER_CS;				// User register selected
wire shift_mosi;			// Shift the mosi shift register
reg [8:0] mosi_r;			// Mosi shift register
wire load_miso;			// Load the Miso shift register
wire [7:0] read_data;   // Read of BUTTON or USER_PIN inputs
reg [7:0] miso_r;			// Miso shift register
wire write_led;			// Write the LED register
wire write_user;			// Write the USER register
wire [7:0] write_data;	// Data to write to registers
reg [7:0] led_r;			// LED register
reg [7:0] user_r;			// USER register

//==============================================================================
// Decode address to determine target of Spi operation
assign FLASHCSN = ~( (~SPICS_N) & (SPIADDR == 2'b00));   // Flash selected
assign SWLED_CS =  ( (~SPICS_N) & (SPIADDR == 2'b01));   // Switch/Led selected
assign USER_CS  =  ( (~SPICS_N) & (SPIADDR == 2'b10));   // User pins selected

//==============================================================================
// MOSI shift register
assign shift_mosi = (SWLED_CS | USER_CS) & ~mosi_r[8];

always @ (negedge SPICLK or posedge SPICS_N) begin
    if (SPICS_N) begin
        mosi_r <= 9'h001;
    end
    else begin
        mosi_r <= shift_mosi ? {mosi_r[7:0], SPIMOSI} : mosi_r;
    end
end

//==============================================================================
// MISO shift register
assign load_miso = (mosi_r[8:1] == 8'h01);
assign read_data  = SWLED_CS ? BUTTON : USER_CS ? USER_PIN : 8'h0;

always @ (negedge SPICLK or posedge SPICS_N) begin
    if (SPICS_N) begin
        miso_r <= 8'h0;
    end
    else begin
        miso_r <= load_miso ? read_data : {miso_r[6:0], 1'b0};
    end
end

assign SPIMISO = miso_r[7];
assign MISO_ENA = SWLED_CS | USER_CS;

//==============================================================================
// Drive LED and USER pins
assign write_led  = SWLED_CS & (mosi_r[8:7] == 2'b01);
assign write_user = USER_CS  & (mosi_r[8:7] == 2'b01);
assign write_data = {mosi_r[6:0], SPIMOSI};

always @ (negedge SPICLK or posedge RESET) begin
    if (RESET) begin
        led_r  <= 8'h55;
        user_r <= 8'h0;
    end
    else begin
        led_r  <= write_led  ? write_data : led_r;
        user_r <= write_user ? write_data : user_r;
    end
end

assign LED = led_r;
//assign USER_PIN[0] = user_output[0] ? user_r[0] : 1'bz;
//assign USER_PIN[1] = user_output[1] ? user_r[1] : 1'bz;
//assign USER_PIN[2] = user_output[2] ? user_r[2] : 1'bz;
//assign USER_PIN[3] = user_output[3] ? user_r[3] : 1'bz;
//assign USER_PIN[4] = user_output[4] ? user_r[4] : 1'bz;
//assign USER_PIN[5] = user_output[5] ? user_r[5] : 1'bz;
//assign USER_PIN[6] = user_output[6] ? user_r[6] : 1'bz;
//assign USER_PIN[7] = user_output[7] ? user_r[7] : 1'bz;

//==============================================================================
// For DEBUG
assign USER_PIN[0] = SPIMISO;
assign USER_PIN[1] = SWLED_CS;
assign USER_PIN[2] = USER_CS;
assign USER_PIN[3] = shift_mosi;
assign USER_PIN[4] = load_miso;
assign USER_PIN[5] = read_data;
assign USER_PIN[6] = write_led;
assign USER_PIN[7] = MISO_ENA;

endmodule