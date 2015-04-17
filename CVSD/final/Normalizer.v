module ShamtCal(
	input [24:0] in,
	output reg [4:0] shamt,
	output reg zero
);
	always @(*) begin
		zero = 0;
		casex (in) //synopsys full_case
			25'b0000000000000000000000000: begin shamt = 24; zero = 1; end
			25'b000000000000000000000001x: shamt = 23;
			25'b00000000000000000000001xx: shamt = 22;
			25'b0000000000000000000001xxx: shamt = 21;
			25'b000000000000000000001xxxx: shamt = 20;
			25'b00000000000000000001xxxxx: shamt = 19;
			25'b0000000000000000001xxxxxx: shamt = 18;
			25'b000000000000000001xxxxxxx: shamt = 17;
			25'b00000000000000001xxxxxxxx: shamt = 16;
			25'b0000000000000001xxxxxxxxx: shamt = 15;
			25'b000000000000001xxxxxxxxxx: shamt = 14;
			25'b00000000000001xxxxxxxxxxx: shamt = 13;
			25'b0000000000001xxxxxxxxxxxx: shamt = 12;
			25'b000000000001xxxxxxxxxxxxx: shamt = 11;
			25'b00000000001xxxxxxxxxxxxxx: shamt = 10;
			25'b0000000001xxxxxxxxxxxxxxx: shamt =  9;
			25'b000000001xxxxxxxxxxxxxxxx: shamt =  8;
			25'b00000001xxxxxxxxxxxxxxxxx: shamt =  7;
			25'b0000001xxxxxxxxxxxxxxxxxx: shamt =  6;
			25'b000001xxxxxxxxxxxxxxxxxxx: shamt =  5;
			25'b00001xxxxxxxxxxxxxxxxxxxx: shamt =  4;
			25'b0001xxxxxxxxxxxxxxxxxxxxx: shamt =  3;
			25'b001xxxxxxxxxxxxxxxxxxxxxx: shamt =  2;
			25'b01xxxxxxxxxxxxxxxxxxxxxxx: shamt =  1;
			25'b1xxxxxxxxxxxxxxxxxxxxxxxx: shamt =  0;
		endcase
	end
endmodule
