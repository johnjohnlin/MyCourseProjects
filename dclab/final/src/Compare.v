module Compare(
	i_clk,
	i_stroke1,
	i_stroke2,
	i_len1,
	i_len2,
	i_start,
	i_dataR,
	o_rw,
	o_addr1,
	o_addr2,
	o_dataW,
	o_done,
	o_score
);
`define STR_BITS 256
`define DAT_BITS 8
`define ADR_BITS 6
`define S_INIT_MEM 1'b0
`define S_CAL 1'b1
	input i_clk;
	input [`STR_BITS-1:0] i_stroke1;
	input [`STR_BITS-1:0] i_stroke2;
	input [`ADR_BITS-1:0] i_len1;
	input [`ADR_BITS-1:0] i_len2;
	input i_start;
	input [`DAT_BITS-1:0] i_dataR;
	output o_rw; // 1/0 = R/W
	output [`ADR_BITS-1:0] o_addr1;
	output [`ADR_BITS-1:0] o_addr2;
	output [`DAT_BITS-1:0] o_dataW;
	output o_done;
	output [`DAT_BITS-1:0] o_score;

	reg state, state_next;
	reg done, done_next;
	reg [`ADR_BITS-1:0] cur1, cur2, cur1_next, cur2_next, len1_m1_buf, len2_m1_buf;
	// relaxPos (refer to compare.c)
	// 000: step(0, 0) R stage
	// 010: step(0, 0) R stage
	// 011: step(0, 0) W stage
	// 100: step(0, 1) R stage
	// 101: step(0, 1) W stage
	// 110: step(1, 1) R stage
	// 111: step(1, 1) W stage
	reg [2:0] relaxPos, relaxPos_next; //used for DP
	reg [`DAT_BITS-1:0] dataW, dataW_next;
	reg [`DAT_BITS-1:0] srcScore;
	reg [3:0] t1, t2;
	wire negStart;
	wire [3:0] diff;
	wire count1Done, count2Done;
	wire [`ADR_BITS-1:0] cur1_p1;
	wire [`ADR_BITS-1:0] cur2_p1;
	wire [`DAT_BITS-1:0] newScore;
	assign o_score = i_dataR;
	assign o_done = done;
	assign o_rw = relaxPos[0];
	assign o_addr1 = (relaxPos[1]? cur1_p1: cur1);
	assign o_addr2 = (relaxPos[2]? cur2_p1: cur2);
	assign o_dataW = dataW;
	assign cur1_p1 = cur1 + `ADR_BITS'b1;
	assign cur2_p1 = cur2 + `ADR_BITS'b1;
	assign count1Done = (len1_m1_buf == cur1);
	assign count2Done = (len2_m1_buf == cur2);
	assign newScore = diff + srcScore;

	always @(*) begin
		case(cur1)
		6'd0: t1 = i_stroke1[3:0];
		6'd1: t1 = i_stroke1[7:4];
		6'd2: t1 = i_stroke1[11:8];
		6'd3: t1 = i_stroke1[15:12];
		6'd4: t1 = i_stroke1[19:16];
		6'd5: t1 = i_stroke1[23:20];
		6'd6: t1 = i_stroke1[27:24];
		6'd7: t1 = i_stroke1[31:28];
		6'd8: t1 = i_stroke1[35:32];
		6'd9: t1 = i_stroke1[39:36];
		6'd10: t1 = i_stroke1[43:40];
		6'd11: t1 = i_stroke1[47:44];
		6'd12: t1 = i_stroke1[51:48];
		6'd13: t1 = i_stroke1[55:52];
		6'd14: t1 = i_stroke1[59:56];
		6'd15: t1 = i_stroke1[63:60];
		6'd16: t1 = i_stroke1[67:64];
		6'd17: t1 = i_stroke1[71:68];
		6'd18: t1 = i_stroke1[75:72];
		6'd19: t1 = i_stroke1[79:76];
		6'd20: t1 = i_stroke1[83:80];
		6'd21: t1 = i_stroke1[87:84];
		6'd22: t1 = i_stroke1[91:88];
		6'd23: t1 = i_stroke1[95:92];
		6'd24: t1 = i_stroke1[99:96];
		6'd25: t1 = i_stroke1[103:100];
		6'd26: t1 = i_stroke1[107:104];
		6'd27: t1 = i_stroke1[111:108];
		6'd28: t1 = i_stroke1[115:112];
		6'd29: t1 = i_stroke1[119:116];
		6'd30: t1 = i_stroke1[123:120];
		6'd31: t1 = i_stroke1[127:124];
		6'd32: t1 = i_stroke1[131:128];
		6'd33: t1 = i_stroke1[135:132];
		6'd34: t1 = i_stroke1[139:136];
		6'd35: t1 = i_stroke1[143:140];
		6'd36: t1 = i_stroke1[147:144];
		6'd37: t1 = i_stroke1[151:148];
		6'd38: t1 = i_stroke1[155:152];
		6'd39: t1 = i_stroke1[159:156];
		6'd40: t1 = i_stroke1[163:160];
		6'd41: t1 = i_stroke1[167:164];
		6'd42: t1 = i_stroke1[171:168];
		6'd43: t1 = i_stroke1[175:172];
		6'd44: t1 = i_stroke1[179:176];
		6'd45: t1 = i_stroke1[183:180];
		6'd46: t1 = i_stroke1[187:184];
		6'd47: t1 = i_stroke1[191:188];
		6'd48: t1 = i_stroke1[195:192];
		6'd49: t1 = i_stroke1[199:196];
		6'd50: t1 = i_stroke1[203:200];
		6'd51: t1 = i_stroke1[207:204];
		6'd52: t1 = i_stroke1[211:208];
		6'd53: t1 = i_stroke1[215:212];
		6'd54: t1 = i_stroke1[219:216];
		6'd55: t1 = i_stroke1[223:220];
		6'd56: t1 = i_stroke1[227:224];
		6'd57: t1 = i_stroke1[231:228];
		6'd58: t1 = i_stroke1[235:232];
		6'd59: t1 = i_stroke1[239:236];
		6'd60: t1 = i_stroke1[243:240];
		6'd61: t1 = i_stroke1[247:244];
		6'd62: t1 = i_stroke1[251:248];
		6'd63: t1 = i_stroke1[255:252];
		endcase
		case(cur2)
		6'd0: t2 = i_stroke2[3:0];
		6'd1: t2 = i_stroke2[7:4];
		6'd2: t2 = i_stroke2[11:8];
		6'd3: t2 = i_stroke2[15:12];
		6'd4: t2 = i_stroke2[19:16];
		6'd5: t2 = i_stroke2[23:20];
		6'd6: t2 = i_stroke2[27:24];
		6'd7: t2 = i_stroke2[31:28];
		6'd8: t2 = i_stroke2[35:32];
		6'd9: t2 = i_stroke2[39:36];
		6'd10: t2 = i_stroke2[43:40];
		6'd11: t2 = i_stroke2[47:44];
		6'd12: t2 = i_stroke2[51:48];
		6'd13: t2 = i_stroke2[55:52];
		6'd14: t2 = i_stroke2[59:56];
		6'd15: t2 = i_stroke2[63:60];
		6'd16: t2 = i_stroke2[67:64];
		6'd17: t2 = i_stroke2[71:68];
		6'd18: t2 = i_stroke2[75:72];
		6'd19: t2 = i_stroke2[79:76];
		6'd20: t2 = i_stroke2[83:80];
		6'd21: t2 = i_stroke2[87:84];
		6'd22: t2 = i_stroke2[91:88];
		6'd23: t2 = i_stroke2[95:92];
		6'd24: t2 = i_stroke2[99:96];
		6'd25: t2 = i_stroke2[103:100];
		6'd26: t2 = i_stroke2[107:104];
		6'd27: t2 = i_stroke2[111:108];
		6'd28: t2 = i_stroke2[115:112];
		6'd29: t2 = i_stroke2[119:116];
		6'd30: t2 = i_stroke2[123:120];
		6'd31: t2 = i_stroke2[127:124];
		6'd32: t2 = i_stroke2[131:128];
		6'd33: t2 = i_stroke2[135:132];
		6'd34: t2 = i_stroke2[139:136];
		6'd35: t2 = i_stroke2[143:140];
		6'd36: t2 = i_stroke2[147:144];
		6'd37: t2 = i_stroke2[151:148];
		6'd38: t2 = i_stroke2[155:152];
		6'd39: t2 = i_stroke2[159:156];
		6'd40: t2 = i_stroke2[163:160];
		6'd41: t2 = i_stroke2[167:164];
		6'd42: t2 = i_stroke2[171:168];
		6'd43: t2 = i_stroke2[175:172];
		6'd44: t2 = i_stroke2[179:176];
		6'd45: t2 = i_stroke2[183:180];
		6'd46: t2 = i_stroke2[187:184];
		6'd47: t2 = i_stroke2[191:188];
		6'd48: t2 = i_stroke2[195:192];
		6'd49: t2 = i_stroke2[199:196];
		6'd50: t2 = i_stroke2[203:200];
		6'd51: t2 = i_stroke2[207:204];
		6'd52: t2 = i_stroke2[211:208];
		6'd53: t2 = i_stroke2[215:212];
		6'd54: t2 = i_stroke2[219:216];
		6'd55: t2 = i_stroke2[223:220];
		6'd56: t2 = i_stroke2[227:224];
		6'd57: t2 = i_stroke2[231:228];
		6'd58: t2 = i_stroke2[235:232];
		6'd59: t2 = i_stroke2[239:236];
		6'd60: t2 = i_stroke2[243:240];
		6'd61: t2 = i_stroke2[247:244];
		6'd62: t2 = i_stroke2[251:248];
		6'd63: t2 = i_stroke2[255:252];
		endcase
	end

	NegDetector ng1(.clk(i_clk), .sig(i_start), .out(negStart));
	DiffTheta d1(.i_t1(t1), .i_t2(t2), .o_diff(diff));

	always @(*) begin
		cur1_next = cur1;
		cur2_next = cur2;
		state_next = state;
		relaxPos_next = relaxPos;
		done_next = done;
		dataW_next = dataW;
		if (negStart) begin
			state_next = `S_INIT_MEM;
			cur1_next = `ADR_BITS'b0;
			cur2_next = `ADR_BITS'b0;
			done_next = 1'b0;
			relaxPos_next = 3'h1;
			dataW_next = `DAT_BITS'b0;
		end else if (count1Done & count2Done & (state == `S_INIT_MEM)) begin
			state_next = `S_CAL;
			cur1_next = `ADR_BITS'b0;
			cur2_next = `ADR_BITS'b0;
			relaxPos_next = 3'h0;
		end else if (state == `S_INIT_MEM) begin
			// initialize the memory
			cur1_next = count1Done? `ADR_BITS'b0 : cur1_p1;
			cur2_next = count1Done? cur2_p1: cur2;
			dataW_next = {1'b1, {(`DAT_BITS-2){1'b0}}};
		end else begin
			// state == `S_CAL
			relaxPos_next = (relaxPos == 3'b0 ? 3'h2: relaxPos + 3'b1);
			dataW_next = newScore > i_dataR ? i_dataR: newScore;
			//dataW_next = newScore;
			if (relaxPos == 3'h7) begin
				cur1_next = count1Done? `ADR_BITS'b0 : cur1_p1;
				cur2_next = count1Done? cur2_p1: cur2;
			end
			if (count1Done & count2Done) begin
				done_next = 1'b1;
				relaxPos_next = 3'b0;
			end
		end
	end

	always @(posedge i_clk) begin
		state <= state_next;
		cur1 <= cur1_next;
		cur2 <= cur2_next;
		len1_m1_buf <= negStart? (i_len1 - `ADR_BITS'b1): len1_m1_buf;
		len2_m1_buf <= negStart? (i_len2 - `ADR_BITS'b1): len2_m1_buf;
		relaxPos <= relaxPos_next;
		done <= done_next;
		dataW <= dataW_next;

		srcScore <= (relaxPos == 3'b0 ? i_dataR: srcScore);
	end
endmodule
