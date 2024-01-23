////////////////
// PWM generator
// Lee Jihyeon
// Description : Advanced project from 555timer
//
//
/////////////////
`timescale 1ns/1ps
module pwm_gen (
    clk, // 100MHz
    reset,
    i_increase,
    i_decrease,
    o_pwm //10MHz
);
input   clk, reset;
input   i_increase;
input   i_decrease;
output  o_pwm;

reg [3:0] counter_pwm; // counter for creating 10MHz PWM
reg [3:0] duty_cycle; // initial duty cycle 50%

// main //
always @(posedge clk or reset) begin
    if(reset) begin
        // Initialize
        counter_pwm = 0;
        duty_cycle = 5;
    end else if(i_increase) begin // Increase duty_cycle
        if(duty_cycle < 9) begin
            duty_cycle = duty_cycle + 1;
        end else begin
            duty_cycle = 9;
        end
    end else if(i_decrease) begin // Decrease duty_cycle
        if(duty_cycle > 1) begin
            duty_cycle = duty_cycle - 1;
        end else begin
            duty_cycle = 1;
        end
    end
end

// Create 10MHz PWM
always @(posedge clk) begin
    counter_pwm <= counter_pwm + 1;
    if(counter_pwm == 9)
    counter_pwm <= 0;
end

assign o_pwm = counter_pwm < duty_cycle ? 1:0;

endmodule