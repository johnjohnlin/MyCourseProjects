`timescale 1ns/100ps

module mul_tb();
	parameter nbits = 256;
	reg clk;
	reg start;
	reg [nbits-1:0] A;
	reg [nbits-1:0] B;
	reg [nbits-1:0] N;
	wire [nbits-1:0] ret;
	wire done;

	initial begin
		$dumpfile("testmul.dump");
		$dumpvars;
		#1000;
		$finish;
	end
	
	initial begin
		clk = 1'b0;
		A = 256'hc22ca5bf7af57661301eeae991a5e84e3d8055baf1cb59110a33fdd5e15b8ca1;
		B = 256'hc22ca5bf7af57661301eeae991a5e84e3d8055baf1cb59110a33fdd5e15b8ca1;
		N = 256'hE07122F2A4A9E81141ADE518A2CD7574DCB67060B005E24665EF532E0CCA73E1;
		#1
		start = 1'b1;
		#1
		start = 1'b0;
	end

	always begin
		#1 clk = ~clk;
	end

	Multi m1(
		// inputs
		.clk(clk),
		.start(start),
		.x(A),
		.y(B),
		.n(N),
		// outputs
		.done(done),
		.out(ret)
	);
endmodule
