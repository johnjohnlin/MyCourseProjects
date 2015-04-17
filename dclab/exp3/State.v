module State(
	CLK50,
	KEY,
	state
);
	input CLK50;
	input KEY;
	output reg state;
	reg [2:0] counter;

	always @(posedge CLK50) begin
		if (counter == 3'h6) begin
			state = ~state;
		end else begin
			state = state;
		end

		if (KEY) begin
			counter = 3'h0;
		end else begin
			counter = counter + (&counter? 3'h0: 3'h1);
		end
	end
endmodule

module Debounce(
	CLK50,
	KEY,
	negEdge
);
	input CLK50;
	input KEY;
	output reg negEdge;
	reg [3:0] counter;

	always @(posedge CLK50) begin
		if (counter == 4'hc | counter == 4'hd | counter == 4'he) begin
			negEdge = 1'b0;
		end else begin
			negEdge = 1'b1;
		end

		if (KEY) begin
			counter = 4'h0;
		end else begin
			counter = counter + (&counter? 4'h0: 4'h1);
		end
	end
endmodule