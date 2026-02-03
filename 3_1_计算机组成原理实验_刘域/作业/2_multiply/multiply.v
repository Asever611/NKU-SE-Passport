`timescale 1ns / 1ps
//*************************************************************************
//   > 文件名: multiply.v
//   > 描述  ：乘法器模块，低效率的迭代乘法算法，使用两个乘数绝对值参与运算
//   > 作者  : LOONGSON
//   > 日期  : 2016-04-14
//*************************************************************************
module multiply(              // 乘法器
    input         clk,        // 时钟
    input         mult_begin, // 乘法开始信号
    input  [31:0] mult_op1,   // 乘法源操作数1
    input  [31:0] mult_op2,   // 乘法源操作数2
    output [63:0] product,    // 乘积
    output        mult_end    // 乘法结束信号
);

    //乘法正在运算信号和结束信号
    reg mult_valid;
    assign mult_end = mult_valid & ~(|multiplier); //乘法结束信号：乘数全0
    always @(posedge clk)
    begin
        if (!mult_begin || mult_end)
        begin
            mult_valid <= 1'b0;
        end
        else
        begin
            mult_valid <= 1'b1;
        end
    end

    //两个源操作取绝对值，正数的绝对值为其本身，负数的绝对值为取反加1
    wire        op1_sign;      //操作数1的符号位
    wire        op2_sign;      //操作数2的符号位
    wire [31:0] op1_absolute;  //操作数1的绝对值
    wire [31:0] op2_absolute;  //操作数2的绝对值
    //Add code here
    assign op1_sign = mult_op1[31]; //最高位为符号位（补码）
    assign op2_sign = mult_op2[31];
    assign op1_absolute = op1_sign ? (~mult_op1 + 1) : mult_op1;
    assign op2_absolute = op2_sign ? (~mult_op2 + 1) : mult_op2;
    
    //加载被乘数，运算时每次左移一位
    reg  [63:0] multiplicand;
    always @ (posedge clk)
    begin
    //Add code here
        if (mult_valid) //运算中每次左移1位
            multiplicand <= multiplicand << 1'b1;
        else if (mult_begin) //开始信号有效时加载被乘数绝对值，扩展为64位
            multiplicand <= {32'd0, op1_absolute};
    end

    //加载乘数，运算时每次右移一位
    reg  [31:0] multiplier;
    always @ (posedge clk)
    begin
    //Add code here
        if (mult_valid) //运算中每次右移1位
            multiplier <= multiplier >> 1'b1;
        else if (mult_begin) //开始信号有效时加载乘数绝对值
            multiplier <= op2_absolute;
    end
    
    // 部分积：乘数末位为1，由被乘数左移得到；乘数末位为0，部分积为0
    wire [63:0] partial_product;
    //Add code here
    assign partial_product = multiplier[0] ? multiplicand : 64'd0;
   
    //累加器
    reg [63:0] product_temp;
    always @ (posedge clk)
    begin
    //Add code here
        if (mult_valid) //运算中累加部分积
            product_temp <= product_temp + partial_product;
        else if (mult_begin) //开始信号有效时清零累加器
            product_temp <= 64'd0;
    end 
     
    //乘法结果的符号位和乘法结果
    reg product_sign;
    always @ (posedge clk)  // 乘积
    begin
    //Add code here
        if (mult_valid) begin
            product_sign <= op1_sign ^ op2_sign; // 符号位异或
        end
    end

    //若乘法结果为负数，则需要对结果取反+1
    //Add code here
    assign product = product_sign ? (~product_temp + 1) : product_temp;
endmodule
