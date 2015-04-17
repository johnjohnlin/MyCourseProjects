`timescale 1ns/100ps

module Clock_tb(
);
	wire clkOut;
	reg clk;
	reg [2:0] ratio;
	reg isNormalSpeed;
	reg interp;
	reg pause;
	reg isRecord;

	initial begin
		$dumpfile("testClock.vcd");
		$dumpvars;
		clk = 1'h0;
		ratio = 3'h7;
		isNormalSpeed = 1'h0;
		interp = 1'h1;
		pause = 1'h0;
		isRecord = 1'b0;
		c1.counter = 2'h0;
		c1.counter2 = 3'h0;
		c1.slowClock = 1'h0;
		#100
		$finish;
	end

	always begin
		#1;
		clk = ~clk;
	end

	Clock c1(
		.clk50(clk),
		.ratio(ratio),
		.isNormalSpeed(isNormalSpeed),
		.interp(interp),
		.pause(pause),
		.isRecord(isRecord),
		.clkOut(clkOut)
	);
endmodule
