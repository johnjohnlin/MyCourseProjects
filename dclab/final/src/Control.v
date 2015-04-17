module Control(
	// Control signals
	i_clk,
	i_dispDone,
	i_compDone,
	i_compResult,

	// Display request
	i_dispAddr,
	o_dispRGB,
	// 0/1 = stroke/pure color
	o_dispMode,
	o_dispStart,

	// Control data request
	i_compDataW,
	i_compAddr,
	i_compRW,
	o_compStart,

	i_touchDataW,
	i_touchAddr,
	i_touchWE,
	i_newStrokeIRQ,
	i_strokeLeng,
	i_strokeData,
	o_stroke1,
	o_stroke2,
	o_leng1,
	o_leng2,

	i_negLockKey,
	i_negResetKey,
	i_negDispPWKey,
	i_negSetKey,
	o_SRAM_A,
	o_SRAM_Dout,
	o_SRAM_RW,
	o_SCEN,
	o_state
);
	// Control signals
	input i_clk;
	input i_dispDone;
	input i_compDone;
	input [7:0] i_compResult;

	// Display request
	input [18:0] i_dispAddr;
	output [2:0] o_dispRGB;
	// 0/1 = stroke/pure color
	output o_dispMode;
	output o_dispStart;

	// Control data request
	input [7:0] i_compDataW;
	input [11:0] i_compAddr;
	input i_compRW;
	output o_compStart;

	input [7:0] i_touchDataW;
	input [18:0] i_touchAddr;
	input i_touchWE;
	input i_newStrokeIRQ;
	input [5:0] i_strokeLeng;
	output [5:0] o_leng1;
	output [5:0] o_leng2;
	input [255:0] i_strokeData;
	output [255:0] o_stroke1;
	output [255:0] o_stroke2;

	input i_negLockKey;
	input i_negResetKey;
	input i_negDispPWKey;
	input i_negSetKey;
	output [20:0] o_SRAM_A;
	output [7:0] o_SRAM_Dout;
	output o_SRAM_RW;
	output o_SCEN;
	output [3:0] o_state;

// SET_CURR = lock state
`define S_SET_PW 4'ha
`define S_SET_CURR 4'hb

`define S_DISP_WHITE 4'h0
`define S_DISP_N 4'h1
`define S_DISP_Y 4'h2
`define S_DISP_CURR 4'h3
`define S_DISP_PW 4'h4
`define S_DISP_BLACK_PW 4'h7
`define S_DISP_BLACK_CURR 4'h8
`define S_CHECKING 4'h5

	// frame counter bits
	parameter FCB = 5;

	reg [3:0] state, state_next;
	reg [FCB-1:0] counter, counter_next;
	reg checkResult;
	reg [20:0] SRAM_Addr;
	reg [7:0] SRAM_Dout;
	reg SCEN, SRAM_RW;
	reg [255:0] pwStroke, curStroke, pwStroke_next, curStroke_next;
	reg [5:0] pwLeng, curLeng, pwLeng_next, curLeng_next;
	reg [2:0] dispRGB;
	reg dispStart, compStart, dispStart_next, compStart_next;
	reg dispMode;
	wire posCompDone, posDispDone, posNewStroke;
	wire cntTo0;
	wire [FCB-1:0] counterMinus1;

	PosDetector pd1(.clk(i_clk), .out(posDispDone), .sig(i_dispDone));
	PosDetector pd2(.clk(i_clk), .out(posCompDone), .sig(i_compDone));

	assign o_SCEN = SCEN;
	assign o_SRAM_A = SRAM_Addr;
	assign o_SRAM_Dout = SRAM_Dout;
	assign o_SRAM_RW = SRAM_RW;
	assign o_dispRGB = dispRGB;
	assign o_leng1 = pwLeng;
	assign o_leng2 = curLeng;
	assign o_stroke1 = pwStroke;
	assign o_stroke2 = curStroke;
	assign o_dispStart = dispStart;
	assign o_compStart = compStart;
	assign o_dispMode = dispMode;
	assign o_state = state;
	assign counterMinus1 = counter - 1;
	assign cntTo0 = (counter == 0);

	always @(*) begin
		state_next = state;
		counter_next = counter;
		SRAM_Addr = 20'bx;
		SRAM_Dout = 7'bx;
		SRAM_RW = 1'b1;
		SCEN = 1'bx;
		pwLeng_next = pwLeng;
		pwStroke_next = pwStroke;
		curLeng_next = curLeng;
		curStroke_next = curStroke;
		dispStart_next = 1'b1;
		compStart_next = 1'b1;
		dispRGB = 3'bx;
		dispMode = 1'bx;
		case (state)
			`S_SET_PW: begin
				SRAM_Addr = {2'b00, i_touchAddr};
				SRAM_Dout = i_touchDataW;
				SRAM_RW = ~i_touchWE;
				SCEN = 1'b0;
				if (i_newStrokeIRQ) begin
					state_next = `S_DISP_PW;
					counter_next = 15;
					pwStroke_next = i_strokeData;
					pwLeng_next = i_strokeLeng;
					dispStart_next = 1'b0;
				end
			end
			`S_SET_CURR: begin
				SRAM_Addr = {2'b01, i_touchAddr};
				SRAM_Dout = i_touchDataW;
				SRAM_RW = ~i_touchWE;
				SCEN = 1'b0;
				if (i_newStrokeIRQ) begin
					state_next = `S_DISP_CURR;
					counter_next = 15;
					curStroke_next = i_strokeData;
					curLeng_next = i_strokeLeng;
					dispStart_next = 1'b0;
				end
			end
			`S_DISP_PW: begin
				dispMode = 1'b0;
				SRAM_Addr = {2'b00, i_dispAddr};
				SCEN = 1'b1;
				if (posDispDone) begin
					dispStart_next = 1'b0;
					if (cntTo0) begin
						state_next = `S_DISP_WHITE;
					end else begin
						counter_next = counterMinus1;
					end
				end
			end
			`S_DISP_CURR: begin
				dispMode = 1'b0;
				SRAM_Addr = {2'b01, i_dispAddr};
				SCEN = 1'b1;
				if (posDispDone) begin
					if (cntTo0) begin
						state_next = `S_CHECKING;
						compStart_next = 1'b0;
					end else begin
						counter_next = counterMinus1;
						dispStart_next = 1'b0;
					end
				end
			end
			`S_CHECKING: begin
				SRAM_Addr = {2'b10, 7'b0, i_compAddr};
				SRAM_RW = ~i_compRW;
				SRAM_Dout = i_compDataW;
				if (posCompDone) begin
					state_next = (i_compResult <= 8'h8)? `S_DISP_Y: `S_DISP_N;
					dispStart_next = 1'b0;
					counter_next = 15;
				end
			end
			`S_DISP_Y: begin
				dispMode = 1'b1;
				SCEN = 1'b1;
				dispRGB = 3'b010;
				if (posDispDone) begin
					dispStart_next = 1'b0;
					if (cntTo0) begin
						state_next = `S_DISP_WHITE;
					end else begin
						counter_next = counterMinus1;
					end
				end
			end
			`S_DISP_N: begin
				dispMode = 1'b1;
				SCEN = 1'b1;
				dispRGB = 3'b100;
				if (posDispDone) begin
					dispStart_next = 1'b0;
					if (cntTo0) begin
						state_next = `S_DISP_BLACK_CURR;
					end else begin
						counter_next = counterMinus1;
					end
				end
			end
			`S_DISP_WHITE: begin
				dispMode = 1'b1;
				SCEN = 1'b1;
				dispRGB = 3'b111;
				if (i_negSetKey) begin
					state_next = `S_DISP_BLACK_PW;
					dispStart_next = 1'b0;
				end else if (i_negLockKey) begin
					state_next = `S_DISP_BLACK_CURR;
					dispStart_next = 1'b0;
				end else if (i_negDispPWKey) begin
					state_next = `S_DISP_PW;
					counter_next = 15;
					dispStart_next = 1'b0;
				end else if (posDispDone) begin
					dispStart_next = 1'b0;
				end
			end
			`S_DISP_BLACK_CURR: begin
				dispMode = 1'b1;
				SCEN = 1'b1;
				dispRGB = 3'b000;
				if (posDispDone) begin
					state_next = `S_SET_CURR;
				end
			end
			`S_DISP_BLACK_PW: begin
				dispMode = 1'b1;
				SCEN = 1'b1;
				dispRGB = 3'b000;
				if (posDispDone) begin
					state_next = `S_SET_PW;
				end
			end
		endcase
	end

	always @(posedge i_clk) begin
		if (i_negResetKey) begin
			state <= `S_DISP_WHITE;
			dispStart <= 1'b0;
		end else begin
			state <= state_next;
			dispStart <= dispStart_next;
		end
		counter <= counter_next;
		checkResult <= posCompDone? (i_compResult <= 8'h8): checkResult;
		pwStroke <= pwStroke_next;
		pwLeng <= pwLeng_next;
		curStroke <= curStroke_next;
		curLeng <= curLeng_next;
		compStart <= compStart_next;
	end
endmodule
