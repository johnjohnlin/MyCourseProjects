`timescale 1ns/100ps

module FADD_tb();
	reg [31:0] f1;
	reg [31:0] f2;
	reg oper;
	wire [31:0] out;
	reg clk;

	always @(*) begin
		#1 clk <= ~clk;
	end

	initial begin
		$fsdbDumpfile("FAdd.fsdb");
		$fsdbDumpvars();
		#1
		f1 = 32'h41D26666; // 2.63
		f2 = 32'hC5B140F6; // -5672.12
		oper = 0; // plus
		clk = 0;
		#2
		oper = 1; // minus
		#2
		f1 = 32'h40000000; // 2.0
		f2 = 32'h3fe3d70a; // 1.78
		oper = 0; // plus
		#2
		oper = 1; // minus
		#2
		f1 = 32'h00700000; // 1.029E-38
		f2 = 32'h1E3CE508; // 1E-20
		oper = 0; // plus
		#2
		oper = 1; // minus
		#2
		f1 = 32'h00700000; // 1.029E-38
		f2 = 32'h00111111; // 1.567E-39
		oper = 0; // plus
		#2
		oper = 1; // minus
		#2
		f1 = 32'h00700000; // 1.029E-38
		f2 = 32'h00700000; // 1.029E-38
		oper = 0; // plus
		#2
		oper = 1; // minus
		#2
		f1 = 32'h41D26666; // 2.63
		f2 = 32'hC1D26666; // -2.63
		oper = 0; // plus
		#2
		oper = 1; // minus
		#2
		f1 = 32'h80400000;
		f2 = 32'h80200000;
		oper = 0; // plus
		#2
		oper = 1; // minus
		#2
		f1 = 32'hx;
		f2 = 32'hx;
		#20
		$finish;
	end

	FAdd fadd1(
		.clk(clk),
		.in1(f1),
		.in2(f2),
		.op(oper),
		.out(out)
	);
endmodule
