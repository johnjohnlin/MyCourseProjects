`timescale 1ns/100ps

module pow_tb();
	parameter nbits = 256;
	reg clk;
	reg start;
	wire [nbits-1:0] a0;
	reg [nbits-1:0] a1;
	reg [nbits-1:0] a2;
	reg [nbits-1:0] a3;
	wire done;

	initial begin
		$dumpfile("testpow.dump");
		$dumpvars(2, pow_tb);
		$dumpvars(2, pow_tb.p1.c1);
		#200000;
		$finish;
	end
	
	initial begin
		clk = 1'b0;
		a1 = 256'hD41B183313D306ADCA09126F3FED6CDEC7DCDCE49DB5C85CB2A37F08C0F2E31;
		a2 = 256'h5972DD91C4AC6E6FCA344AD4C9B586B4805B5C6219B2DEF7CEE09D88F680228D;
		a3 = 256'hE07122F2A4A9E81141ADE518A2CD7574DCB67060B005E24665EF532E0CCA73E1;
		#1
		start = 1'b1;
		#1
		start = 1'b0;
		#2
		start = 1'b1;
		#100000
		a1 = 256'hC60BFA4C927DDF1A4F1E55C3AD10E65858AC609E668C2092E17D90D6AF29B71A;
		start = 1'b0;
		#2
		start = 1'b1;
	end

	always begin
		#1 clk = ~clk;
	end

	Power p1(
		// inputs
		.clk(clk),
		.start(start),
		.a1(a1),
		.a2(a2),
		.a3(a3),
		// outputs
		.done(done),
		// inout
		.a0(a0)
	);
endmodule
