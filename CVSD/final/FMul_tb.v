`timescale 1ns/100ps

module FMUL_tb();
	reg [31:0] f1;
	reg [31:0] f2;
	wire [31:0] out;
	reg clk;

	always @(*) begin
		#1 clk <= ~clk;
	end

	initial begin
		$fsdbDumpfile("FMul.fsdb");
		$fsdbDumpvars();
		#1
		f1 = 32'h41D26666; // 2.63
		f2 = 32'hC5B140F6; // -5672.12
		clk = 0;
		#2
		f1 = 32'h40000000; // 2.0
		f2 = 32'h3fe3d70a; // 1.78
		#2
		f1 = 32'h1C71C901;
		f2 = 32'h9BB556C0;
		#2
		f1 = 32'h1F800000; // 2^(-63)
		f2 = 32'h1F000000; // 2^(-64)
		#2
		f1 = 32'h00700000; // 1.029E-38
		f2 = 32'h40000000; // 2
		#2
		f1 = 32'h00700000; // 1.029E-38
		f2 = 32'h1E3CE508; // 1E-20
		#20
		$finish;
	end

	FMul fmul1(
		.clk(clk),
		.in1(f1),
		.in2(f2),
		.out(out)
	);
endmodule
