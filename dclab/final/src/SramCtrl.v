module Sram(
	// Note: clock to SRAM must be 4x faster than i_clk
	i_clk,
	i_addr,
	i_rw, // 1 = read, 0 = write
	i_data,
	// lines to real hardware
	io_SRAM_DQ,
	o_SRAM_A,
	o_SRAM_ADSC_N,
	o_SRAM_ADSP_N,
	o_SRAM_ADV_N,
	o_SRAM_BE_N,
	o_SRAM_CE1_N,
	o_SRAM_CE2,
	o_SRAM_CE3_N,
	o_SRAM_GW_N,
	o_SRAM_OE_N,
	o_SRAM_WE_N,
	// output data
	o_data
);
	input i_clk;
	input [20:0] i_addr;
	input i_rw;
	input [7:0] i_data;

	inout [31:0] io_SRAM_DQ;

	output [18:0] o_SRAM_A;
	output o_SRAM_ADSC_N;
	output o_SRAM_ADSP_N;
	output o_SRAM_ADV_N;
	output [3:0] o_SRAM_BE_N;
	output o_SRAM_CE1_N;
	output o_SRAM_CE2;
	output o_SRAM_CE3_N;
	output o_SRAM_GW_N;
	output o_SRAM_OE_N;
	output o_SRAM_WE_N;
	output [7:0] o_data;

	reg [3:0] o_SRAM_BE_N;
	reg [7:0] o_data;

	assign io_SRAM_DQ = i_rw? 32'bz: {4{i_data}};
	assign o_SRAM_ADSC_N = 1'b0;
	assign o_SRAM_ADSP_N = 1'b1;
	assign o_SRAM_ADV_N = 1'b1;
	assign o_SRAM_CE1_N = 1'b0;
	assign o_SRAM_CE2 = 1'b1;
	assign o_SRAM_CE3_N = 1'b0;
	assign o_SRAM_OE_N = 1'b0;
	assign o_SRAM_GW_N = 1'b1;
	assign o_SRAM_WE_N = i_rw;
	assign o_SRAM_A = i_addr[20:2];

	always @(*) begin
		case(i_addr[1:0])
			2'h0: begin
				o_SRAM_BE_N = 4'b1110;
				o_data = io_SRAM_DQ[7:0];
			end
			2'h1: begin
				o_SRAM_BE_N = 4'b1101;
				o_data = io_SRAM_DQ[15:8];
			end
			2'h2: begin
				o_SRAM_BE_N = 4'b1011;
				o_data = io_SRAM_DQ[23:16];
			end
			2'h3: begin
				o_SRAM_BE_N = 4'b0111;
				o_data = io_SRAM_DQ[31:24];
			end
		endcase
	end

endmodule
