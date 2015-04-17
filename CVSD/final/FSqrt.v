module FSqrt(
	input [31:0] in,
	output [31:0] out,
	input clk
);
`define SQRT2 28'hB504F33
`define   ONE 28'h8000000
	wire [7:0] ein = in[30-:8];
	wire [22:0] min = in[22:0];
	wire isNormal = |ein;

	wire [4:0] _n;
	wire mZero;
	ShamtCal sh(
		.in({1'b0, min, 1'b0}),
		.shamt(_n),
		.zero(mZero)
	);
	wire zero = mZero & ~isNormal;
	wire [4:0] n = _n & {5{~isNormal}};
	wire [23:0] init = {~zero, min << n};
	// out expbit = (e + isNormal + 127)/2
	wire [8:0] powerPlus127 = (ein | {7'b0, ~isNormal}) - n + 8'd127;
	wire [7:0] eOut = powerPlus127[8:1];
	wire mulSqrt2 = powerPlus127[0];


	wire [7:0] lut_in = init[22-:8];
	wire [7:0] x;
	FSqrt_LUT lut(
		.in(lut_in),
		.out(x)
	);

	// TODO's: shift multiply is not what it is in C/C++
	wire [4+27:0] r1 = (init * x) >> 4;
	wire [10+25:0] r2 = (r1 * x) >> 10;
	wire [19:0] ep1 = r1[19:0] - r2[19:0] - (init[15:0]<<4);
	wire [27+12:0] ep1_sq = (ep1 * ep1) >> 27;
	wire [27+ 6:0] ep1_cu = (ep1_sq * ep1) >> 27;
	wire [14:0] _ep2 = 3*ep1_sq - ep1_cu;
	wire [12:0] ep2 = _ep2 >> 2;


	wire [26:0] d1 = r1[27:1];
	wire [27:0] _step1 = (init<<4) - d1;
	wire [27+27:0] step1 = (_step1 * (mulSqrt2? `SQRT2: `ONE)) >> 27;
	wire [28+18:0] d2 = (step1*ep1) >> 28;
	wire [27:0] step2 = step1 + d2;
	wire [28+12:0] d3 = (step2*ep2) >> 28;
	wire [27:0] step3 = step2 + d3;
	
	wire [27:0] result = step3;
	assign out[22:0] = result[26:4] + result[3];
	assign out[31] = 1'b0;
	assign out[30-:8] = zero? 8'b0: eOut;
endmodule
