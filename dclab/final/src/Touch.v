module Touch(
	iclk,
	// from real hardware
	iADC_PENIRQ_n,
	iADC_DOUT,
	iADC_BUSY,
	oADC_DIN,
	oADC_DCLK,
	oSCEN,
	// internel signal
	o_data_len,
	o_send,
	o_data,
	o_address,
	o_write_en,
	o_write_data
);

	input			iADC_PENIRQ_n;
	input			iADC_DOUT;
	input			iADC_BUSY;
	output			oADC_DIN;
	output			oADC_DCLK;
	output			oSCEN;
	input			iclk;
	output	[5:0]	o_data_len;
	output 			o_send;
	output	[255:0]	o_data;
	output	[18:0]	o_address;
	output			o_write_en;
	output	[7:0]	o_write_data;
	reg				oADC_DIN;
	reg				iADC_PENIRQ_n_prev;
	reg				transmit_en;
	reg				transmit_en_next;
	reg				transmit_end;
	reg				transmit_finish;
	reg		[15:0]	DCLK_cnt;
	reg		[15:0]	DCLK_cnt_next;
	reg				oADC_DCLK;
	reg				oADC_DCLK_next;
	reg		[5:0]	DCLK_8_cnt;
	reg		[5:0]	DCLK_8_cnt_next;
	wire	[7:0]	x_config_reg;
	wire	[7:0]	y_config_reg;
	reg		[4:0]	transmit_cnt_next;
	reg		[4:0]	transmit_cnt;
	reg		[7:0]	x_coord_next;
	reg		[7:0]	y_coord_next;
	reg		[7:0]	x_coord;
	reg		[7:0]	y_coord;
	reg				transmit_end_prev;
	reg		[7:0]	x_ref_next;
	reg		[7:0]	y_ref_next;
	reg		[7:0]	x_ref;
	reg		[7:0]	y_ref;
	reg		[6:0]	data_cnt;
	reg		[6:0]	data_cnt_next;
	reg				x_diff_sign;
	reg				y_diff_sign;
	reg		[7:0]	x_diff;
	reg		[7:0]	y_diff;
	reg		[7:0]	xx;
	reg		[7:0]	yy;
	reg		[8:0]	distant;
	wire	[3:0]	out_theta;
	reg		[3:0]	data_theta	[63:0];
	reg		[3:0]	data_theta_next	[63:0];
	reg		[7:0]	store_cnt_next;
	reg		[7:0]	store_cnt;
	reg		[8:0]	store_x_next;
	reg		[8:0]	store_x;
	reg		[9:0]	store_y_next;
	reg		[9:0]	store_y;
	reg		[3:0]	store_data_next;
	reg		[3:0]	store_data;
	reg				write_en;
	wire	[12:0]	pos_x;
	wire	[12:0]	pos_y;
	reg		[18:0]	address;
	reg		[18:0]	reset_cnt_next;
	reg		[18:0]	reset_cnt;
	reg				reset_wr;

	integer i = 0;
	parameter SYSCLK_FRQ	= 12500000;
	parameter ADC_DCLK_FRQ	= 10000;
	parameter ADC_DCLK_CNT	= SYSCLK_FRQ/(ADC_DCLK_FRQ*2);


//touch detected
	always@(*) begin
		transmit_en_next = transmit_en;
		transmit_finish = 0;
		if(iADC_PENIRQ_n_prev && (~iADC_PENIRQ_n) && (~transmit_en))//iADC_PENIRQ_n from high to low
			transmit_en_next = 1;
		else if(transmit_en && transmit_end && iADC_PENIRQ_n)begin//end of transmit and no finger on touch panel
			transmit_en_next = 0;
			transmit_finish = 1;
		end
	end
	always@(posedge iclk ) begin
		iADC_PENIRQ_n_prev <= iADC_PENIRQ_n;
		transmit_en <= transmit_en_next;
	end
	
//ADC_DCLK
	always@(*)begin
		oADC_DCLK_next = oADC_DCLK;
		DCLK_cnt_next = DCLK_cnt;
		DCLK_8_cnt_next = DCLK_8_cnt;
		if(~transmit_en) begin
			oADC_DCLK_next = 0;
			DCLK_cnt_next = 0;
			DCLK_8_cnt_next = 0;
		end
		else if(DCLK_8_cnt[4])begin
			if(DCLK_cnt == ADC_DCLK_CNT) begin
				DCLK_cnt_next = 16'b0;
				DCLK_8_cnt_next = 5'b0;
			end
			else
				DCLK_cnt_next = DCLK_cnt + 16'b1;
		end
		else begin
			if(DCLK_cnt == ADC_DCLK_CNT) begin
				DCLK_cnt_next = 16'b0;
				oADC_DCLK_next = ~oADC_DCLK;
				DCLK_8_cnt_next = DCLK_8_cnt + 5'b1;
			end
			else
				DCLK_cnt_next = DCLK_cnt + 16'b1;
		end
		
	end
	always@(posedge iclk ) begin
			oADC_DCLK <= oADC_DCLK_next;
			DCLK_cnt <= DCLK_cnt_next;
			DCLK_8_cnt <= DCLK_8_cnt_next;
	end
	
//get x y coordinary
	assign x_config_reg  = 8'b10011010;
	assign y_config_reg  = 8'b11011010;
	always@(*)begin
		transmit_cnt_next = transmit_cnt + 5'b1;
		x_coord_next = x_coord;
		y_coord_next = y_coord;
		transmit_end = 0;
		oADC_DIN = 0;
		if(iADC_PENIRQ_n_prev && (~iADC_PENIRQ_n) && (~transmit_en))begin//first time touching the panel and reset to get coord
			transmit_cnt_next = 0;
		end
		if(transmit_cnt == 5'd29)begin
			transmit_cnt_next = transmit_cnt;
			transmit_end = 1;
			 if(~transmit_finish)begin //continue ot get coord
				transmit_cnt_next = 0;
			end
		end
		if(transmit_cnt <= 5'd7)
			oADC_DIN = x_config_reg[7-transmit_cnt];
		if(transmit_cnt >= 5'd11 && transmit_cnt <= 5'd18)
			oADC_DIN = y_config_reg[18-transmit_cnt];
		if(transmit_cnt >= 5'd9 && transmit_cnt <= 5'd16)
			x_coord_next = {x_coord[6:0], iADC_DOUT};
		if(transmit_cnt >= 5'd20 && transmit_cnt <= 5'd27)
			y_coord_next = {y_coord[6:0], iADC_DOUT};
	end
	always@(negedge oADC_DCLK)begin
			transmit_cnt <= transmit_cnt_next;
			x_coord <= x_coord_next;
			y_coord <= y_coord_next;
	end

//choose the point to record
	Atan2 atan(
		.i_xSign(x_diff_sign),
		.i_ySign(y_diff_sign),
		.i_x(x_diff),
		.i_y(y_diff),
		.o_theta(out_theta)
	);
	always@(*) begin
		x_ref_next = x_ref;
		y_ref_next = y_ref;
		data_cnt_next = data_cnt;
		x_diff_sign = 0;
		x_diff = 8'b0;
		y_diff_sign = 0;
		y_diff = 8'b0;
		for(i = 0; i < 64; i = i + 1)
			data_theta_next[i] = data_theta[i];
		xx = 8'b0;
		yy = 8'b0;
		distant = 9'b0;
		if(iADC_PENIRQ_n_prev && (~iADC_PENIRQ_n) && (~transmit_en))begin //first time touching the panel and reset data
			data_cnt_next = 6'b0;
		end
		if((~transmit_end_prev) && transmit_end)begin
			if(data_cnt == 0)begin
				x_ref_next = x_coord;
				y_ref_next = y_coord;
				data_cnt_next = data_cnt + 7'b1;
			end
			else if(data_cnt[6])
				data_cnt_next = data_cnt;
			else begin
				if(x_coord >= x_ref)begin
					x_diff_sign = 0;
					x_diff = x_coord - x_ref;
				end
				else begin
					x_diff_sign = 1;
					x_diff = x_ref - x_coord;
				end
				
				if(y_coord >= y_ref)begin
					y_diff_sign = 0;
					y_diff = y_coord - y_ref;
				end
				else begin
					y_diff_sign = 1;
					y_diff = y_ref - y_coord;
				end
				
				if(x_diff > 8'd16)begin
					data_theta_next[data_cnt-1] = out_theta;
					x_ref_next = x_coord;
					y_ref_next = y_coord;
					data_cnt_next = data_cnt + 7'b1;
				end
				else if(y_diff > 8'd16)begin
					data_theta_next[data_cnt-1] = out_theta;
					x_ref_next = x_coord;
					y_ref_next = y_coord;
					data_cnt_next = data_cnt + 7'b1;
				end
				else begin
					xx = x_diff[3:0] * x_diff[3:0];
					yy = y_diff[3:0] * y_diff[3:0];
					distant = xx + yy;
					if(distant[8])begin
						data_theta_next[data_cnt-1] = out_theta;
						x_ref_next = x_coord;
						y_ref_next = y_coord;
						data_cnt_next = data_cnt + 7'b1;
					end
				end
			end
		end
	end
	always@(posedge iclk)begin
			transmit_end_prev <= transmit_end;
			x_ref <= x_ref_next;
			y_ref <= y_ref_next;
			data_cnt <= data_cnt_next;
			for(i = 0; i < 64; i = i + 1)
				data_theta[i] <= data_theta_next[i];
	end

//output for compare
	assign oSCEN = transmit_en;
	assign o_data_len = (|data_cnt)? (data_cnt - 1) : 6'b0;
	assign o_send = (transmit_end && (~transmit_en))? ((data_cnt > 6'd5)? 1:0):0;
	assign o_data[3:0] = data_theta[0];
	assign o_data[7:4] = data_theta[1];
	assign o_data[11:8] = data_theta[2];
	assign o_data[15:12] = data_theta[3];
	assign o_data[19:16] = data_theta[4];
	assign o_data[23:20] = data_theta[5];
	assign o_data[27:24] = data_theta[6];
	assign o_data[31:28] = data_theta[7];
	assign o_data[35:32] = data_theta[8];
	assign o_data[39:36] = data_theta[9];
	assign o_data[43:40] = data_theta[10];
	assign o_data[47:44] = data_theta[11];
	assign o_data[51:48] = data_theta[12];
	assign o_data[55:52] = data_theta[13];
	assign o_data[59:56] = data_theta[14];
	assign o_data[63:60] = data_theta[15];
	assign o_data[67:64] = data_theta[16];
	assign o_data[71:68] = data_theta[17];
	assign o_data[75:72] = data_theta[18];
	assign o_data[79:76] = data_theta[19];
	assign o_data[83:80] = data_theta[20];
	assign o_data[87:84] = data_theta[21];
	assign o_data[91:88] = data_theta[22];
	assign o_data[95:92] = data_theta[23];
	assign o_data[99:96] = data_theta[24];
	assign o_data[103:100] = data_theta[25];
	assign o_data[107:104] = data_theta[26];
	assign o_data[111:108] = data_theta[27];
	assign o_data[115:112] = data_theta[28];
	assign o_data[119:116] = data_theta[29];
	assign o_data[123:120] = data_theta[30];
	assign o_data[127:124] = data_theta[31];
	assign o_data[131:128] = data_theta[32];
	assign o_data[135:132] = data_theta[33];
	assign o_data[139:136] = data_theta[34];
	assign o_data[143:140] = data_theta[35];
	assign o_data[147:144] = data_theta[36];
	assign o_data[151:148] = data_theta[37];
	assign o_data[155:152] = data_theta[38];
	assign o_data[159:156] = data_theta[39];
	assign o_data[163:160] = data_theta[40];
	assign o_data[167:164] = data_theta[41];
	assign o_data[171:168] = data_theta[42];
	assign o_data[175:172] = data_theta[43];
	assign o_data[179:176] = data_theta[44];
	assign o_data[183:180] = data_theta[45];
	assign o_data[187:184] = data_theta[46];
	assign o_data[191:188] = data_theta[47];
	assign o_data[195:192] = data_theta[48];
	assign o_data[199:196] = data_theta[49];
	assign o_data[203:200] = data_theta[50];
	assign o_data[207:204] = data_theta[51];
	assign o_data[211:208] = data_theta[52];
	assign o_data[215:212] = data_theta[53];
	assign o_data[219:216] = data_theta[54];
	assign o_data[223:220] = data_theta[55];
	assign o_data[227:224] = data_theta[56];
	assign o_data[231:228] = data_theta[57];
	assign o_data[235:232] = data_theta[58];
	assign o_data[239:236] = data_theta[59];
	assign o_data[243:240] = data_theta[60];
	assign o_data[247:244] = data_theta[61];
	assign o_data[251:248] = data_theta[62];
	assign o_data[255:252] = data_theta[63];

// store data path?
	assign pos_x = (x_coord * 5'b11110);
	assign pos_y = (y_coord * 5'b11001);
	assign o_address = (reset_wr)? reset_cnt_next : address;
	assign o_write_en = write_en || reset_wr;
	assign o_write_data = (reset_wr)? 8'hff : {store_data,4'b0};
	always@(*)begin
		store_cnt_next = store_cnt;
		store_x_next = store_x;
		store_y_next = store_y;
		store_data_next = store_data;//this maybe can be delete;
		write_en = 0;
		address = 19'b0;
		if(data_cnt[6] || (&data_cnt[5:2]))
			store_data_next = 4'he;
		else
			store_data_next = data_cnt[5:2];
		if((~transmit_end_prev) && transmit_end && (~transmit_finish) && (~data_cnt[6]))begin
			store_cnt_next = 8'd124;
			store_x_next = pos_x[12:4];
			store_y_next = pos_y[12:3];
			address = 0;
		end
		if(|store_cnt)begin
			if(store_cnt == 8'd124)begin
				store_x_next = store_x + 10'd1;
				store_y_next = store_y - 9'd6;
				address = {store_y_next, store_x_next};
				write_en = 1;
				store_cnt_next = store_cnt - 8'b1;
			end
			else if(store_cnt == 8'd2 || store_cnt == 8'd122)begin
				store_x_next = store_x - 10'd2;
				store_y_next = store_y - 9'd1;
				address = {store_y_next, store_x_next};
				write_en = 1;
				store_cnt_next = store_cnt - 8'b1;
			end
			else if(store_cnt == 8'd6 || store_cnt == 8'd118)begin
				store_x_next = store_x - 10'd5;
				store_y_next = store_y - 9'd1;
				address = {store_y_next, store_x_next};
				write_en = 1;
				store_cnt_next = store_cnt - 8'b1;
			end
			else if(store_cnt == 8'd14 || store_cnt == 8'd110)begin
				store_x_next = store_x - 10'd8;
				store_y_next = store_y - 9'd1;
				address = {store_y_next, store_x_next};
				write_en = 1;
				store_cnt_next = store_cnt - 8'b1;
			end
			else if(store_cnt == 8'd24 || store_cnt == 8'd100)begin
				store_x_next = store_x - 10'd10;
				store_y_next = store_y - 9'd1;
				address = {store_y_next, store_x_next};
				write_en = 1;
				store_cnt_next = store_cnt - 8'b1;
			end
			else if(store_cnt == 8'd36 || store_cnt == 8'd88)begin
				store_x_next = store_x - 10'd11;
				store_y_next = store_y - 9'd1;
				address = {store_y_next, store_x_next};
				write_en = 1;
				store_cnt_next = store_cnt - 8'b1;
			end
			else if(store_cnt == 8'd48 || store_cnt == 8'd76)begin
				store_x_next = store_x - 10'd12;
				store_y_next = store_y - 9'd1;
				address = {store_y_next, store_x_next};
				write_en = 1;
				store_cnt_next = store_cnt - 8'b1;
			end
			else if(store_cnt == 8'd62)begin
				store_x_next = store_x - 10'd13;
				store_y_next = store_y - 9'd1;
				address = {store_y_next, store_x_next};
				write_en = 1;
				store_cnt_next = store_cnt - 8'b1;
			end
			else begin
				store_x_next = store_x + 10'd1;
				store_y_next = store_y;
				address = {store_y_next, store_x_next};
				write_en = 1;
				store_cnt_next = store_cnt - 8'b1;
			end
		end
	end
	
	always@(posedge iclk )begin
			store_cnt <= store_cnt_next;
			store_x <= store_x_next;
			store_y <= store_y_next;
			store_data <= store_data_next;
	end
	
	always@(*)begin
		reset_cnt_next = reset_cnt;
		reset_wr = 0;
		if(iADC_PENIRQ_n_prev && (~iADC_PENIRQ_n) && (~transmit_en))begin//first time touching the panel and reset to get coord
			reset_cnt_next = 19'd409600;
		end
		if(|reset_cnt)begin
			reset_cnt_next = reset_cnt-19'b1;
			reset_wr = 1;
		end
	end
	always@(posedge iclk )begin
		reset_cnt <= reset_cnt_next;
	end
	
endmodule
