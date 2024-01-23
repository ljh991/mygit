////////////////
// PWM generator
// Lee Jihyeon
// Description : Advanced project from 555timer
// Verilog project: testbench code for PWM Generator with variable duty cycle
//
/////////////////
`timescale 1ns / 1ps 
module tb_pwm_gen;
 // Inputs
 reg clk;
 reg reset;
 reg i_increase;
 reg i_decrease;
 // Outputs
 wire o_pwm;
 // Instantiate 
 pwm_gen u_pwm_gen(
  .clk(clk), 
  .reset(reset),
  .i_increase(i_increase), 
  .i_decrease(i_decrease), 
  .o_pwm(o_pwm)
 );
 // Create 100Mhz clock
 initial begin
 clk = 0; 
 reset = 0;
 forever #5 clk = ~clk;
 end 
 initial begin
  i_increase = 0;
  i_decrease = 0;
  #10     reset = 1;
  #10     reset = 0;
  #100    i_increase = 1; 
  #10    i_increase = 0;
  #100    i_increase = 1;
  #10    i_increase = 0;
  #100    i_increase = 1;
  #10    i_increase = 0;
  #100    i_decrease = 1;
  #10    i_decrease = 0;
  #100    i_decrease = 1;
  #10    i_decrease = 0;
  #100    i_decrease = 1;
  #10    i_decrease = 0;
   #100    i_decrease = 1;
   #10    i_decrease = 0;
   #100    i_decrease = 1;
    #10    i_decrease = 0;
    #100    i_decrease = 1;
     #10    i_decrease = 0;
    
 end
endmodule