#include <graphics.h>
#include <conio.h>

/*定义宏:方块宽度、基准坐标x、y，窗口宽度、高度等*/
#define BLOCK_WIDTH 20    // 蛇身体方块的宽度
#define WIN_WIDTH 640     // 窗口宽度
#define WIN_HEIGHT 480    // 窗口高度
#define BASE_X 20         // 绘制基准X坐标
#define BASE_Y 20         // 绘制基准Y坐标

const int snakeLength = 12;

/*蛇身体的初始化坐标*/
int snake[12][2] =
{
    {14, 11},
    {13, 11},
    {13, 10},
    {12, 10},
    {11, 10},
    {11, 9},
    {11, 8},
    {11, 7},
    {10, 7},
    {9, 7},
    {8, 7},
    {7, 7}
};

/*画出蛇身体的单元格（参数：方块坐标，方块索引）*/
void drawUnitBlock(int x, int y, int segment) {
    // 计算方块实际绘制坐标 = 基准坐标 + 行列 * 方块宽度
    int drawX = BASE_X + x * BLOCK_WIDTH;
    int drawY = BASE_Y + y * BLOCK_WIDTH;

    COLORREF fillColor, lineColor;
    if (segment == 0) {
        fillColor = RGB(255, 150, 150);
        lineColor = RGB(255, 140, 140);
    } else {
        // 蛇身：粉色渐变
        int g = 180 + (segment * 60) / (snakeLength - 1);
        int b = 180 + (segment * 60) / (snakeLength - 1);
        fillColor = RGB(255, g, b);
        // 边框颜色：比填充色略深
        lineColor = RGB(255, g - 10, b - 10);
    }

    // 设置蛇身颜色
    setfillcolor(fillColor);
    setlinecolor(lineColor);

    // 绘制带边框的矩形方块
    fillrectangle(drawX, drawY, drawX + BLOCK_WIDTH - 1, drawY + BLOCK_WIDTH - 1);
}

/*画出完整的蛇*/
void drawSnake() {
    // 遍历蛇的所有身体节点，逐个绘制
    for (int i = 0; i < 12; i++) {
        drawUnitBlock(snake[i][0], snake[i][1], i);
    }
}

int main() {
    // 初始化图形窗口
    initgraph(WIN_WIDTH, WIN_HEIGHT);
    // 设置窗口背景色
    setbkcolor(RGB(255, 250, 250));
    // 清空画布
    cleardevice();

    // 绘制完整的蛇
    drawSnake();

    // 等待按键按下
    _getch();
    // 关闭图形窗口
    closegraph();
    return 0;
}