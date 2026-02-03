`timescale 1ns / 1ps

////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer:
//
// Create Date:   16:49:44 04/19/2016
// Design Name:   single_cycle_cpu
// Module Name:   F:/new_lab/6_single_cycle_cpu/tb.v
// Project Name:  single_cycle_cpu
// Target Device:  
// Tool versions:  
// Description: 
//
// Verilog Test Fixture created by ISE for module: single_cycle_cpu
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
    reg resetn;
    reg [4:0] rf_addr;
    reg [31:0] mem_addr;

    // Outputs
    wire [31:0] rf_data;
    wire [31:0] mem_data;
    wire [31:0] cpu_pc;
    wire [31:0] cpu_inst;

    // Instantiate the Unit Under Test (UUT)
    single_cycle_cpu uut (
        .clk(clk), 
        .resetn(resetn), 
        .rf_addr(rf_addr), 
        .mem_addr(mem_addr), 
        .rf_data(rf_data), 
        .mem_data(mem_data), 
        .cpu_pc(cpu_pc), 
        .cpu_inst(cpu_inst)
    );

    initial begin
        // Initialize Inputs
        clk = 0;
        resetn = 0;
        rf_addr = 0;
        mem_addr = 0;

        // Wait 100 ns for global reset to finish
        #25;
        resetn = 1;
        // Add stimulus here
        
        
        rf_addr = 5'd1; // 0. 观察 addiu 初始化的 $1（预期0x00000001） 
        #10;
        rf_addr = 5'd2; // 1. 观察 sll 后的 $2（预期 0x00000010）
        #10;
        rf_addr = 5'd3; // 2. 观察 addu 后的 $3（预期0x00000011） 
        #10;
        rf_addr = 5'd4; // 3. 观察 srl 后的 $4（预期 0x00000004）
        #10;
        rf_addr = 5'd5; // 4. 观察 subu 后的 $5（预期0x0000000D） 
        #10;
        mem_addr = 32'h00000014; // 5. 观察 sw 后的 #20（预期0x0000000D） 
        rf_addr = 5'd0; // 使波形整齐
        #10;
        rf_addr = 5'd6; // 6. 观察 nor 后的 $6（预期 0xFFFFFE2）
        #10;
        rf_addr = 5'd7; // 7. 观察 or 后的 $7（预期0xFFFFFFF3） 
        #10;
        rf_addr = 5'd8; // 8. 观察 xor 后的 $8（预期 0x00000011）
        #10;
        mem_addr = 32'h0000001C; // 9. 观察 sw 后的 #28（预期 0x00000011）
        rf_addr = 5'd0; // 使波形整齐
        #10;
        rf_addr = 5'd9; // 10. 观察 slt 后的 $9（预期 0x00000001）
        #10;
        rf_addr = 5'd0; // 11. beq 跳转，应观察 PC，随便设个 $0 使波形整齐（预期 PC: 0x00000034） 
        #10;
        rf_addr = 5'd1; // 12. 观察 addiu 被跳过后的 $1（预期0x00000001） 
        #10;
        rf_addr = 5'd10; // 13. 观察 lw 读出的 $10（预期0x0000000D） 
        #10;
        rf_addr = 5'd0; // 14. bne 不跳转，应观察 PC，随便设个 $0 使波形整齐（预期 PC: 0x0000003C） 
        #10;
        rf_addr = 5'd11; // 15. 观察 and 后的 $11（预期 0x00000000）
        #10;
        mem_addr = 32'h0000001C; // 16. 观察 sw 后的 #28（预期 0x00000000）
        rf_addr = 5'd0; // 使波形整齐
        #10;
        mem_addr = 32'h00000010; // 17. 观察 sw 后的 #16（预期 0x00000004）
        rf_addr = 5'd0; // 使波形整齐
        #10;
        rf_addr = 5'd12; // 18. 观察 lui 后的 $12（预期 0x000C0000）
        #10;
        rf_addr = 5'd0; // 19. j 跳转，应观察 PC，随便设个 $0 使波形整齐（预期 PC: 0x00000000） 
        #10;
        // 观察 PC 是否跳回 0x00000000
        #10;
        
        // 仿真结束
        $finish;
    end
    always #5 clk=~clk;
endmodule

