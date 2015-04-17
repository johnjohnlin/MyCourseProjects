module Proj2(
	clk,
	reset,
	ready,
	we,
	oe,
	start,
	reg_sel,
	addr,
	data_i,
	data_o,
	done
);

	input        clk;
	input        reset;
	
	input        we;
	input        oe;
	input        start;
	input  [1:0] reg_sel;
	input  [4:0] addr;
	input  [7:0] data_i;
	output [7:0] data_o;
	wire [255:0] a_0_result;
	output       done;
	output       ready;
	//
	reg          ready;
	reg    [7:0] data_o;
	reg  [255:0] a_0;
	reg  [255:0] a_1;
	reg  [255:0] a_2;
	reg  [255:0] a_3;
	reg  [255:0] a_0_next;
	reg  [255:0] a_1_next;
	reg  [255:0] a_2_next;
	reg  [255:0] a_3_next;
	reg          ready_next;
	wire         done;

// power module
	Power power(
		.clk(clk),
		.start(start),
		.a1(a_1),
		.a2(a_2),
		.a3(a_3),
		.done(done),
		.a0(a_0_result)
	);


	always @(*)begin
		a_0_next = a_0;
		a_1_next = a_1;
		a_2_next = a_2;
		a_3_next = a_3;
		data_o   = 8'b0;
		ready_next = ready;
		if(~ready) begin
			if(~we) begin
				case(reg_sel)
					2'b00:	case(addr)
								5'd0:a_0_next[7:0] = data_i;
								5'd1:a_0_next[15:8] = data_i;
								5'd2:a_0_next[23:16] = data_i;
								5'd3:a_0_next[31:24] = data_i;
								5'd4:a_0_next[39:32] = data_i;
								5'd5:a_0_next[47:40] = data_i;
								5'd6:a_0_next[55:48] = data_i;
								5'd7:a_0_next[63:56] = data_i;
								5'd8:a_0_next[71:64] = data_i;
								5'd9:a_0_next[79:72] = data_i;
								5'd10:a_0_next[87:80] = data_i;
								5'd11:a_0_next[95:88] = data_i;
								5'd12:a_0_next[103:96] = data_i;
								5'd13:a_0_next[111:104] = data_i;
								5'd14:a_0_next[119:112] = data_i;
								5'd15:a_0_next[127:120] = data_i;
								5'd16:a_0_next[135:128] = data_i;
								5'd17:a_0_next[143:136] = data_i;
								5'd18:a_0_next[151:144] = data_i;
								5'd19:a_0_next[159:152] = data_i;
								5'd20:a_0_next[167:160] = data_i;
								5'd21:a_0_next[175:168] = data_i;
								5'd22:a_0_next[183:176] = data_i;
								5'd23:a_0_next[191:184] = data_i;
								5'd24:a_0_next[199:192] = data_i;
								5'd25:a_0_next[207:200] = data_i;
								5'd26:a_0_next[215:208] = data_i;
								5'd27:a_0_next[223:216] = data_i;
								5'd28:a_0_next[231:224] = data_i;
								5'd29:a_0_next[239:232] = data_i;
								5'd30:a_0_next[247:240] = data_i;
								5'd31:a_0_next[255:248] = data_i;
								default:a_0_next = a_0;
							endcase
					2'b01:case(addr)
								5'd0:a_1_next[7:0] = data_i;
								5'd1:a_1_next[15:8] = data_i;
								5'd2:a_1_next[23:16] = data_i;
								5'd3:a_1_next[31:24] = data_i;
								5'd4:a_1_next[39:32] = data_i;
								5'd5:a_1_next[47:40] = data_i;
								5'd6:a_1_next[55:48] = data_i;
								5'd7:a_1_next[63:56] = data_i;
								5'd8:a_1_next[71:64] = data_i;
								5'd9:a_1_next[79:72] = data_i;
								5'd10:a_1_next[87:80] = data_i;
								5'd11:a_1_next[95:88] = data_i;
								5'd12:a_1_next[103:96] = data_i;
								5'd13:a_1_next[111:104] = data_i;
								5'd14:a_1_next[119:112] = data_i;
								5'd15:a_1_next[127:120] = data_i;
								5'd16:a_1_next[135:128] = data_i;
								5'd17:a_1_next[143:136] = data_i;
								5'd18:a_1_next[151:144] = data_i;
								5'd19:a_1_next[159:152] = data_i;
								5'd20:a_1_next[167:160] = data_i;
								5'd21:a_1_next[175:168] = data_i;
								5'd22:a_1_next[183:176] = data_i;
								5'd23:a_1_next[191:184] = data_i;
								5'd24:a_1_next[199:192] = data_i;
								5'd25:a_1_next[207:200] = data_i;
								5'd26:a_1_next[215:208] = data_i;
								5'd27:a_1_next[223:216] = data_i;
								5'd28:a_1_next[231:224] = data_i;
								5'd29:a_1_next[239:232] = data_i;
								5'd30:a_1_next[247:240] = data_i;
								5'd31:a_1_next[255:248] = data_i;
								default:a_1_next = a_1;
							endcase
					2'b10:case(addr)
								5'd0:a_2_next[7:0] = data_i;
								5'd1:a_2_next[15:8] = data_i;
								5'd2:a_2_next[23:16] = data_i;
								5'd3:a_2_next[31:24] = data_i;
								5'd4:a_2_next[39:32] = data_i;
								5'd5:a_2_next[47:40] = data_i;
								5'd6:a_2_next[55:48] = data_i;
								5'd7:a_2_next[63:56] = data_i;
								5'd8:a_2_next[71:64] = data_i;
								5'd9:a_2_next[79:72] = data_i;
								5'd10:a_2_next[87:80] = data_i;
								5'd11:a_2_next[95:88] = data_i;
								5'd12:a_2_next[103:96] = data_i;
								5'd13:a_2_next[111:104] = data_i;
								5'd14:a_2_next[119:112] = data_i;
								5'd15:a_2_next[127:120] = data_i;
								5'd16:a_2_next[135:128] = data_i;
								5'd17:a_2_next[143:136] = data_i;
								5'd18:a_2_next[151:144] = data_i;
								5'd19:a_2_next[159:152] = data_i;
								5'd20:a_2_next[167:160] = data_i;
								5'd21:a_2_next[175:168] = data_i;
								5'd22:a_2_next[183:176] = data_i;
								5'd23:a_2_next[191:184] = data_i;
								5'd24:a_2_next[199:192] = data_i;
								5'd25:a_2_next[207:200] = data_i;
								5'd26:a_2_next[215:208] = data_i;
								5'd27:a_2_next[223:216] = data_i;
								5'd28:a_2_next[231:224] = data_i;
								5'd29:a_2_next[239:232] = data_i;
								5'd30:a_2_next[247:240] = data_i;
								5'd31:a_2_next[255:248] = data_i;
								default:a_2_next = a_2;
							endcase
					2'b11:case(addr)
								5'd0:a_3_next[7:0] = data_i;
								5'd1:a_3_next[15:8] = data_i;
								5'd2:a_3_next[23:16] = data_i;
								5'd3:a_3_next[31:24] = data_i;
								5'd4:a_3_next[39:32] = data_i;
								5'd5:a_3_next[47:40] = data_i;
								5'd6:a_3_next[55:48] = data_i;
								5'd7:a_3_next[63:56] = data_i;
								5'd8:a_3_next[71:64] = data_i;
								5'd9:a_3_next[79:72] = data_i;
								5'd10:a_3_next[87:80] = data_i;
								5'd11:a_3_next[95:88] = data_i;
								5'd12:a_3_next[103:96] = data_i;
								5'd13:a_3_next[111:104] = data_i;
								5'd14:a_3_next[119:112] = data_i;
								5'd15:a_3_next[127:120] = data_i;
								5'd16:a_3_next[135:128] = data_i;
								5'd17:a_3_next[143:136] = data_i;
								5'd18:a_3_next[151:144] = data_i;
								5'd19:a_3_next[159:152] = data_i;
								5'd20:a_3_next[167:160] = data_i;
								5'd21:a_3_next[175:168] = data_i;
								5'd22:a_3_next[183:176] = data_i;
								5'd23:a_3_next[191:184] = data_i;
								5'd24:a_3_next[199:192] = data_i;
								5'd25:a_3_next[207:200] = data_i;
								5'd26:a_3_next[215:208] = data_i;
								5'd27:a_3_next[223:216] = data_i;
								5'd28:a_3_next[231:224] = data_i;
								5'd29:a_3_next[239:232] = data_i;
								5'd30:a_3_next[247:240] = data_i;
								5'd31:a_3_next[255:248] = data_i;
								default:a_3_next = a_3;
							endcase
					default:a_0_next = a_0;
				endcase
			end
			else if(~oe) begin
				case(reg_sel)				
					2'b00:	case(addr)
								5'd0:data_o = a_0_next[7:0];
								5'd1:data_o = a_0_next[15:8];
								5'd2:data_o = a_0_next[23:16];
								5'd3:data_o = a_0_next[31:24];
								5'd4:data_o = a_0_next[39:32];
								5'd5:data_o = a_0_next[47:40];
								5'd6:data_o = a_0_next[55:48];
								5'd7:data_o = a_0_next[63:56];
								5'd8:data_o = a_0_next[71:64];
								5'd9:data_o = a_0_next[79:72];
								5'd10:data_o = a_0_next[87:80];
								5'd11:data_o = a_0_next[95:88];
								5'd12:data_o = a_0_next[103:96];
								5'd13:data_o = a_0_next[111:104];
								5'd14:data_o = a_0_next[119:112];
								5'd15:data_o = a_0_next[127:120];
								5'd16:data_o = a_0_next[135:128];
								5'd17:data_o = a_0_next[143:136];
								5'd18:data_o = a_0_next[151:144];
								5'd19:data_o = a_0_next[159:152];
								5'd20:data_o = a_0_next[167:160];
								5'd21:data_o = a_0_next[175:168];
								5'd22:data_o = a_0_next[183:176];
								5'd23:data_o = a_0_next[191:184];
								5'd24:data_o = a_0_next[199:192];
								5'd25:data_o = a_0_next[207:200];
								5'd26:data_o = a_0_next[215:208];
								5'd27:data_o = a_0_next[223:216];
								5'd28:data_o = a_0_next[231:224];
								5'd29:data_o = a_0_next[239:232];
								5'd30:data_o = a_0_next[247:240];
								5'd31:data_o = a_0_next[255:248];
								default:data_o   = 8'b0;
							endcase
					2'b01:case(addr)
								5'd0:data_o = a_1_next[7:0];
								5'd1:data_o = a_1_next[15:8];
								5'd2:data_o = a_1_next[23:16];
								5'd3:data_o = a_1_next[31:24];
								5'd4:data_o = a_1_next[39:32];
								5'd5:data_o = a_1_next[47:40];
								5'd6:data_o = a_1_next[55:48];
								5'd7:data_o = a_1_next[63:56];
								5'd8:data_o = a_1_next[71:64];
								5'd9:data_o = a_1_next[79:72];
								5'd10:data_o = a_1_next[87:80];
								5'd11:data_o = a_1_next[95:88];
								5'd12:data_o = a_1_next[103:96];
								5'd13:data_o = a_1_next[111:104];
								5'd14:data_o = a_1_next[119:112];
								5'd15:data_o = a_1_next[127:120];
								5'd16:data_o = a_1_next[135:128];
								5'd17:data_o = a_1_next[143:136];
								5'd18:data_o = a_1_next[151:144];
								5'd19:data_o = a_1_next[159:152];
								5'd20:data_o = a_1_next[167:160];
								5'd21:data_o = a_1_next[175:168];
								5'd22:data_o = a_1_next[183:176];
								5'd23:data_o = a_1_next[191:184];
								5'd24:data_o = a_1_next[199:192];
								5'd25:data_o = a_1_next[207:200];
								5'd26:data_o = a_1_next[215:208];
								5'd27:data_o = a_1_next[223:216];
								5'd28:data_o = a_1_next[231:224];
								5'd29:data_o = a_1_next[239:232];
								5'd30:data_o = a_1_next[247:240];
								5'd31:data_o = a_1_next[255:248];
								default:data_o   = 8'b0;
							endcase
					2'b10:case(addr)
								5'd0:data_o = a_2_next[7:0];
								5'd1:data_o = a_2_next[15:8];
								5'd2:data_o = a_2_next[23:16];
								5'd3:data_o = a_2_next[31:24];
								5'd4:data_o = a_2_next[39:32];
								5'd5:data_o = a_2_next[47:40];
								5'd6:data_o = a_2_next[55:48];
								5'd7:data_o = a_2_next[63:56];
								5'd8:data_o = a_2_next[71:64];
								5'd9:data_o = a_2_next[79:72];
								5'd10:data_o = a_2_next[87:80];
								5'd11:data_o = a_2_next[95:88];
								5'd12:data_o = a_2_next[103:96];
								5'd13:data_o = a_2_next[111:104];
								5'd14:data_o = a_2_next[119:112];
								5'd15:data_o = a_2_next[127:120];
								5'd16:data_o = a_2_next[135:128];
								5'd17:data_o = a_2_next[143:136];
								5'd18:data_o = a_2_next[151:144];
								5'd19:data_o = a_2_next[159:152];
								5'd20:data_o = a_2_next[167:160];
								5'd21:data_o = a_2_next[175:168];
								5'd22:data_o = a_2_next[183:176];
								5'd23:data_o = a_2_next[191:184];
								5'd24:data_o = a_2_next[199:192];
								5'd25:data_o = a_2_next[207:200];
								5'd26:data_o = a_2_next[215:208];
								5'd27:data_o = a_2_next[223:216];
								5'd28:data_o = a_2_next[231:224];
								5'd29:data_o = a_2_next[239:232];
								5'd30:data_o = a_2_next[247:240];
								5'd31:data_o = a_2_next[255:248];
								default:data_o   = 8'b0;
							endcase
					2'b11:case(addr)
								5'd0:data_o = a_3_next[7:0];
								5'd1:data_o = a_3_next[15:8];
								5'd2:data_o = a_3_next[23:16];
								5'd3:data_o = a_3_next[31:24];
								5'd4:data_o = a_3_next[39:32];
								5'd5:data_o = a_3_next[47:40];
								5'd6:data_o = a_3_next[55:48];
								5'd7:data_o = a_3_next[63:56];
								5'd8:data_o = a_3_next[71:64];
								5'd9:data_o = a_3_next[79:72];
								5'd10:data_o = a_3_next[87:80];
								5'd11:data_o = a_3_next[95:88];
								5'd12:data_o = a_3_next[103:96];
								5'd13:data_o = a_3_next[111:104];
								5'd14:data_o = a_3_next[119:112];
								5'd15:data_o = a_3_next[127:120];
								5'd16:data_o = a_3_next[135:128];
								5'd17:data_o = a_3_next[143:136];
								5'd18:data_o = a_3_next[151:144];
								5'd19:data_o = a_3_next[159:152];
								5'd20:data_o = a_3_next[167:160];
								5'd21:data_o = a_3_next[175:168];
								5'd22:data_o = a_3_next[183:176];
								5'd23:data_o = a_3_next[191:184];
								5'd24:data_o = a_3_next[199:192];
								5'd25:data_o = a_3_next[207:200];
								5'd26:data_o = a_3_next[215:208];
								5'd27:data_o = a_3_next[223:216];
								5'd28:data_o = a_3_next[231:224];
								5'd29:data_o = a_3_next[239:232];
								5'd30:data_o = a_3_next[247:240];
								5'd31:data_o = a_3_next[255:248];
								default:data_o   = 8'b0;
							endcase
					default:data_o = 8'b0;
				endcase
			end
			else if(~start)
				ready_next = 1'b1;
			else
				ready_next = ready;
		end
		else if (done) begin
				ready_next = 1'b0;
				a_0_next = a_0_result;
			end
		else
			ready_next = ready;	
	end
	
	
	always@(posedge clk or negedge reset) begin
		if(!reset) begin
			a_0 = 256'b0;
			a_1 = 256'b0;
			a_2 = 256'b0;
			a_3 = 256'b0;
			ready = 1'b0;
		end
		else begin
			a_0 = a_0_next;
			a_1 = a_1_next;
			a_2 = a_2_next;
			a_3 = a_3_next;
			ready = ready_next;
		end
	end
	
endmodule
