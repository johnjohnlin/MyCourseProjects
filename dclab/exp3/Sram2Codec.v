module Sram2Codec(
	dataStream,
	dataR,
	dataW,
	addrIn,
	write,
	read,
	on,
	clk,
	_WE,
	_CE,
	_OE,
	_LB,
	_UB,
	_Addr
);

	inout [15:0] dataStream;
	reg [15:0] dataStream_buf;

	input write, read, on, clk;
	input [15:0] dataW;
	input [17:0] addrIn;

	output reg _WE, _CE, _OE, _LB, _UB;
	output reg [15:0] dataR;
	output [17:0] _Addr;

	assign _Addr = addrIn;
	assign dataStream = write? dataStream_buf: 16'hzzzz;

	always @(*) begin
		dataR = 16'ha00a;
		dataStream_buf = 16'h8001;
		_WE = 1'b1;
		_CE = ~on;
		_OE = 1'b1;
		_LB = 1'b0;
		_UB = 1'b0;

		// Read
		if (read) begin
			_OE = 1'b0;
			dataR = dataStream;
		end

		// Write
		if (write) begin
			_WE = 1'b0;
			dataStream_buf = dataW;
		end
	end
endmodule
