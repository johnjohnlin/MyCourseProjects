`timescale 1ns/100ps

module i2c_tb(
);
	reg clk, reset;
	wire I2C_DATA, I2C_CLK;

	initial begin
		$dumpfile("testI2c.tb");
		$dumpvars;
		reset = 1'b1;
		clk = 1'b0;
		#10;
		reset = 1'b0;
		#10;
		reset = 1'b1;
		#1000
		$finish;
	end

	always begin
		#1 clk = ~clk;
	end

	i2c i2(
		.I2C_SCLK(I2C_CLK),
		.I2C_SDAT(I2C_DATA),
		.clk(clk),
		.reset(reset)
	);
endmodule
