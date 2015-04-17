`timescale 1ns/100ps
`include "../Compare.v"
`include "../Math.v"
`include "../Util.v"

module Compare_tb(
);
	reg clk;
	reg start;
	reg [7:0] sram [1023:0];
	reg [7:0] dataFrSRAM;
	wire [7:0] dataToSRAM;
	wire [9:0] addr;
	wire rw;
	wire done;
	wire [7:0] score;
	wire [7:0] sram00, sram01, sram02, sram10, sram11, sram12;
	wire [7:0] sram20, sram21, sram22, sram30, sram31, sram32;
	wire [7:0] sram40, sram41, sram42;
	assign sram00 = sram[0];
	assign sram01 = sram[1];
	assign sram02 = sram[2];
	assign sram10 = sram[32];
	assign sram11 = sram[33];
	assign sram12 = sram[34];
	assign sram20 = sram[64];
	assign sram21 = sram[65];
	assign sram22 = sram[66];
	assign sram30 = sram[96];
	assign sram31 = sram[97];
	assign sram32 = sram[98];
	assign sram40 = sram[128];
	assign sram41 = sram[129];
	assign sram42 = sram[130];
	integer i;

	initial begin
		$dumpfile("Compare_tb.vcd");
		$dumpvars;
		for (i = 0; i < 1023; i = i+1) begin
			sram[i] = 0;
		end
		c1.state = 1'b1;
		clk = 1'b1;
		start = 1'b1;
		#3
		start = 1'b0;
		#3
		start = 1'b1;
		#300
		$finish;
	end

	always #1 begin
		clk = ~clk;
	end

	always @(*) begin
		if (rw) begin
			// 1, write
			sram[addr] = dataToSRAM;
		end else begin
			// 0 , read
			dataFrSRAM = sram[addr];
		end
	end

	Compare c1(
		.i_clk(clk),
		.i_stroke1(128'hffffffffffffffffffffffffffffffff),
		.i_stroke2(128'hfffffffffffffffffffffffffffffff1),
		.i_len1(3),
		.i_len2(5),
		.i_start(start),
		.i_dataR(dataFrSRAM),
		.o_rw(rw),
		.o_addr1(addr[4:0]),
		.o_addr2(addr[9:5]),
		.o_dataW(dataToSRAM),
		.o_done(done),
		.o_score(score)
	);
endmodule
