`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2025/10/23 19:39:05
// Design Name: 
// Module Name: mux4
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module mux4(
    input [31:0] data0,    // 输入数据0
    input [31:0] data1,    // 输入数据1
    input [31:0] data2,    // 输入数据2
    input [31:0] data3,    // 输入数据3
    input [1:0]  sel,      // 选择信号
    output reg [31:0] out  // 输出数据
    );
    
    always @(*) begin
        case(sel)
            2'b00: out = data0;
            2'b01: out = data1;
            2'b10: out = data2;
            2'b11: out = data3;
            default: out = 32'b0;
        endcase
    end
endmodule
