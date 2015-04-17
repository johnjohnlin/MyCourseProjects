`timescale 1ns/100ps

module m256_tb();
	parameter nbits = 256;
	reg clk;
	reg start;
	reg [nbits-1:0] a1;
	reg [nbits-1:0] a3;
	wire [nbits-1:0] ret;
	wire done;

	initial begin
		$dumpfile("testm256.dump");
		$dumpvars(0, m256_tb);
		#1000;
		$finish;
	end

	initial begin
		clk = 1'b0;
		a1 = 256'd192304;
		a3 = 256'hE07122F2A4A9E81141ADE518A2CD7574DCB67060B005E24665EF532E0CCA73E1;
		#1
		start = 1'b1;
		#1
		start = 1'b0;
	end

	always begin
		#1 clk = ~clk;
	end

	Cal_x_m2_256 c1(
		// input
		.clk(clk),
		.start(start),
		.a1(a1),
		.a3(a3),
		// output
		.x_m2_256(ret),
		.done(done)
	);

endmodule
