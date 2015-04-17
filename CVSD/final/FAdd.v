
module FAdd(
	input clk,
	input [31:0] in1,
	input [31:0] in2,
	input op, // op = 0 is plus
	output reg [31:0] out
);

	wire larger1e = in1[30-:8] > in2[30-:8];
	wire Eequal = in1[30-:8] == in2[30-:8];
	wire larger1m = in1[22:0] > in2[22:0];
	wire larger1 = larger1e | (Eequal & larger1m);
	wire minus = op ^ in1[31] ^ in2[31];
	wire osign = larger1? in1[31]: (op ^ in2[31]);

	wire [22:0] mLarge = larger1? in1[22:0]: in2[22:0];
	wire [22:0] mSmall = larger1? in2[22:0]: in1[22:0];
	wire [7:0] eLarge = larger1? in1[30-:8]: in2[30-:8];
	wire [7:0] eSmall = larger1? in2[30-:8]: in1[30-:8];
	wire isNormalLarge = |eLarge;
	wire isNormalSmall = |eSmall;
	wire [23:0] mLarge_ext = {isNormalLarge, mLarge[22:0]};
	wire [23:0] mSmall_ext = {isNormalSmall, mSmall[22:0]};
	wire [7:0] de = eLarge - eSmall;

	/* pipeline */
	reg [7:0] de_reg;
	reg [23:0] mSmall_reg;
	reg [23:0] mLarge_reg_0;
	reg minus_reg, osign_reg_0;
	reg [7:0] eLarge_reg_0;
    reg isNormalLarge_reg_0;
	always @(posedge clk) begin
		de_reg <= de;
		mSmall_reg <= mSmall_ext;
		mLarge_reg_0 <= mLarge_ext;
		minus_reg <= minus;
		osign_reg_0 <= osign;
		eLarge_reg_0 <= eLarge;
        isNormalLarge_reg_0 <= isNormalLarge;
	end

	wire [4:0] rshamt = (de_reg > 24)? 24: de_reg;
	wire signed [25:0] mSmallAlign_sign = {1'b0, mSmall_reg, 1'b0} ^ {26{minus_reg}};
	wire [25:0] mSmallAlign = mSmallAlign_sign >>> rshamt;

	/* pipeline */
	reg [25:0] mSmallAlign_reg;
	reg [23:0] mLarge_reg_1;
	reg osign_reg_1;
	reg [7:0] eLarge_reg_1;
    reg isNormalLarge_reg_1;
	always @(posedge clk) begin
		mSmallAlign_reg <= mSmallAlign;
		mLarge_reg_1 <= mLarge_reg_0;
		osign_reg_1 <= osign_reg_0;
		eLarge_reg_1 <= eLarge_reg_0;
        isNormalLarge_reg_1 <= isNormalLarge_reg_0;
	end

	wire [24:0] sum = mLarge_reg_1 + mSmallAlign_reg[25:1] + mSmallAlign_reg[0];
	wire [4:0] n;
	wire zero;
	ShamtCal sc(
		.in(sum),
		.shamt(n),
		.zero(zero)
	);

	/* pipeline */
	reg [7:0] eLarge_reg_2;
	reg [4:0] n_reg;
	reg [24:0] sum_reg;
	reg osign_reg_2;
    reg isNormalLarge_reg_2;
	reg zero_reg;
	always @(posedge clk) begin
		eLarge_reg_2 <= eLarge_reg_1;
		n_reg <= n;
		sum_reg <= sum;
		osign_reg_2 <= osign_reg_1;
        isNormalLarge_reg_2 <= isNormalLarge_reg_1;
		zero_reg <= zero;
	end

	wire [7:0] power = eLarge_reg_2 | {7'b0, ~isNormalLarge_reg_2};
	wire pl = power >= n_reg;
	// Hardware duplication to reduce critical path
	wire [23:0] sum_n = sum_reg[23:0] << n_reg;
	wire [23:0] sum_p = sum_reg[23:0] << power[4:0];
	wire [23:0] denormalSum = pl? sum_n: sum_p;

	/* pipeline */
	always @(posedge clk) begin
		out[30-:8] <= (pl&~zero_reg)? (isNormalLarge_reg_2? (eLarge_reg_2-n_reg+1): 1): 0;
		out[22:0] <= denormalSum[23:1];
		out[31] <= osign_reg_2;
	end
endmodule
