module Main(
	CLK50,
	PAUSE_KEY,
	STOP_KEY,
	RATIO_SW,
	RECORD_SW,
	INTERP_SW,
	NORMAL_SPEED_SW,
	IS_SLOW_SW,
	RESET_KEY,

	AUD_ADCLRCK,
	AUD_DACLRCK,
	AUD_ADCDAT,
	AUD_DACDAT,
	AUD_BCLK,
	AUD_XCK,
	GPIO,
	I2C_SCLK,
	I2C_SDAT,

	SRAM_ADDR,
	SRAM_WE,
	SRAM_OE,
	SRAM_UB,
	SRAM_LB,
	SRAM_CE,
	SRAM_DQ,
	// seven seg.
	HEX0,
	HEX1,
	HEX2,
	HEX3,
	HEX4,
	HEX5,
	HEX6,
	HEX7,
	stop,
	DEBUG_SW
);
	input CLK50;
	input PAUSE_KEY;
	input STOP_KEY;
	input [2:0] RATIO_SW;
	input RECORD_SW;
	input INTERP_SW;
	input IS_SLOW_SW;
	input NORMAL_SPEED_SW;
	input RESET_KEY;
	input AUD_ADCLRCK;
	input AUD_ADCDAT;
	input AUD_DACLRCK;
	input AUD_BCLK;
	input DEBUG_SW;

	output AUD_XCK;
	output [35:0] GPIO;
	output I2C_SCLK;
	output I2C_SDAT;
	output AUD_DACDAT;
	output [17:0] SRAM_ADDR;
	output SRAM_WE;
	output SRAM_OE;
	output SRAM_UB;
	output SRAM_LB;
	output SRAM_CE;
	output [6:0] HEX0, HEX1, HEX2, HEX3, HEX4, HEX5, HEX6, HEX7;
	output stop;

	inout [15:0] SRAM_DQ;

	wire pause, stop, reset;
	wire [17:0] address;
	wire [3:0] ratio = (NORMAL_SPEED_SW? 3'h1: RATIO_SW + 3'h1);
	reg [5:0] counter;
	wire CLK_400K = counter[5];
	always @(posedge CLK50) begin
		counter = counter + 6'b1;
	end

	State s1(
		.CLK50(CLK50),
		.KEY(PAUSE_KEY),
		.state(pause)
	);

	State s2(
		.CLK50(CLK50),
		.KEY(STOP_KEY),
		.state(stop)
	);

	Debounce db1(
		.CLK50(CLK_400K),
		.KEY(RESET_KEY),
		.negEdge(reset)
	);

	Clock c1(
		.CLK50(CLK50),
		.ratio_m1(RATIO_SW),
		.isNormalSpeed(NORMAL_SPEED_SW),
		.isSlow(IS_SLOW_SW),
		.interp(INTERP_SW),
		.pause(pause),
		.isRecord(RECORD_SW),
		.clkOut(AUD_XCK)
	);
	assign GPIO[0] = ~(~I2C_SDAT);
	assign GPIO[1] = ~(~I2C_SCLK);
	assign GPIO[2] = ~(~reset);
	assign GPIO[3] = ~(~AUD_BCLK);
	assign GPIO[4] = ~(~AUD_XCK);
	assign GPIO[5] = ~(~AUD_ADCLRCK);
	assign GPIO[6] = ~(~AUD_DACLRCK);
	assign GPIO[22:7] = (DEBUG_SW? data_fr_sram: data_to_sram);
	assign GPIO[23] = ~(~AUD_DACDAT);
	assign GPIO[24] = ~(~AUD_ADCDAT);
	assign GPIO[30] = ~(~SRAM_OE);
	assign GPIO[31] = ~(~SRAM_WE);

	wire write, read;
	wire [15:0] data_fr_sram, data_to_sram;
	Codec cd1(
		.AUD_BCLK(AUD_BCLK),
		.AUD_DACLRCK(AUD_DACLRCK),
		.AUD_DACDAT(AUD_DACDAT),
		.fast(!(IS_SLOW_SW|NORMAL_SPEED_SW)),
		.slow((IS_SLOW_SW && (!NORMAL_SPEED_SW))),
		.interp(INTERP_SW),
		.rate(ratio),
		.stop(stop),
		.data_fr_sram(data_fr_sram),
		.read(read),
		.AUD_ADCLRCK(AUD_ADCLRCK),
		.AUD_ADCDAT(AUD_ADCDAT),
		.record(RECORD_SW),
		.data_to_sram(data_to_sram),
		.write(write),
		.address(address),
		.counter(GPIO[29:25])
	);

	Display d1(
		.inTime(address[17:13]),
		.inRate(ratio),
		.IS_SLOW(IS_SLOW_SW),
		.IS_RECORD(RECORD_SW),
		.IS_PAUSE(pause),
		.INTERP_MODE(INTERP_SW),
		.IS_NORMAL_SPEED(),

		.SEVEN10(HEX7),
		.SEVEN1(HEX6),
		.PAUSE(HEX5),
		.REC(HEX4),
		.SLOW(HEX3),
		.RATIO(HEX2),
		.NOT_USED(HEX1),
		.INTERP(HEX0)
	);

	i2c i1(
		.I2C_SCLK(I2C_SCLK),
		.I2C_SDAT(I2C_SDAT),
		.clk(CLK_400K),
		.reset(reset)
	);

	Sram2Codec sc1(
		.dataStream(SRAM_DQ),
		.dataR(data_fr_sram),
		.dataW(data_to_sram),
		.addrIn(address),
		.write(write),
		.read(read),
		.on(1),
		.clk(AUD_BCLK),
		._WE(SRAM_WE),
		._CE(SRAM_CE),
		._OE(SRAM_OE),
		._LB(SRAM_LB),
		._UB(SRAM_UB),
		._Addr(SRAM_ADDR)
	);
endmodule
