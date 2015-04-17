module Main(
	GPIO_1,
	GPIO_0,
	GPIO_CLKIN_P1,
	GPIO_CLKIN_N1,
	GPIO_CLKOUT_P1,
	GPIO_CLKOUT_N1,
	iCLK_50,
	iKEY,
	oSRAM_ADSC_N,
	oSRAM_ADSP_N,
	oSRAM_ADV_N,
	oSRAM_BE_N,
	oSRAM_CE1_N,
	oSRAM_CE2,
	oSRAM_CE3_N,
	oSRAM_CLK,
	oSRAM_A,
	oSRAM_GW_N,
	oSRAM_OE_N,
	oSRAM_WE_N,
	SRAM_DQ,
	oLEDR
);
	// IO declaraction
	inout [31:0] GPIO_1;
	output [31:0] GPIO_0;
	input GPIO_CLKIN_P1;
	input GPIO_CLKIN_N1;
	output GPIO_CLKOUT_P1;
	output GPIO_CLKOUT_N1;
	input iCLK_50;
	input [3:0] iKEY;
	output oSRAM_ADSC_N;
	output oSRAM_ADSP_N;
	output oSRAM_ADV_N;
	output [3:0]oSRAM_BE_N;
	output oSRAM_CE1_N;
	output oSRAM_CE2;
	output oSRAM_CE3_N;
	output oSRAM_CLK;
	output [18:0] oSRAM_A;
	output oSRAM_GW_N;
	output oSRAM_OE_N;
	output oSRAM_WE_N;
	inout [31:0] SRAM_DQ;
	output [17:0] oLEDR;

	wire   touchtest;

	assign GPIO_0[0] = iCLK_12;
	assign GPIO_0[1] = compStart;
	assign GPIO_0[2] = posTouchSend;
	assign GPIO_0[14:3] = compAddr;
	assign GPIO_0[22] = compRW;
	assign GPIO_0[30:23] = dataFrSRAM;
	assign GPIO_0[31] = compDone;
	assign oLEDR[3:0] = state;
	// For clock divider
	reg [1:0] clkCounter;
	wire iCLK_12;
	assign iCLK_12 = clkCounter[1];
	always @(posedge iCLK_50) begin
		clkCounter <= clkCounter + 1;
	end

	// LTM module
	wire adc_penirq_n;
	wire adc_dout;
	wire adc_busy;
	wire adc_din;
	wire adc_ltm_sclk;
	wire ltm_nclk;
	wire ltm_den;
	wire ltm_hd;
	wire ltm_vd;
	wire [7:0] ltm_r, ltm_g, ltm_b;
	wire ltm_grst;
	wire ltm_scen;
	wire ltm_sda;
	wire posTouchSend, touchSend;

	// Key signal
	wire negResetKey, negLockKey, negSetKey, negDispPWKey;
	NegDetector nd1(.sig(iKEY[0]), .clk(iCLK_12), .out(negResetKey));
	NegDetector nd2(.sig(iKEY[1]), .clk(iCLK_12), .out(negLockKey));
	NegDetector nd3(.sig(iKEY[2]), .clk(iCLK_12), .out(negSetKey));
	NegDetector nd4(.sig(iKEY[3]), .clk(iCLK_12), .out(negDispPWKey));
	PosDetector pd1(.sig(touchSend), .clk(iCLK_12), .out(posTouchSend));

	// Inter-module signal
	
	wire [255:0] str1, str2;
	wire [3:0] state;
	wire [2:0] dispRGB;
	wire [5:0] strokeLeng, leng1, leng2;
	wire [255:0] strokeOut;
	wire dispDone, dispStart, compDone, compStart;
	wire touchWEN, compRW, SRAM_RW;
	wire dispMode;
	wire SCEN_touch, SCEN_disp;
	wire [7:0] compResult, dataToSRAM, dataFrSRAM;
	wire [7:0] touchDataW, compDataW;
	wire [18:0] touchAddr, dispAddr;
	wire [11:0] compAddr;
	wire [20:0] addr20b;

	// bypass some signal
	assign oSRAM_CLK = iCLK_50;

	// rename GPIO to mnemonic name
	assign adc_penirq_n = GPIO_CLKIN_N1;
	assign adc_dout = GPIO_1[0];
	assign adc_busy = GPIO_CLKIN_P1;
	assign GPIO_1[1] = adc_din;
	assign GPIO_1[2] = adc_ltm_sclk;
	assign GPIO_1[3] = ltm_b[3];
	assign GPIO_1[4] = ltm_b[2];
	assign GPIO_1[5] = ltm_b[1];
	assign GPIO_1[6] = ltm_b[0];
	assign GPIO_1[7] = ltm_nclk;
	assign GPIO_1[8] = ltm_den;
	assign GPIO_1[9] = ltm_hd;
	assign GPIO_1[10] = ltm_vd;
	assign GPIO_1[13:11] = ltm_b[6:4];
	assign GPIO_CLKOUT_N1 = ltm_b[7];
	assign GPIO_1[14] = ltm_g[0];
	assign GPIO_CLKOUT_P1 = ltm_g[1];
	assign GPIO_1[20:15] = ltm_g[7:2];
	assign GPIO_1[28:21] = ltm_r;
	assign GPIO_1[29] = ltm_grst;
	assign GPIO_1[30] = ltm_scen;
	assign GPIO_1[31] = ltm_sda;
	// bypass these signals
	assign ltm_nclk = iCLK_12;
	assign ltm_grst = iKEY[0];
	assign ltm_scen = SCEN_disp | SCEN_touch;

	// All modules
	Control c1(
		.i_clk(iCLK_12),
		.i_dispDone(dispDone),
		.i_compDone(compDone),
		.i_compResult(compResult),

		.i_dispAddr(dispAddr),
		.o_dispRGB(dispRGB),
		.o_dispMode(dispMode),
		.o_dispStart(dispStart),

		.i_compDataW(compDataW),
		.i_compAddr(compAddr),
		.i_compRW(compRW),
		.o_compStart(compStart),

		.i_touchDataW(touchDataW),
		.i_touchAddr(touchAddr),
		.i_touchWE(touchWEN),
		.i_newStrokeIRQ(posTouchSend),
		.i_strokeLeng(strokeLeng),
		.i_strokeData(strokeOut),
		.o_stroke1(str1),
		.o_stroke2(str2),
		.o_leng1(leng1),
		.o_leng2(leng2),
		.o_state(state),

		.i_negLockKey(negLockKey),
		.i_negResetKey(negResetKey),
		.i_negDispPWKey(negDispPWKey),
		.i_negSetKey(negSetKey),
		.o_SRAM_A(addr20b),
		.o_SRAM_Dout(dataToSRAM),
		.o_SRAM_RW(SRAM_RW),
		.o_SCEN(SCEN_disp)
	);

	Touch t1(
		.iclk(iCLK_12),
		.iADC_PENIRQ_n(adc_penirq_n),
		.iADC_DOUT(adc_dout),
		.iADC_BUSY(), // bypass, not used
		.oADC_DIN(adc_din),
		.oADC_DCLK(adc_ltm_sclk),
		.oSCEN(SCEN_touch),
		.o_data_len(strokeLeng),
		.o_send(touchSend),
		.o_data(strokeOut),
		.o_address(touchAddr),
		.o_write_en(touchWEN),
		.o_write_data(touchDataW)
	);

	LTM_LCD l1(
		// control
		.i_clk(iCLK_12),
		.i_start(dispStart),
		.i_mode(dispMode),
		.o_done(dispDone),
		.i_RGBMode(dispRGB),
		// request for the data
		.i_data(dataFrSRAM),
		.o_addrX(dispAddr[18:9]),
		.o_addrY(dispAddr[8:0]),
		// to real hardware
		.o_HD(ltm_hd),
		.o_VD(ltm_vd),
		.o_DEN(ltm_den),
		.o_R(ltm_r),
		.o_G(ltm_g),
		.o_B(ltm_b)
	);

	Compare cmp1(
		.i_clk(iCLK_12),
		.i_stroke1(str1),
		.i_stroke2(str2),
		.i_len1(leng1),
		.i_len2(leng2),
		.i_start(compStart),
		.i_dataR(dataFrSRAM),
		.o_rw(compRW),
		.o_addr1(compAddr[5:0]),
		.o_addr2(compAddr[11:6]),
		.o_dataW(compDataW),
		.o_done(compDone),
		.o_score(compResult)
	);

	Sram s1(
		.i_clk(iCLK_12),
		.i_addr(addr20b),
		.i_rw(SRAM_RW), // 1 = read, 0 = read
		.i_data(dataToSRAM),
		.o_data(dataFrSRAM),
		// lines to real hardware
		.io_SRAM_DQ(SRAM_DQ),
		.o_SRAM_A(oSRAM_A),
		.o_SRAM_ADSC_N(oSRAM_ADSC_N),
		.o_SRAM_ADSP_N(oSRAM_ADSP_N),
		.o_SRAM_ADV_N(oSRAM_ADV_N),
		.o_SRAM_BE_N(oSRAM_BE_N),
		.o_SRAM_CE1_N(oSRAM_CE1_N),
		.o_SRAM_CE2(oSRAM_CE2),
		.o_SRAM_CE3_N(oSRAM_CE3_N),
		.o_SRAM_WE_N(oSRAM_WE_N),
		.o_SRAM_GW_N(oSRAM_GW_N),
		.o_SRAM_OE_N(oSRAM_OE_N)
	);

endmodule
