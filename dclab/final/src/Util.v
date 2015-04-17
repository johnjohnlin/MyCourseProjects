module NegDetector(
	clk,
	sig,
	out
);
	input clk;
	input sig;
	output out;
	reg prevSig;
	assign out = prevSig & (~sig);

	always @(posedge clk) begin
		prevSig <= sig;
	end
endmodule

module PosDetector(
	clk,
	sig,
	out
);
	input clk;
	input sig;
	output out;
	reg prevSig;
	assign out = (~prevSig) & sig;

	always @(posedge clk) begin
		prevSig <= sig;
	end
endmodule
