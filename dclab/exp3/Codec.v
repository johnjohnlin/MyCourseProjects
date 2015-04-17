module Codec(
	AUD_BCLK,       //audio
	AUD_DACLRCK,    //audio
	AUD_DACDAT,     //audio
	fast,           //control
	slow,			//control
	rate,           //control
	stop,			//control
	record,         //control
	interp,         //control
	data_fr_sram,   //sram
	read,           //sram
	AUD_ADCLRCK,    //audio
	AUD_ADCDAT,     //audio
	data_to_sram,   //sram
	write,          //sram
	address,        //sram
	counter
);
	output [4:0] counter;
	input  AUD_BCLK, AUD_DACLRCK, AUD_ADCLRCK, AUD_ADCDAT;
	input  interp, record, stop, fast, slow;
	input [3:0] rate;
	input [15:0] data_fr_sram;
	output read;
	output [17:0] address;

	output AUD_DACDAT;
	output [15:0] data_to_sram;
	output write;
	
	reg 		AUD_DACDAT;
	reg	[17:0]	addr, addr_next;
	reg [15:0]	data_write, data_write_next, data_to_sram;
	reg [4:0]	counter, counter_next;
	reg 		ADCLRCK_prev, write;
	
	reg	[15:0]	data_read, data_read_next;
	reg			DACLRCK_prev, read;
	reg [3:0]	counter_interp, counter_interp_next;
	
	assign address = addr;
	
	always @(*) begin
		addr_next = addr;
		data_write_next = data_write;
		data_read_next = data_read;
		read = 1'b0;
		write = 1'b0;
		data_to_sram = 16'bxxxxxxxxxxxxxxxx;
		AUD_DACDAT = 1'b0;
		counter_next = counter;
		counter_interp_next = counter_interp;
		if (stop) begin              // control for stop
			addr_next = 18'b0;
			data_write_next = 16'b0;
			counter_next = 5'b0;
			counter_interp_next = 4'b0;
			data_read_next = 16'b0;
		end else if (record) begin// record  mode -- four-state ADCLRCK 0-1 1-1 (1-0 0-0)
			if(ADCLRCK_prev == 1'b0 && AUD_ADCLRCK == 1'b1) begin
				if (&addr) begin              //first data of sram will be empty
					addr_next = addr;          //and after the last data of sram
				end else begin
					addr_next = addr + 18'b1;
				end
				data_write_next = 16'b0;
				counter_next = 5'b0;
			end else if(ADCLRCK_prev == 1'b1 && AUD_ADCLRCK == 1'b1) begin
				if (counter[4] == 1) begin
					counter_next = counter;
				end else begin
					data_write_next[counter[3:0]] = AUD_ADCDAT; //maybe have compile error
					counter_next = counter + 5'b1;
				end
			end else begin //output for sram
				if (counter[4] == 1) begin
					write = 1'b1;
					data_to_sram = data_write;
				end
			end
		end else begin //play mode -- four-state ADCLRCK 1-0 0-0 0-1 1-1
			if(DACLRCK_prev == 1'b1 && AUD_DACLRCK == 1'b0) begin
				read = 1'b1;
				data_read_next = data_fr_sram;
				counter_next = 5'b0;
				if (slow && !interp)begin // zero-order slow
					counter_interp_next = counter_interp + 4'b1;
					if (counter_interp_next == rate)begin
						if(&addr)
							addr_next = addr;
						else
							addr_next = addr + 18'b1;
						counter_interp_next = 4'b0;
					end else begin
						addr_next = addr;
					end
				end	else if (fast) begin // fast 
					addr_next = addr + {14'b0, rate};
					if (addr[17] && (!addr_next[17])) begin
						addr_next = 18'b11_1111_1111_1111_1111;
					end
				end else begin   //normal and first-order slow
					if (&addr) begin         //first data of sram will be empty
						addr_next = addr;     //and after the last data of sram ,the recording will be stoped
					end else begin
						addr_next = addr + 18'b1;
					end
				end
			end else if(DACLRCK_prev == 1'b0 && AUD_DACLRCK == 1'b0) begin
				if (counter[4] == 1) begin
					counter_next = counter;
					AUD_DACDAT = 1'b0;
				end else begin
					counter_next = counter + 5'b1;
					AUD_DACDAT = data_read[counter]; //maybe have compile error
				end
			end else if(DACLRCK_prev == 1'b0 && AUD_DACLRCK == 1'b1) begin
				counter_next = 5'b0;
			end else begin
				if (counter[4] == 1) begin
					counter_next = counter;
					AUD_DACDAT = 1'b0;
				end else begin
					counter_next = counter + 5'b1;
					AUD_DACDAT = data_read[counter]; //maybe have compile error
				end
			end		
		end
	end
	
	always@(posedge AUD_BCLK)begin
		ADCLRCK_prev <= AUD_ADCLRCK;
		data_write <= data_write_next;
		DACLRCK_prev <= AUD_DACLRCK;
		addr <= addr_next;
		data_read <= data_read_next;
		counter <= counter_next;
		counter_interp <= counter_interp_next;
	end

endmodule
