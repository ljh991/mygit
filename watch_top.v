`timescale 1ns / 1ps

module watch_top(
    clk,
    reset,
	i_run_en,
	i_freq,
    o_sec,
    o_min,
    o_hour
    );
parameter P_COUNT_BIT = 30; // (default) 30b, under 1GHz. 2^30 = 1073741824
parameter P_SEC_BIT	 = 6; // 2^6 = 64
parameter P_MIN_BIT	 = 6; // 2^6 = 64 
parameter P_HOUR_BIT = 5; // 2^5 = 32 

input 							clk;
input 							reset;
input 							i_run_en;
input		[P_COUNT_BIT-1:0]	i_freq;
output reg 	[P_SEC_BIT-1:0]		o_sec;
output reg 	[P_MIN_BIT-1:0]		o_min;
output reg 	[P_HOUR_BIT-1:0]	o_hour;

wire w_one_sec_tick;

// Gen one sec
one_sec_gen 
# (
	.P_COUNT_BIT	(P_COUNT_BIT) 
) u_one_sec_gen(
	.clk 				(clk			),
	.reset 				(reset			),
	.i_run_en			(i_run_en		),
	.i_freq				(i_freq			),
	.o_one_sec_tick 	(w_one_sec_tick	)
);

reg [17-1:0] r_sec_cnt; // 60*60*24 < 2^17 
wire [6-1:0] sec_val = r_sec_cnt % 60;
wire [6-1:0] min_val = (r_sec_cnt/60) % 60;
wire [5-1:0] hour_val = (r_sec_cnt/(60*60)) % 24;
	always @(posedge clk) begin
	    if(reset) begin
			o_sec		<= 0;
			o_min		<= 0;
			o_hour		<= 0;
			r_sec_cnt 	<= 0;
		end else if(w_one_sec_tick) begin
			if (r_sec_cnt == 24*60*60-1)  begin
				r_sec_cnt 	<= 0;
			end else begin
				r_sec_cnt <= r_sec_cnt + 1'b1;
			end
			o_sec		<= sec_val ;
			o_min		<= min_val ;
			o_hour		<= hour_val;
		end
	end
endmodule
