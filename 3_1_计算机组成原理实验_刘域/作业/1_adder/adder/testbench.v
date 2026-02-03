`timescale 1ns / 1ps   //仿真单位时间为1ns，精度为1ps
module testbench;

    // Inputs
    reg [31:0] operand1;
    reg [31:0] operand2;
    reg cin;

    // Outputs
    wire [31:0] result;
    wire cout;
    // Instantiate the Unit Under Test (UUT)
    adder uut (
        .operand1(operand1), 
        .operand2(operand2), 
        .cin(cin), 
        .result(result), 
        .cout(cout)
    );
    initial begin
        // Initialize Inputs
        operand1 = 0;
        operand2 = 0;
        cin = 0;
        // Wait 100 ns for global reset to finish
        #100;
        // Add stimulus here
        
        // 第1组测试：无进位加法（1 + 2 + 0）
        operand1 = 32'h0000_0001;
        operand2 = 32'd0000_0002;
        cin = 1'b0;
        #20;  // 保持20ns

        // 第2组测试：带进位输入加法（1 + 2 + 1）
        operand1 = 32'h0000_0001;
        operand2 = 32'd0000_0002;
        cin = 1'b1;
        #20;  // 保持20ns

        // 第3组测试：产生进位输出加法（0xFFFFFFFF + 1 + 0）
        operand1 = 32'hFFFF_FFFF;
        operand2 = 32'h0000_0001;
        cin = 1'b0;
        #20;  // 保持20ns

        // 第4组测试：边界值测试（0xFFFFFFFF + 0xFFFFFFFF + 1）
        operand1 = 32'hFFFF_FFFF;
        operand2 = 32'hFFFF_FFFF;
        cin = 1'b1;
        #20;  // 保持20ns
    end
//    always #10 operand1 = $random;  //$random为系统任务，产生一个随机的32位数
//    always #10 operand2 = $random;  //#10 表示等待10个单位时间(10ns)，即每过10ns，赋值一个随机的32位数
//    always #10 cin = {$random} % 2; //加了拼接符，{$random}产生一个非负数，除2取余得到0或1
endmodule

