module i2c(
	I2C_SCLK, //audio
	I2C_SDAT, //audio
	clk,      //clk
	reset     //reset trigger?
);
	output	I2C_SCLK, I2C_SDAT;
	input	clk, reset;
	
	wire [28:0]	data [0:9];
	reg [2:0]   counter_clk;
	reg 		send, send_next;
	reg         start, start_next, finish, finish_next, reset_prev;
	reg         I2C_SDAT;
	reg [3:0]	counter_reg, counter_reg_next;
	reg	[4:0]	counter_data, counter_data_next;
	
	assign I2C_SCLK = (send)? counter_clk[2] : 1'b1;
	assign data[0] = 29'b0;
	assign data[1] = 29'b000110100_0_00011110_0_00000000_00; // reset
	assign data[2] = 29'b000110100_0_00001100_0_00000000_00; // power
	assign data[3] = 29'b000110100_0_00001000_0_00010101_00; // analog audio path control //fix**
	assign data[4] = 29'b000110100_0_00010000_0_00001100_00; // sample rate
	assign data[5] = 29'b000110100_0_00001110_0_01010010_00; // master mode
	assign data[6] = 29'b000110100_0_00000100_0_01111001_00; // left headphone
	assign data[7] = 29'b000110100_0_00000110_0_01111001_00; // right headphone
	assign data[8] = 29'b000110100_0_00001010_0_00000000_00; // digital audio path control //fix**
	assign data[9] = 29'b000110100_0_00010010_0_00000001_00; // active

	always @(*) begin
		send_next = send;
		start_next = start;
		finish_next = 1'b0;
		counter_reg_next = counter_reg;
		counter_data_next = counter_data;

		if(start)
			I2C_SDAT = data[counter_reg][counter_data]; //maybe have compile error?
		else
			I2C_SDAT = 1'b1;

		if(reset == 1'b1 && reset_prev == 1'b0)
			send_next = 1'b1;

		if (counter_data == 5'd0 && counter_reg == 4'd9) begin
			send_next = 1'b0;
			counter_reg_next = 4'b0;
		end
		
		if (send && start && (counter_data != 5'd0)) begin  // send signal
			if (!counter_clk[2]) begin
				if(counter_data == 5'd0)
					counter_data_next = 5'd28;
				else
					counter_data_next = counter_data - 5'b1;
			end
		end else if(send && start && (counter_data == 5'd0))begin  // send signal finish! 
			if(counter_clk[2])begin
				start_next = 1'b0;
				counter_data_next = 5'd28;
				finish_next = 1'b1;
			end
		end else if (send && (!start) && counter_clk[2] && (counter_reg != 4'd9)) begin //start send signal trigger;
			if(finish) begin
				if(counter_clk[1])
					I2C_SDAT = 1'b1;
				else
					I2C_SDAT = 1'b0;
			end else begin
				if(counter_clk[1])
					I2C_SDAT = 1'b0;
				else
					I2C_SDAT = 1'b1;
			end
			start_next = 1'b1;
			counter_reg_next = counter_reg + 4'b1; //data[0] will be empty!
		end
	end

	always @(posedge clk or negedge reset) begin
		if(!reset)
			counter_clk <= 3'b0;
		else
			counter_clk <= counter_clk + 3'b1;
	end
	
	always @(posedge counter_clk[1] or negedge reset) begin
		if (!reset) begin
			counter_data <= 5'd28;
			counter_reg  <= 4'b0;
			send         <= 1'b1;
			start        <= 1'b0;
			finish		 <= 1'b0;
			reset_prev   <= reset;
		end else begin
			counter_data <= counter_data_next;
			counter_reg  <= counter_reg_next;
			send         <= send_next;		
			start        <= start_next;
			finish		 <= finish_next;
			reset_prev   <= reset;
		end
	end
	
	
endmodule

