`timescale 1ns / 1ps
//*************************************************************************
//   > 文件名: regfile.v
//   > 描述  ：寄存器堆模块，同步写，异步读
//*************************************************************************
module regfile(
    input             clk,                 //时钟
    input             wen,                 //写使能
    input      [4 :0] raddr1,              //读地址1
    input      [4 :0] raddr2,              //读地址2
    input      [4 :0] waddr,               //写地址
    input      [31:0] wdata,               //写数据
    output reg [31:0] rdata1,              //读数据1
    output reg [31:0] rdata2,              //读数据2
    input      [4 :0] test_addr,           //测试地址
    output reg [31:0] test_data            //测试数据
    );
    reg [31:0] rf[31:0];                    //定义32个，每个32位的寄存器
     
    // three ported register file
    // read two ports combinationally
    // write third port on rising edge of clock
    // register 0 hardwired to 0

    always @(posedge clk)                //时钟上升沿
    begin
        if (wen && (wdata[0] == 1'b0))    //写使能为1且数据为偶数，则写入寄存器
        begin
            rf[waddr] <= wdata;
        end
    end
     
    //读端口1
    always @(*)                           //alwasys@（*）任意输入或电平发生变化
    begin
    //此处添加代码完成读端口1
        if (raddr1 == 5'd0)                //0号寄存器为0
            rdata1 <= 32'd0;
        else
            rdata1 <= rf[raddr1];          //非0号寄存器读取对应地址
    end
    
    
    //读端口2
    //此处添加代码完成读端口2
    always @(*)
    begin
        if (raddr2 == 5'd0)
            rdata2 <= 32'd0;
        else
            rdata2 <= rf[raddr2];
    end
    
   
    //调试端口，读出寄存器值显示在触摸屏上
    //此处添加代码完成测试端口
    always @(*)
    begin
        if (test_addr == 5'd0)
            test_data <= 32'd0;
        else
            test_data <= rf[test_addr];
    end
       
endmodule
