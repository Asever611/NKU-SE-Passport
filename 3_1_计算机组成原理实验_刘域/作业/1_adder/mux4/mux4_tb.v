`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2025/10/23 19:46:59
// Design Name: 
// Module Name: mux4_tb
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


module mux4_tb;
    // 输入信号
    reg [31:0] data0;
    reg [31:0] data1;
    reg [31:0] data2;
    reg [31:0] data3;
    reg [1:0]  sel;
    // 输出信号
    wire [31:0] out;

    // 实例化被测试模块
    mux4 uut (
        .data0(data0),
        .data1(data1),
        .data2(data2),
        .data3(data3),
        .sel(sel),
        .out(out)
    );

    // 生成4组测试波形
    initial begin
        // 初始化输入数据
        data0 = 32'hA0A0A0A0;
        data1 = 32'hB1B1B1B1;
        data2 = 32'hC2C2C2C2;
        data3 = 32'hD3D3D3D3;
        sel = 2'b00;
        #100;

        // 第1组：选择data0（sel=00）
        sel = 2'b00;
        #100;

        // 第2组：选择data1（sel=01）
        sel = 2'b01;
        #100;

        // 第3组：选择data2（sel=10）
        sel = 2'b10;
        #100;

        // 第4组：选择data3（sel=11）
        sel = 2'b11;
        #100;

        // 结束仿真
        $finish;
    end

endmodule
