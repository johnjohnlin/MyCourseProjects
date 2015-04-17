module Multi(
	x,
	y,
	n,
	clk,
	start,
	out,
	done
);
`define nbits 256
	// inputs
	input [`nbits-1:0] x, y, n;
	input clk;
	input start;
	// out and done is a reg, so use next state
	// out is accumulator alse XD
	output reg [`nbits-1:0] out;
	reg [`nbits-1:0] out_next;
	output reg done;
	reg done_next;
	reg [`nbits-1:0] xBuf, xBuf_next;
	reg [6:0] counter, counter_next;
	wire negStart;
	NegDetector nd1(
		.clk(clk),
		.sig(start),
		.out(negStart)
	);

	// middle vars.
	reg [`nbits:0] loop0Buf;
	reg [`nbits-1:0] loop0Accum;
	reg [`nbits:0] loop1Buf;
	reg [`nbits-1:0] loop1Accum;

	// Combinational Part
	always @(*) begin
		if (negStart) begin
			xBuf_next = x;
			counter_next = 1'b0;
			done_next = 1'b0;
		end else begin
			xBuf_next = {2'b0, xBuf[`nbits-1:2]};
			counter_next = counter + {6'b0, ~(&counter)};
			done_next = (&counter);
		end

		loop0Buf = {(`nbits+1){1'b0}};
		loop1Buf = {(`nbits+1){1'b0}};
		loop0Accum = `nbits'b0;
		loop1Accum = `nbits'b0;
		if (negStart) begin
			out_next = `nbits'b0;
		end else begin
			// loop unroll, loop0
			loop0Buf = out + (xBuf[0]? y: 256'b0);
			if (loop0Buf[0]) begin
				if (loop0Buf > n) begin
					loop0Accum = loop0Buf[`nbits:1] - {1'b0, n[`nbits-1:1]};
				end else begin
					loop0Accum = loop0Buf[`nbits:1] + {1'b0, n[`nbits-1:1]} + `nbits'b1;
				end
			end else begin
				loop0Accum = loop0Buf[`nbits:1];
			end
			// loop unroll, loop1
			loop1Buf = loop0Accum + (xBuf[1]? y: 256'b0);
			if (loop1Buf[0]) begin
				if (loop1Buf > n) begin
					loop1Accum = loop1Buf[`nbits:1] - {1'b0, n[`nbits-1:1]};
				end else begin
					loop1Accum = loop1Buf[`nbits:1] + {1'b0, n[`nbits-1:1]} + `nbits'b1;
				end
			end else begin
				loop1Accum = loop1Buf[`nbits:1];
			end

			if (done) begin
				out_next = out;
			end else begin
				out_next = loop1Accum;
			end
		end
	end

	// Sequential Part
	always @(posedge clk) begin
		xBuf <= xBuf_next;
		counter <= counter_next;
		done <= done_next;
		out <= out_next;
	end
endmodule
