module NegDetector(
	clk,
	sig,
	out
);
	input clk;
	input sig;
	output wire out;
	reg prevSig;
	wire prevSig_next;
	assign prevSig_next = sig;
	assign out = prevSig & (~sig);

	always @(posedge clk) begin
		prevSig <= prevSig_next;
	end
endmodule

module PosDetector(
	clk,
	sig,
	out
);
	input clk;
	input sig;
	output wire out;
	reg prevSig;
	wire prevSig_next;
	assign prevSig_next = sig;
	assign out = (~prevSig) & sig;

	always @(posedge clk) begin
		prevSig <= prevSig_next;
	end
endmodule
