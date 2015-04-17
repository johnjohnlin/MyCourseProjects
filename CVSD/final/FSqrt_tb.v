`timescale 1ns/100ps

module FMUL_tb();
	reg [31:0] in;
	wire [31:0] out;
	reg clk;

	always @(*) begin
		#1 clk <= ~clk;
	end

	initial begin
		$fsdbDumpfile("FSqrt.fsdb");
		$fsdbDumpvars();
		#1
		in = 32'h41D26666; // 2.63
		clk = 0;
		#2
		in = 32'h40000000; // 2.0
		#2
		in = 32'h3fe3d70a; // 1.78
		#2
		in = 32'h1C71C901;
		#2
		in = 32'h9BB556C0;
		#2
		in = 32'h1F800000; // 2^(-63)
		#2
		in = 32'h1F000000; // 2^(-64)
		#2
		in = 32'h00700000; // 1.029E-38
		#2
		in = 32'h00700000; // 1.029E-38
		#2
		in = 32'h1E3CE508; // 1E-20
		#20
		$finish;
	end

	FSqrt FSq(
		.in(in),
		.out(out),
		.clk(clk)
	);
endmodule
