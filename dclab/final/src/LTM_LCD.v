module LTM_LCD(
	// control
	i_clk,
	i_start,
	i_mode,
	i_RGBMode,
	o_done,

	// request for the data
	i_data,
	o_addrX,
	o_addrY,

	// to real hardware
	o_HD,
	o_VD,
	o_DEN,
	o_R,
	o_G,
	o_B
);
	parameter xLine = 1055;
	parameter yLine = 524;
	parameter xMin = 216;
	parameter xMax = 1015;
	parameter yMin = 35;
	parameter yMax = 514;

	input i_clk;
	input i_start;
	// 1 = color, 0 = stroke
	input i_mode;

	// RGB mask
	input [2:0] i_RGBMode;

	// request for the data
	input [7:0] i_data;
	output [9:0] o_addrX;
	output [8:0] o_addrY;

	output [7:0] o_R;
	output [7:0] o_G;
	output [7:0] o_B;
	output o_HD;
	output o_VD;
	output o_DEN;
	output o_done;

	reg [10:0] xCnt, xCnt_next;
	reg  [9:0] yCnt, yCnt_next;
	reg [7:0] o_R;
	reg [7:0] o_G;
	reg [7:0] o_B;
	reg done, done_next;

	wire validData;
	wire negStart;

	NegDetector nd1(.clk(i_clk), .sig(i_start), .out(negStart));

	assign validData = (xCnt >= xMin) & (xCnt <= xMax) & (yCnt >= yMin) & (yCnt <= yMax);
	assign o_DEN = validData;
	assign o_HD = (xCnt != 0) | done;
	assign o_VD = (yCnt != 0) | done;
	// use +1 for prefetch
	assign o_addrX = xCnt_next - xMin;
	assign o_addrY = yMax - yCnt_next;
	assign o_done = done;

	always @(*) begin
		done_next = done;
		// done: normal signal
		// negStart: force reset counter
		if (done | negStart) begin
			xCnt_next <= 11'b0;
			yCnt_next <= 10'b0;
			if (negStart) begin
				done_next = 0;
			end
		end else begin
			if ((xCnt == xLine) & (yCnt == yLine)) begin
				done_next = 1;
			end
			xCnt_next = (xLine == xCnt)? 0: (xCnt + 1);
			yCnt_next = yCnt + ((xLine == xCnt)? 1: 0);
		end
	end

	always @(posedge i_clk) begin
		xCnt <= xCnt_next;
		yCnt <= yCnt_next;
		done <= done_next;

		if (i_mode) begin
			o_R <= {8{i_RGBMode[2]}};
			o_G <= {8{i_RGBMode[1]}};
			o_B <= {8{i_RGBMode[0]}};
		end else begin
			o_R <= i_data;
			o_G <= i_data;
			o_B <= i_data;
		end
	end
endmodule
