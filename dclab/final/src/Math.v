module Atan2(
	i_xSign,
	i_ySign,
	i_x,
	i_y,
	o_theta
);
	parameter BITS = 8;
	input i_xSign;
	input i_ySign;
	input [BITS-1:0] i_x;
	input [BITS-1:0] i_y;
	output [3:0] o_theta;

	wire swapXY;
	wire [BITS-1:0] x;
	wire [BITS-1:0] y;
	wire [BITS+1:0] x2;
	wire [BITS+1:0] y2;
	wire [BITS+2:0] x5;
	wire [BITS+2:0] y5;
	wire cmp1;
	wire cmp2;
	wire cmp3;

	assign swapXY = i_xSign ^ i_ySign;
	assign x = swapXY? i_y: i_x;
	assign y = swapXY? i_x: i_y;
	assign x2 = x*2;
	assign y2 = y*2;
	assign x5 = x*5;
	assign y5 = y*5;
	assign cmp1 = y2 > x5;
	assign cmp2 = y  > x ;
	assign cmp3 = y5 > x2;

	assign o_theta[3] = i_ySign;
	assign o_theta[2] = swapXY;
	assign o_theta[1:0] = cmp1 + cmp2 + cmp3;
endmodule

module Diff(
	i_t1,
	i_t2,
	o_sign,
	o_diff
);
	parameter BITS = 4;
	input [BITS-1:0] i_t1;
	input [BITS-1:0] i_t2;
	output [BITS-1:0] o_diff;
	output o_sign;

	assign o_sign = i_t1 < i_t2;
	assign o_diff = o_sign? (i_t2-i_t1): (i_t1-i_t2);
endmodule

module DiffTheta(
	i_t1,
	i_t2,
	o_diff
);
	// x = abs(t1-t2)
	// x = (x<(1<<(BITS-1))? (1<<BITS)-x: x);
	// return (x==1? 0: x)
	parameter BITS = 4;
	input [BITS-1:0] i_t1;
	input [BITS-1:0] i_t2;
	output [BITS-1:0] o_diff;
	wire [BITS-1:0] tmp1, tmp2;
	assign tmp1 = i_t1>i_t2? i_t1-i_t2: i_t2-i_t1;
	assign tmp2 = tmp1[BITS-1]? ~tmp1+1: tmp1;
	assign o_diff = (tmp2==1)? 0: tmp2;
endmodule
