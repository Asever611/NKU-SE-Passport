#include <graphics.h>
#include <conio.h>
#include <stdio.h>
#include <windows.h>
#include <string>
using namespace std;

#define WINDOW_WIDTH 1000   // 窗口宽度
#define WINDOW_HEIGHT 640   // 窗口高度
#define x_origin 0          // x轴起始坐标
#define y_origin 0          // y轴起始坐标
#define blockWidth 20       // 蛇身方块宽度
#define initSnakeLen 12     // 初始长度
#define initSpeed 200       // 初始速度

// 方向定义
#define DIR_UP    1
#define DIR_DOWN  2
#define DIR_LEFT  3
#define DIR_RIGHT 4

int toDir = DIR_RIGHT;  // 初始方向向右
int snakeLen = initSnakeLen;    // 蛇长
IMAGE bgImg;            // 背景图片对象
LPCTSTR bgImgPath = "D:\\0\\课程\\3_2_高级语言编程实训1\\作业\\homework03\\bg_grassland.jpg";  // 背景图地址


// 蛇身体坐标
int snake[initSnakeLen][2] = {
    {14,11}, {13,11}, {13,10}, {12,10},
    {11,10}, {11,9},  {11,8},  {11,7},
    {10,7},  {9,7},   {8,7},   {7,7}
};

/* 绘制背景 */
void drawBackground() {
    // 加载背景图片
    loadimage(&bgImg, bgImgPath, WINDOW_WIDTH, WINDOW_HEIGHT);
    // 绘制背景图
    putimage(x_origin, y_origin, &bgImg);

    // 绘制游戏边框
    setlinecolor(RGB(80, 80, 120));
    setlinestyle(PS_SOLID, 2);  // 实线，宽度2像素
    rectangle(x_origin, y_origin, WINDOW_WIDTH, WINDOW_HEIGHT);
}

/* 绘制单个方块 */
void drawUnitBlock(int x, int y, int segment) {
    // 计算方块像素坐标
    int left = x_origin + x * blockWidth;
    int top = y_origin + y * blockWidth;
    int right = left + blockWidth - 1;
    int bottom = top + blockWidth - 1;

    // 设置蛇身颜色
    COLORREF fillColor, lineColor;
    if (segment == 0) {
        fillColor = RGB(255, 150, 150);
        lineColor = RGB(255, 140, 140);
    }
    else {
        // 蛇身：粉色渐变
        int g = 180 + (segment * 60) / (snakeLen - 1);
        int b = 180 + (segment * 60) / (snakeLen - 1);
        fillColor = RGB(255, g, b);
        // 边框颜色：比填充色略深
        lineColor = RGB(255, g - 10, b - 10);
    }

    // 设置蛇身颜色
    setfillcolor(fillColor);
    setlinecolor(lineColor);
    fillrectangle(left, top, right, bottom);
}

/* 绘制整条蛇 */
void drawSnake() {
    for (int i = 0; i < snakeLen; i++) {
        drawUnitBlock(snake[i][0], snake[i][1], i);
    }
}

/* 移动蛇 */
void moveSnake() {
    // 从尾往前复制
    for (int i = snakeLen - 1; i > 0; i--) {
        snake[i][0] = snake[i - 1][0];
        snake[i][1] = snake[i - 1][1];
    }

    // 蛇头移动
    switch (toDir) {
    case DIR_UP:    snake[0][1]--; break;
    case DIR_DOWN:  snake[0][1]++; break;
    case DIR_LEFT:  snake[0][0]--; break;
    case DIR_RIGHT: snake[0][0]++; break;
    }
}

/* 控制台消息 */
void printArray() {
    printf("========== 蛇身坐标 ==========\n");
    for (int i = 0; i < snakeLen; i++) {
        printf("(%2d,%2d) ", snake[i][0], snake[i][1]);
        if ((i + 1) % 4 == 0 || i == snakeLen - 1) printf("\n");
    }
    printf("\n方向：");
    if (toDir == DIR_UP) printf("上");
    else if (toDir == DIR_DOWN) printf("下");
    else if (toDir == DIR_LEFT) printf("左");
    else printf("右");
    printf("\n\n");
}

/* 自动移动 */
void autoMove() {
    moveSnake();        // 更新坐标
    drawBackground();   // 绘制背景
    drawSnake();        // 绘制新蛇
    Sleep(initSpeed);  // 速度控制
}

// ===================== 主函数 =====================
int main(void) {
    // 打开图形窗口 + 控制台
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT, EX_SHOWCONSOLE);

    // 初始画面
    drawBackground();
    drawSnake();
    printArray();

    // 游戏循环
    while (1) {
        if (_kbhit()) {
            char input = _getch();

            // WASD 控制
            if (input == 'w') toDir = DIR_UP;
            else if (input == 's') toDir = DIR_DOWN;
            else if (input == 'a') toDir = DIR_LEFT;
            else if (input == 'd') toDir = DIR_RIGHT;
            else if (input == 'q') break; // 退出

            // 按键后立即移动刷新
            autoMove();
            printArray();
        }
        // 无按键自动走
        else {
            autoMove();
        }
    }

    closegraph();
    return 0;
}