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
    reg wen;
    reg [4:0] raddr1;
    reg [4:0] raddr2;
    reg [4:0] waddr;
    reg [31:0] wdata;
    reg [4:0] test_addr;

    // Outputs
    wire [31:0] rdata1;
    wire [31:0] rdata2;
    wire [31:0] test_data;

    // Instantiate the Unit Under Test (UUT)
    regfile uut (
        .clk(clk),
        .wen(wen),
        .raddr1(raddr1),
        .raddr2(raddr2),
        .waddr(waddr),
        .wdata(wdata),
        .rdata1(rdata1),
        .rdata2(rdata2),
        .test_addr(test_addr),
        .test_data(test_data)
    );
    
    initial begin
        //Initialize Inputs
        clk = 0;
        wen = 0;
        raddr1 = 5'd0;
        raddr2 = 5'd0;
        waddr = 5'd0;
        wdata = 32'd0;
        test_addr = 5'd0;

        // Wait 100 ns for global reset to finish
        #100;
        
        // ²âÊÔ1£ºĞ´Å¼Êı
        wen = 1'b1;
        waddr = 5'd1;
        wdata = 32'h12345678;
        #20;
        wen = 1'b0;
        #30;
        raddr1 = 5'd1;
        raddr2 = 5'd1;
        test_addr = 5'd1;
        #50;
        
        // ²âÊÔ2£ºĞ´ÆæÊı
        wen = 1'b1;
        waddr = 5'd1;
        wdata = 32'h87654321;
        #20;
        wen = 1'b0;
        #30;
        raddr1 = 5'd1;
        raddr2 = 5'd1;
        test_addr = 5'd1;
        #50;
        
        // ²âÊÔ3£ºÔÙĞ´Å¼Êı
        wen = 1'b1;
        waddr = 5'd1;
        wdata = 32'h88888888;
        #20;
        wen = 1'b0;
        #30;
        raddr1 = 5'd1;
        raddr2 = 5'd1;
        test_addr = 5'd1;
        #50;
        
        // ²âÊÔ4£ºĞ´Ê¹ÄÜ¹Ø±Õ
        wen = 1'b0;
        waddr = 5'd8;
        wdata = 32'h11223344;
        #50;
        raddr1 = 5'd1;
        raddr2 = 5'd1;
        test_addr = 5'd1;
        #50;
        
        $finish; //Í£Ö¹·ÂÕæ
        
    end
   always #5 clk = ~clk; 
endmodule

