//////////////////////////////////////////////////////////////////////////////
//
// i2c_module module
//
// Intended target for Xilinx CoolRunner-II CPLD 
//
// Author:  Xilinx, Inc.
//
// Notes:   This module decodes and controls I2C bus transactions for the
//          device.  The design functions as an I2C slave with 7 bit address.
//          Design supports single-byte reads/writes
//
//          I2C Write: 
//            <start> + <I2C address + wr><data byte> 
//          
//          I2C Read: 
//            <start> + <I2C address + rd><data byte>
//          
// Date:    04-15-2005
///////////////////////////////////////////////////////////////////////////////

`timescale 1ns/1ps

module i2c_module (

   scl,
   i2c_rst,
   sda_in,
   gpio_input_pins,
   ack_out,
   out_en,
   sda_out,
   gpio_output_pins);
   
   input        scl,             // I2C clock input
                i2c_rst,         // reset for all FFs in this module
                sda_in;          // I2C input data
   input [7:0]  gpio_input_pins; // gp io from pins to I2C bus
                                    
   output       ack_out,         // controls oe for acknowledge
                out_en,          // controls sda output enable
                sda_out;         // sda data going to pin during read access
   output [7:0] gpio_output_pins;// gp io from I2C to pins
                

   reg          ack_out;
   reg          out_en;
   reg [1:0]    next_state;									
   reg [1:0]    present_state;          
   reg          latch_data_in;          // pulse to move data_in to gpio outputs
   reg          shift_data_in;
   reg [3:0]    index;
   reg          start;
   reg          sda_out;
   reg [7:0]    data_in; 
   reg [7:0]    gpio_output;
   reg [7:0]    gpio_input;
   reg          latch_data_out;          // pulse to grab gpio data off bus for I2C read


   parameter         
             start_state  = 2'b00,    // wait for start of transaction
             addr_comp    = 2'b01,    // decode i2c address
             write_state  = 2'b10,    // store incoming data
             read_state   = 2'b11;    // write data out to I2C bus,

   parameter [7:1]
             my_i2c_addr = 7'bXXXXXXX; // CPLD i2c slave address, passed via parameter at top level.
				 									// Do not insert value here.  Instead do it at top level. 


   // ########################################
   // rising edge FFs for data and ack capture
   // ########################################
   
   always @ (posedge i2c_rst or posedge scl)
   begin
      if(i2c_rst)
         data_in <= 8'h00;
      else
      begin      
         // S2P register for input data and i2c address
         if (shift_data_in)
            data_in <= {data_in[6:0], sda_in};
         else
            data_in <= data_in;
      end // else
   end // always


   // ####################################################
   // falling edge FFs for state machine and control logic
   // ####################################################
   
   always @ (posedge i2c_rst or negedge scl)
   begin
      if(i2c_rst) 
      begin
         present_state <= start_state;
         out_en <= 0;
         ack_out <= 0;
         shift_data_in <= 0;
         latch_data_out <= 0;
         latch_data_in <= 0;
         index <= 4'd1;
      end // if
      else
      begin
         present_state <= next_state;

         // decode output enable for i2c reads
         if ((next_state == read_state) && (present_state != read_state) )
            out_en <= 1'b1;
         else if (((present_state == read_state) && (index == 4'd8)) ||
                   (present_state == start_state))
            out_en <= 1'b0;

         // decode when to signal acknowledge to the i2c transmitter
         ack_out <=  (((present_state == write_state) && (index == 4'd8)) ||
                        ((present_state == addr_comp) && (index == 4'd8) &&
                         (data_in[7:1] == my_i2c_addr)));

         // decode when to shift in data for write register (S2P).
         if (((present_state != addr_comp)  && (next_state == addr_comp)) ||
             ((present_state == addr_comp) && (next_state == write_state)))
            shift_data_in <= 1'b1;
         else if (((present_state == addr_comp) && (index == 4'd8)) ||
                  ((present_state == write_state) && (index == 4'd8)) ||
                   (present_state == start_state))
            shift_data_in <= 1'b0;
         else
            shift_data_in <= shift_data_in;

         // Use incrementer to index I2C bits
         // index count matches data bit order
         if ((next_state != present_state) || (start == 1))
            index <= 4'd1;
         else
            index <= index + 1;
        
         // create load pulse for output register (P2S)
         latch_data_out <=
            ((present_state == addr_comp) && (index == 4'd8) && (data_in[0] == 1));
      
         // create load pulse for register writes
         latch_data_in <= (present_state == write_state) && (index == 4'd7);
        
      end // else
   end // always



   // #################
   // I2C state machine
   // #################

   always @ (present_state or start or index or data_in) // or ack_in)
   begin

      case (present_state) /* synthesis parallel_case */
         start_state :
            if (start)
               next_state = addr_comp;
            else
               next_state = start_state;

         addr_comp:
            if (start)
               next_state = addr_comp;
            else if(index <= 4'd7)
               next_state = addr_comp;
            else if((data_in[7:1] == my_i2c_addr) && (index == 4'd8))
               next_state = addr_comp;
            else if((data_in[7:1] == my_i2c_addr) && (index == 4'd9) && !data_in[0])
               next_state = write_state;
            else if((data_in[7:1] == my_i2c_addr) && (index == 4'd9) && data_in[0])
               next_state = read_state;
            else
               next_state = start_state;

         write_state:  
            if (start)
               next_state = addr_comp;
            else if(index <= 4'd8)
               next_state = write_state;
            else 
               next_state = start_state;

         read_state: 
            if (start)
               next_state = addr_comp;
            else if(index <= 4'd8)                     
               next_state = read_state;
            else
               next_state = start_state;

         default:
            next_state = start_state;
      
      endcase // case(present_state)
   end // always


   // ###############################################################
   // I2C start decode
   // Try to detect start w/o faster clk.
   // ###############################################################
 
 
 	reg start_rst;

 	always @ (negedge sda_in or posedge start_rst)
   	if (start_rst)
     		start <= 1'b0;
   	else
   		start<=scl;

   
 	always @ (negedge scl) 
  		start_rst <= start;   
	


   // ###############################################################
   // I2C data xfer
   // move data_in or gpio_data_in to internal reg
   // ###############################################################
   always @ (negedge scl or posedge i2c_rst)
   	if(i2c_rst)
			gpio_output <=  8'hA5;	
      else if (latch_data_in)
        	gpio_output <= data_in [7:0];
       
 
   always @ (posedge scl or posedge i2c_rst)    
     	if(i2c_rst)
 //      	gpio_input <= 2'b0;                
       	gpio_input <= 8'h00;                
    	else if (latch_data_out)
       	gpio_input <= gpio_input_pins;
     
     
     
   


   // ###############################################################
   // Put data from internal reg to GPIO or to SDA
   // ###############################################################
   always @ (negedge scl or posedge i2c_rst)
   begin
   	if(i2c_rst)
      	sda_out <= 1'b1;                
      else if (out_en)
     	begin
			sda_out <= gpio_input[7-index];
//     		if (index == 6)
//             sda_out <= gpio_input[1];
//       	else if (index == 7)
//             sda_out <= gpio_input[0];
//       	else
//             sda_out <= 1'b1;
         end
   end

   wire [7:0] gpio_output_pins = gpio_output;
   	

endmodule // i2c_module
