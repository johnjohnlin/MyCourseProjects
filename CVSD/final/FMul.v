module FMul(
	input clk,
	input [31:0] in1,
	input [31:0] in2,
	output reg [31:0] out
);
	wire [7:0] e1 = in1[30-:8];
	wire [7:0] e2 = in2[30-:8];
	wire isNormal1 = |e1;
	wire isNormal2 = |e2;
	wire [7:0] power1 = e1 | {7'b0, ~isNormal1};
	wire [7:0] power2 = e2 | {7'b0, ~isNormal2};

	wire [4:0] _n1, n1;
	wire [4:0] _n2, n2;
	wire _z1, z1, _z2, z2;
	ShamtCal sc1(
		.in({1'b0, in1[22:0], 1'b0}),
		.shamt(_n1),
		.zero(_z1)
	);
	ShamtCal sc2(
		.in({1'b0, in2[22:0], 1'b0}),
		.shamt(_n2),
		.zero(_z2)
	);
	assign z1 = _z1&~isNormal1;
	assign z2 = _z2&~isNormal2;
	wire zero = z1 | z2;
	assign n1 = _n1 & {5{~isNormal1}};
	assign n2 = _n2 & {5{~isNormal2}};

	wire [23:0] mulNorm1 = {~zero, in1[22:0] << n1};
	wire [23:0] mulNorm2 = {~zero, in2[22:0] << n2};

	wire [8:0] power_partial = power1 + power2;
	wire osign = in1[31] ^ in2[31];

	/* pipeline 0 */
	reg [23:0] mul1_reg;
	reg [23:0] mul2_reg;
	// reg zero_reg0, zero_reg1;
	reg zero_reg;
	reg osign_reg0, osign_reg1, osign_reg2;
	reg [8:0] power_partial_reg;
	reg [4:0] n1_reg, n2_reg;
	always @(posedge clk) begin
		mul1_reg <= mulNorm1;
		mul2_reg <= mulNorm2;
		zero_reg <= zero;
		//zero_reg0 <= zero;
		// zero_reg1 <= zero_reg0;
		osign_reg0 <= osign;
		osign_reg1 <= osign_reg0;
		osign_reg2 <= osign_reg1;
		power_partial_reg <= power_partial;
		n1_reg <= n1;
		n2_reg <= n2;
	end

	/* pipeline 1 */
	reg [9:0] power_reg;
	wire [9:0] power_tmp = power_partial_reg - n1_reg - n2_reg - 127;
	always @(posedge clk) begin
		power_reg <= power_tmp;
	end

	// HW duplication
	/* 10 bits for negative & infinite overflow detection
	   upper bound: 254+254+1-127 < 512
	   lower bound: 1-24+1-24+0-127 < -256*/
	wire [9:0] powerIf0 = power_reg;
	wire [7:0] shamtIf0__ = 1 - powerIf0;
	wire [4:0] shamtIf0_ = (shamtIf0__ > 31)? 31: shamtIf0__[4:0];
	wire [4:0] shamtIf0 = powerIf0[9]? shamtIf0_: 0;
	wire [7:0] eIf0 = (powerIf0[9] | zero_reg)? 0 : powerIf0[7:0];

	wire [9:0] powerIf1 = power_reg + 1'b1;
	wire [7:0] shamtIf1__ = 1 - powerIf1;
	wire [4:0] shamtIf1_ = (shamtIf1__ > 31)? 31: shamtIf1__[4:0];
	wire [4:0] shamtIf1 = powerIf1[9]? shamtIf1_: 0;
	wire [7:0] eIf1 = (powerIf1[9] | zero_reg)? 0 : powerIf1[7:0];

	/* pipeline 2 */
	/* TODO: these values differs no more than 1
	   shamtIf0 = shamtIf1 of shamtIf1+1
	   eIf1 = eIf0 of eIf0+1
	   This reduces area & time (I think)
	 */
	reg [4:0] shamtIf0_reg;
	reg [4:0] shamtIf1_reg;
	reg [7:0] eIf0_reg;
	reg [7:0] eIf1_reg;
	always @(posedge clk) begin
		shamtIf0_reg <= shamtIf0;
		shamtIf1_reg <= shamtIf1;
		eIf0_reg <= eIf0;
		eIf1_reg <= eIf1;
	end

	wire [47:0] mul;
	DW02_mult_2_stage #(24,24) m0(
		.A(mul1_reg),
		.B(mul2_reg),
		.TC(1'b0),
		.CLK(clk),
		.PRODUCT(mul)
	);

	/* pipeline 1-2 */
	reg [47:22] mul_reg;
	always @(posedge clk) begin
		mul_reg <= mul[47:22];
	end

	wire sl1 = mul_reg[47];
	wire [24:0] value = sl1? mul_reg[47-:25] : mul_reg[46-:25];
	wire [23:0] valueRnd = value[24:1] + value[0];
	wire [23:0] valueRndSh = valueRnd >> (sl1? shamtIf1_reg: shamtIf0_reg);

	/* pipeline 3 */
	always @(posedge clk) begin
		out[31] <= osign_reg2;
		out[30-:8] <= sl1? eIf1_reg: eIf0_reg;
		out[22:0] <= valueRndSh[22:0];
	end

endmodule
