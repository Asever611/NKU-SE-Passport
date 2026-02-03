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
    reg [3:0] wen;
    reg [4:0] addr;
    reg [31:0] wdata;
    reg [4:0] test_addr;

    // Outputs
    wire [31:0] rdata;
    wire [31:0] test_data;

    // Instantiate the Unit Under Test (UUT)
    data_ram uut (
        .clk(clk),
        .wen(wen),
        .addr(addr),
        .wdata(wdata),
        .rdata(rdata),
        .test_addr(test_addr),
        .test_data(test_data)
    );


    initial begin
        // 初始化输入信号
        clk = 0;
        wen = 4'b0000;
        addr = 5'd0;
        wdata = 32'd0;
        test_addr = 5'd0;

        // 等待100ns全局复位稳定
        #100;

        // 测试1：4字节同时写入
        // 地址1写入0x11111111
        wen = 4'b1111;
        addr = 5'd1;
        wdata = 32'h11111111;
        test_addr = 5'd1;
        #10;

        // 测试2：单字节写，仅写第0字节
        // 地址2的最低字节写入0x11
        wen = 4'b0001;
        addr = 5'd2;
        wdata = 32'h11111111;
        test_addr = 5'd2;
        #10;

        // 测试3：双字节写（中间两字节）
        // 地址3中间两字节写入0x1111
        wen = 4'b0110;
        addr = 5'd3;
        wdata = 32'h11111111;
        test_addr = 5'd3;
        #10;

        // 测试4：地址边界测试（最大地址31）
        // 地址31写入0x11111111
        wen = 4'b1111;
        addr = 5'd31;
        wdata = 32'h11111111;
        test_addr = 5'd31;
        #10;
        
        // 测试5：无写使能时写操作无效
        // 写入地址4，但数据应保持不变
        wen = 4'b0000;
        addr = 5'd4;
        wdata = 32'h11111111;
        test_addr = 5'd4;
        #10;

        // 测试6：读测试
        // 读取前面测试已写入的地址
        test_addr = 5'd1;
        #10;
        test_addr = 5'd2;
        #10;
        test_addr = 5'd3;
        #10;
        test_addr = 5'd31;
        #10;

        // 仿真结束
        $finish;
    end
    always #5 clk = ~clk; 
endmodule

