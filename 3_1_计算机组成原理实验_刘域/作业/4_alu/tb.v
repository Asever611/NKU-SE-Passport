`timescale 1ns / 1ps



module tb;

    // Inputs
    reg clk;
    reg [12:0] alu_control;
    reg [31:0] alu_src1;
    reg [31:0] alu_src2;

    // Outputs
    wire [31:0] alu_result;

    // Instantiate the Unit Under Test (UUT)
    alu uut (
        .alu_control(alu_control),
        .alu_src1(alu_src1),
        .alu_src2(alu_src2),
        .alu_result(alu_result)
    );

    initial begin
        //Initialize Inputs
        clk = 0;
        alu_control = 12'd0;
        alu_src1 = 32'd0;
        alu_src2 = 32'd0;

        // Wait 100 ns for global reset to finish
        #100;
        
        // 有符号比较大于 alu_gt，控制信号 bit12 = 1
        alu_control = 13'b1000000000000;
        // 测试 1：1 > 2 (0)
        alu_src1 = 32'h00000001;
        alu_src2 = 32'h00000002;
        #25;
        // 测试2：-1 > -2 (1)
        alu_src1 = 32'hFFFFFFFF;
        alu_src2 = 32'hFFFFFFFE;
        #25;
        
        $finish; //停止仿真
        
        // 1. 加法运算 alu_add，控制信号 bit11 = 1
        alu_control = 12'b100000000000;
        // 测试 1：正数 + 正数（1 + 2 = 3）
        alu_src1 = 32'h00000001;
        alu_src2 = 32'h00000002;
        #25;
        // 测试 2：负数 + 正数（-1 + 2 = 1）
        alu_src1 = 32'hFFFFFFFF;
        alu_src2 = 32'h00000002;
        #25;
        
        // 2. 减法运算 alu_sub，控制信号 bit10 = 1
        alu_control = 12'b010000000000;
        // 测试 1：正数 - 正数（2 - 1 = 1）
        alu_src1 = 32'h00000002;
        alu_src2 = 32'h00000001;
        #25;
        // 测试 2：负数 - 负数（-2 - (-1) = -1）
        alu_src1 = 32'hFFFFFFFE;
        alu_src2 = 32'hFFFFFFFF;
        #25;
        
        // 3. 有符号比较小于 alu_slt，控制信号 bit9 = 1
        alu_control = 12'b001000000000;
        // 测试 1：2 < 1（0）
        alu_src1 = 32'h00000002;
        alu_src2 = 32'h00000001;
        #25;
        // 测试 2：-2 < -1（1）
        alu_src1 = 32'hFFFFFFFE; // -5
        alu_src2 = 32'hFFFFFFFF;
        #25;
        
        // 4. 无符号比较小于 alu_sltu，控制信号 bit8 = 1
        alu_control = 12'b000100000000;
        // 测试 1：1 < 2（1）
        alu_src1 = 32'h00000001;
        alu_src2 = 32'h00000002;
        #25;
        // 测试 2：0xFFFFFFFF < 1（0）
        alu_src1 = 32'hFFFFFFFF;
        alu_src2 = 32'h00000001;
        #25;
        
        // 5. 按位与 alu_and，控制信号 bit7 = 1
        alu_control = 12'b000010000000;
        // 测试1：0x0F0F0F0F & 0xF0F0F0F0 = 0x00000000
        alu_src1 = 32'h0F0F0F0F;
        alu_src2 = 32'hF0F0F0F0;
        #25;
        // 测试2：0x0F0F0F0F & 0x0F0F0F0F = 0x0F0F0F0F
        alu_src1 = 32'h0F0F0F0F;
        alu_src2 = 32'h0F0F0F0F;
        #25;

        // 6. 按位或非 alu_nor，控制信号 bit6 = 1
        alu_control = 12'b000001000000;
        // 测试 1：~(0x0F0F0F0F & 0xF0F0F0F0) = 0x00000000
        alu_src1 = 32'h0F0F0F0F;
        alu_src2 = 32'hF0F0F0F0;
        #25;
        // 测试 2：~(0x00000000 | 0x00000000) = 0xFFFFFFFF
        alu_src1 = 32'h00000000;
        alu_src2 = 32'h00000000;
        #25;

        // 7. 按位或 alu_or，控制信号 bit5 = 1
        alu_control = 12'b000000100000;
        // 测试 1：0x0F0F0F0F & 0xF0F0F0F0 = 0xFFFFFFFF
        alu_src1 = 32'h0F0F0F0F;
        alu_src2 = 32'hF0F0F0F0;
        #25;
        // 测试 2：0x00000000 | 0x00000000 = 0x00000000
        alu_src1 = 32'h00000000;
        alu_src2 = 32'h00000000;
        #25;

        // 8. 按位异或 alu_xor，控制信号 bit4 = 1
        alu_control = 12'b000000010000;
        // 测试 1：0x0000000F ^ 0x000000F0 = 0x000000FF
        alu_src1 = 32'h0000000F;
        alu_src2 = 32'h000000F0;
        #25;
        // 测试 2：0x0000000F ^ 0x0000000F = 0x00000000
        alu_src1 = 32'h0000000F;
        alu_src2 = 32'h0000000F;
        #25;

        // 9. 逻辑左移 alu_sll，控制信号 bit3 = 1
        alu_control = 12'b000000001000;
        // 测试 1：0x00000001 << 1 = 0x00000002 左移 1 位
        alu_src1 = 32'h00000001;
        alu_src2 = 32'h00000001;
        #25;
        // 测试 2：0x00000001 << 4 = 0x00000010 左移 4 位
        alu_src1 = 32'h00000004;
        alu_src2 = 32'h00000001;
        #25;

        // 10. 逻辑右移 alu_srl，控制信号 bit2 = 1
        alu_control = 12'b000000000100;
        // 测试 1：0x00000100 >> 8 = 0x00000001
        alu_src1 = 32'h00000008;
        alu_src2 = 32'h00000100;
        #25;
        // 测试 2：0xFFFFFFFF >> 16 = 0x0000FFFF
        alu_src1 = 32'h00000010;
        alu_src2 = 32'hFFFFFFFF;
        #25;

        // 11. 算术右移 alu_sra，控制信号 bit1 = 1
        alu_control = 12'b000000000010;
        // 测试 1：-16（0xFFFFFFF0）>> 4 = -1
        alu_src1 = 32'h00000004;
        alu_src2 = 32'hFFFFFFF0;
        #25;
        // 测试 2：0x000000F0 >> 4 = 0x0000000F
        alu_src1 = 32'h00000004;
        alu_src2 = 32'h000000F0;
        #25;

        // 12. 高位加载 alu_lui，控制信号 bit0 = 1
        alu_control = 12'b000000000001;
        // 测试 1：0x00001111 → 0x11110000
        alu_src2 = 32'h00001111;
        #25;
        // 测试 2：0x0000FFFF → 0xFFFF0000
        alu_src2 = 32'h0000FFFF;
        #25;

        
        $finish; //停止仿真
        
    end
   always #5 clk = ~clk; 
endmodule

