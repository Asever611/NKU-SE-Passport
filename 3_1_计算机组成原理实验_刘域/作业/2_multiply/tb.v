`timescale 1ns / 1ps

////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer:
//
// Create Date:   17:13:38 04/15/2016
// Design Name:   multiply
// Module Name:   F:/new_lab/multiply/tb.v
// Project Name:  multiply
// Target Device:  
// Tool versions:  
// Description: 
//
// Verilog Test Fixture created by ISE for module: multiply
//
// Dependencies:
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
////////////////////////////////////////////////////////////////////////////////

module tb;

    // Inputs
    reg clk;
    reg mult_begin;
    reg [31:0] mult_op1;
    reg [31:0] mult_op2;

    // Outputs
    wire [63:0] product;
    wire mult_end;

    // Instantiate the Unit Under Test (UUT)
    multiply uut (
        .clk(clk), 
        .mult_begin(mult_begin), 
        .mult_op1(mult_op1), 
        .mult_op2(mult_op2), 
        .product(product), 
        .mult_end(mult_end)
    );

    initial begin
         //Initialize Inputs
         clk = 0;
         mult_begin = 0;
         mult_op1 = 0;
         mult_op2 = 0;

        // Wait 100 ns for global reset to finish
        #100;
        // 添加4组信号做为激励信号，得出正确波形，并验证功能。
        // 测试1：正数×正数（3 × 5 = 15）
        mult_begin = 1'b1;
        mult_op1 = 32'h00000003;
        mult_op2 = 32'h00000005;
        #50;
        mult_begin = 1'b0;
        #100;
        
        // 测试2：正数×负数（3 × (-5) = -15）
        mult_begin = 1'b1;        
        mult_op1 = 32'h00000003;
        mult_op2 = 32'hFFFFFFFB; //-5
        #50;
        mult_begin = 1'b0;
        #100;
        
        // 测试3：负数×正数（(-3) × 5 = -15）
        mult_begin = 1'b1;        
        mult_op1 = 32'hFFFFFFFD; //-3
        mult_op2 = 32'h00000005;
        #50;
        mult_begin = 1'b0;
        #100;
        
        // 测试4：负数×负数（(-3) × (-5) = 15）
        mult_begin = 1'b1;        
        mult_op1 = 32'hFFFFFFFD;
        mult_op2 = 32'hFFFFFFFB;
        #50;
        mult_begin = 1'b0;
        #100;
        
        $finish; //停止仿真
        
    end
   always #5 clk = ~clk; 
endmodule

