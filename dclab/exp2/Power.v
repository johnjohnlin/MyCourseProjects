`define nbits 256

module Control_pow256(
	// input
	clk,
	start,
	a1,
	a2,
	m512_done,
	m512_ret,
	m1_done,
	m1_ret,
	m2_done,
	m2_ret,
	// output
	a0,
	m512_start,
	//m512_input,
	m1_input,
	m1_start,
	m2_input,
	m2_start,
	done
);
	input               clk;
	input               start;
	input [`nbits-1:0]  a1;
	input [`nbits-1:0]  a2;
	input               m512_done;
	input [`nbits-1:0]  m512_ret;
	input               m1_done;
	input [`nbits-1:0]  m1_ret;
	input               m2_done;
	input [`nbits-1:0]  m2_ret;
	output [`nbits-1:0] a0;
	//output [`nbits-1:0] m512_input;
	output              m512_start;
	output [`nbits-1:0] m1_input;
	output              m1_start;
	output [`nbits-1:0] m2_input;
	output              m2_start;
	output              done;

	// Util modules
	wire negStart;
	wire posM512_done;
	wire posM1_done;
	wire posM2_done;
	NegDetector nd1(
		.clk(clk),
		.sig(start),
		.out(negStart)
	);
	PosDetector pd1(
		.clk(clk),
		.sig(m512_done),
		.out(posM512_done)
	);
	PosDetector pd2(
		.clk(clk),
		.sig(m1_done),
		.out(posM1_done)
	);
	PosDetector pd3(
		.clk(clk),
		.sig(m2_done),
		.out(posM2_done)
	);
`define S_INIT   2'h0
`define S_DO_256 2'h1
`define S_DO_MUL 2'h2
`define S_DONE   2'h3
	reg [1:0] state, state_next;
	reg [`nbits-1:0] a2Buf, a2Buf_next;
	// this needs buffer
	reg [`nbits-1:0] x2Buf, x2Buf_next;

	assign done = (state == `S_DONE);
	assign m512_start = (state == `S_INIT);
	assign m1_start = ((state_next != `S_DO_MUL) | ~posM2_done | ~a2Buf_next[0]) & ~posM512_done;
	assign m2_start = ((state_next != `S_DO_MUL) | ~posM2_done ) & ~posM512_done;
	assign a0 = (posM512_done? `nbits'b1: m1_ret);
	// forward sending...
	assign m1_input = x2Buf_next;
	assign m2_input = x2Buf_next;

	always @(*) begin
		a2Buf_next = a2Buf;
		x2Buf_next = x2Buf;
		case (state)
			`S_INIT: begin
				state_next = `S_DO_256;
				a2Buf_next = a2;
			end
			`S_DO_256: begin
				if (posM512_done) begin
					state_next = `S_DO_MUL;
					x2Buf_next = m512_ret;
				end else begin
					state_next = `S_DO_256;
				end
			end
			`S_DO_MUL: begin
				if (~|a2Buf) begin
					state_next = `S_DONE;
				end else begin
					state_next = `S_DO_MUL;
				end

				if (posM2_done) begin
					a2Buf_next = a2Buf >> 1;
					x2Buf_next = m2_ret;
				end
			end
			`S_DONE: begin
				state_next = `S_DONE;
			end
		endcase
	end

	always @(posedge clk) begin
		state <= (negStart? `S_INIT: state_next);
		a2Buf <= a2Buf_next;
		x2Buf <= x2Buf_next;
	end
`undef S_INIT
`undef S_DO_256
`undef S_DO_MUL
`undef S_DONE
endmodule

module Cal_x_m2_256(
	// input
	clk,
	start,
	a1,
	a3,
	// output
	x_m2_256,
	done
);
	input wire clk;
	input wire start;
	input wire [`nbits-1:0] a3;
	input wire [`nbits-1:0] a1;
	output reg done;
	reg done_next;
	output reg [`nbits-1:0] x_m2_256;
	reg [`nbits:0] x_m2_256_next;
`define counter_bits 8
	reg [`counter_bits-1:0] counter, counter_next;
	wire startCalc;
	NegDetector nd1(
		.clk(clk),
		.sig(start),
		.out(startCalc)
	);
	
	always @(*) begin
		if (startCalc) begin
			counter_next = `counter_bits'b0;
		end else begin
			counter_next = counter + {{(`counter_bits-1){1'b0}}, ~(&counter)};
		end
	end

	reg [`nbits:0] x_m2_256_next_tmp0;
	reg [`nbits:0] x_m2_256_next_tmp1;
	always @(*) begin
		if (startCalc) begin
			x_m2_256_next_tmp0 = {1'b0, a1};
		end else begin
			x_m2_256_next_tmp0 = {x_m2_256, 1'b0};
		end

		if (x_m2_256_next_tmp0 > {1'b0, a3}) begin
			x_m2_256_next_tmp1 = x_m2_256_next_tmp0 - {1'b0, a3};
		end else begin
			x_m2_256_next_tmp1 = x_m2_256_next_tmp0;
		end

		if ((&counter_next) & done) begin
			x_m2_256_next = x_m2_256;
		end else begin
			x_m2_256_next = x_m2_256_next_tmp1;
		end
		done_next = &counter;
	end

	always @(posedge clk) begin
		x_m2_256 <= x_m2_256_next[`nbits-1:0];
		counter <= counter_next;
		done <= done_next;
	end
`undef counter_bits
endmodule

module Power(
	// inputs
	clk,
	start,
	a1,
	a2,
	a3,
	// outputs
	done,
	a0
);
	input clk;
	input start;
	input [`nbits-1:0] a1;
	input [`nbits-1:0] a2;
	input [`nbits-1:0] a3;
	output done;
	output [`nbits-1:0] a0;

	wire              m512_done;
	wire [`nbits-1:0] m512_ret;
	wire              m512_start;
	wire              m1_done;
	wire [`nbits-1:0] m1_ret;
	wire [`nbits-1:0] m1_input;
	wire              m1_start;
	wire              m2_done;
	wire [`nbits-1:0] m2_ret;
	wire [`nbits-1:0] m2_input;
	wire              m2_start;

	Control_pow256 cp1(
		// input
		.clk(clk),
		.start(start),
		.a1(a1),
		.a2(a2),
		.m512_done(m512_done),
		.m512_ret(m512_ret),
		.m1_done(m1_done),
		.m1_ret(m1_ret),
		.m2_done(m2_done),
		.m2_ret(m2_ret),
		// output
		.a0(a0),
		.m512_start(m512_start),
		.m1_input(m1_input),
		.m1_start(m1_start),
		.m2_input(m2_input),
		.m2_start(m2_start),
		.done(done)
	);
	Cal_x_m2_256 c1(
		.clk(clk),
		.start(m512_start),
		.a1(a1),
		.a3(a3),
		.x_m2_256(m512_ret),
		.done(m512_done)
	);
	// for a0
	Multi m1(
		.clk(clk),
		.start(m1_start),
		.done(m1_done),
		.x(a0),
		.y(m1_input),
		.n(a3),
		.out(m1_ret)
	);
	// for x2
	Multi m2(
		.clk(clk),
		.start(m2_start),
		.done(m2_done),
		.x(m2_input),
		.y(m2_input),
		.n(a3),
		.out(m2_ret)
	);
endmodule
