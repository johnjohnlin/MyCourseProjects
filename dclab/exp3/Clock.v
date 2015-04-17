module Clock(
	CLK50,
	ratio_m1, //ratio minus 1
	isNormalSpeed,
	isSlow,
	interp,
	pause,
	isRecord,
	clkOut
);
	input CLK50;
	input [2:0] ratio_m1;
	input isNormalSpeed;
	input interp;
	input isRecord;
	input isSlow;
	input pause;
	output clkOut;

	reg [1:0] counter;
	reg [2:0] counter2, counter2_next;
	reg slowClock, slowClock_next;
	reg clkOut_tmp;

	always @(posedge CLK50) begin
		counter <= counter + 2'b1;
		if (isSlow & ~isNormalSpeed & ~isRecord & interp) begin
			clkOut_tmp = slowClock;
		end else begin
			clkOut_tmp = counter[1];
		end
	end
	assign clkOut = clkOut_tmp | pause;

	always @(*) begin
		if (counter2 == ratio_m1) begin
			counter2_next = 3'b0;
			slowClock_next = ~slowClock;
		end else begin
			counter2_next = counter2 + 3'b1;
			slowClock_next = slowClock;
		end		
	end

	always @(posedge counter[0]) begin
		counter2 <= counter2_next;
		slowClock <= slowClock_next;
	end
endmodule
