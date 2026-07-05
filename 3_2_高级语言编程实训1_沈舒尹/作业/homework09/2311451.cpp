#include <graphics.h>
#include <conio.h>
#include <math.h>
#include <iostream>
#include <cstdio>
#include <stdlib.h>
using namespace std;

#define N 15                  // 棋盘大小
#define block_width 40        // 每个格子宽度
#define margin 50             // 棋盘边距
#define points 4              // 星位数量

int chessBox[N][N] = { 0 };   // 棋盘数组：0空/1黑/-1白
const double checkBound = block_width * 0.4; // 鼠标点击有效范围
int clickRow, clickCol;       // 有效点击的行列
int currentPlayer = 1;        // 当前玩家：1黑 / -1白
int total_count = 0;          // 总落子数

// 绘制游戏棋盘
void drawBoard() {
    // 计算棋盘尺寸
    int board_width = (N - 1) * block_width + 2 * margin;
    int board_height = (N - 1) * block_width + 2 * margin;

    // 设置棋盘背景色并清空画布
    int boardColor = RGB(255, 205, 150);
    setbkcolor(boardColor);
    cleardevice();

    // 绘制N*N网格线
    setlinecolor(BLACK);
    for (int i = 0; i < N; i++) {
        // 横线
        line(margin, margin + i * block_width,
            margin + (N - 1) * block_width, margin + i * block_width);
        // 竖线
        line(margin + i * block_width, margin,
            margin + i * block_width, margin + (N - 1) * block_width);
    }

    // 绘制4个星位
    setfillcolor(BLACK);
    solidcircle(margin + 3 * block_width, margin + 3 * block_width, 3);
    solidcircle(margin + 11 * block_width, margin + 3 * block_width, 3);
    solidcircle(margin + 3 * block_width, margin + 11 * block_width, 3);
    solidcircle(margin + 11 * block_width, margin + 11 * block_width, 3);

    // 绘制天元
    solidcircle(margin + 7 * block_width, margin + 7 * block_width, 4);

    // 绘制坐标数字和字母
    setbkmode(TRANSPARENT);
    settextcolor(BLACK);
    char num[15] = "", abc[15] = "";
    for (int i = 0; i < N; i++) {
        sprintf_s(num, "%d", i + 1);
        outtextxy(margin - 8 + block_width * i, margin - 25, num); // 数字
        sprintf_s(abc, "%c", i + 65);
        outtextxy(margin - 25, margin - 8 + block_width * i, abc); // 字母
    }
}

// 初始化游戏：创建画布、绘制棋盘、初始化棋盘数组
void initGame() {
    int board_width = (N - 1) * block_width + 2 * margin;
    int board_height = (N - 1) * block_width + 2 * margin;
    initgraph(board_width, board_height); // 创建绘图窗口
    drawBoard();                          // 绘制棋盘
    // 初始化棋盘数组（默认全0，无需额外操作）
}

// 绘制棋子：row行 col列 color（1黑/-1白）
void drawChess(int row, int col, int color) {
    if (row < 0 || row >= N || col < 0 || col >= N) return;

    int x = margin + col * block_width;
    int y = margin + row * block_width;

    if (color == 1) { // 黑棋
        setfillcolor(BLACK);
        solidcircle(x, y, block_width / 2 - 2);
    }
    else if (color == -1) { // 白棋
        setfillcolor(WHITE);
        solidcircle(x, y, block_width / 2 - 2);
        setlinecolor(BLACK);
        circle(x, y, block_width / 2 - 2); // 白棋加黑边
    }
}

// 检测鼠标点击是否有效：返回true=有效空位置
bool checkClick(ExMessage msg) {
    int x = msg.x;
    int y = msg.y;

    // 初步计算点击的格子
    int col = (x - margin) / block_width;
    int row = (y - margin) / block_width;

    // 边界过滤
    if (row < 0 || row >= N || col < 0 || col >= N)
        return false;

    // 计算格子四个角坐标
    int leftTopX = margin + col * block_width;
    int leftTopY = margin + row * block_width;

    // 计算点击点到四个角的距离
    double disLT = sqrt(pow(x - leftTopX, 2) + pow(y - leftTopY, 2));
    double disLB = sqrt(pow(x - leftTopX, 2) + pow(y - (leftTopY + block_width), 2));
    double disRT = sqrt(pow(x - (leftTopX + block_width), 2) + pow(y - leftTopY, 2));
    double disRB = sqrt(pow(x - (leftTopX + block_width), 2) + pow(y - (leftTopY + block_width), 2));

    // 初始化点击位置为无效
    clickRow = -1;
    clickCol = -1;

    // 判断点击是否在四个角的有效范围内
    if (disLT < checkBound) {
        clickRow = row;
        clickCol = col;
    }
    else if (disLB < checkBound) {
        clickRow = row + 1;
        clickCol = col;
    }
    else if (disRT < checkBound) {
        clickRow = row;
        clickCol = col + 1;
    }
    else if (disRB < checkBound) {
        clickRow = row + 1;
        clickCol = col + 1;
    }

    // 二次边界过滤
    if (clickRow < 0 || clickRow >= N || clickCol < 0 || clickCol >= N)
        return false;

    // 检查位置是否为空
    if (chessBox[clickRow][clickCol] == 0)
        return true;

    return false;
}

// 边界检查辅助函数：确保n在[0,N)范围内
int checkLimit(int n) {
    if (n < 0) return 0;
    if (n >= N) return N - 1;
    return n;
}

// 判断胜负：checkColor=当前落子颜色（1/-1），返回true=获胜
bool judgeWin(int checkColor) {
    int i = clickRow;
    int j = clickCol;
    int chess_Count = 0;

    // 横向（x轴）检查
    chess_Count = 0;
    for (j = 0; j < N; j++) {
        if (chessBox[i][j] == checkColor) {
            chess_Count++;
            if (chess_Count >= 5) return true;
        }
        else {
            chess_Count = 0;
        }
    }

    // 纵向（y轴）检查
    chess_Count = 0;
    for (i = 0; i < N; i++) {
        if (chessBox[i][clickCol] == checkColor) {
            chess_Count++;
            if (chess_Count >= 5) return true;
        }
        else {
            chess_Count = 0;
        }
    }

    // 正斜线（y=x）检查
    chess_Count = 0;
    // 先找到斜线起点
    int startRow = clickRow - min(clickRow, clickCol);
    int startCol = clickCol - min(clickRow, clickCol);
    for (int k = 0; startRow + k < N && startCol + k < N; k++) {
        if (chessBox[startRow + k][startCol + k] == checkColor) {
            chess_Count++;
            if (chess_Count >= 5) return true;
        }
        else {
            chess_Count = 0;
        }
    }

    // 反斜线（y=-x）检查
    chess_Count = 0;
    // 先找到斜线起点
    startRow = clickRow + min(N - 1 - clickRow, clickCol);
    startCol = clickCol - min(N - 1 - clickRow, clickCol);
    for (int k = 0; startRow - k >= 0 && startCol + k < N; k++) {
        if (chessBox[startRow - k][startCol + k] == checkColor) {
            chess_Count++;
            if (chess_Count >= 5) return true;
        }
        else {
            chess_Count = 0;
        }
    }

    return false;
}

// 玩家落子逻辑：绘制棋子、更新棋盘、切换玩家
void human_player() {
    drawChess(clickRow, clickCol, currentPlayer); // 绘制棋子
    chessBox[clickRow][clickCol] = currentPlayer; // 更新棋盘数组
    total_count++;                                // 落子数+1
    currentPlayer = (currentPlayer == 1) ? -1 : 1;// 切换玩家
}

int main() {
    initGame(); // 初始化游戏
    ExMessage m;

    while (1) {
        m = getmessage(EM_MOUSE | EM_KEY); // 监听鼠标+键盘事件

        // 鼠标左键点击且落子有效
        if (m.message == WM_LBUTTONDOWN && checkClick(m)) {
            human_player(); // 玩家落子

            // 判断胜负
            if (judgeWin(chessBox[clickRow][clickCol])) {
                const char* winner = (currentPlayer == -1) ? "黑方" : "白方";
                MessageBox(GetHWnd(), (string(winner) + "获胜！").c_str(), "游戏结束", MB_OK);
                closegraph(); // 关闭绘图窗口
                exit(0);      // 退出程序
            }

            // 平局判断（棋盘下满）
            if (total_count >= N * N) {
                MessageBox(GetHWnd(), "棋盘已满，平局！", "游戏结束", MB_OK);
                closegraph();
                exit(0);
            }
        }

        // 按ESC键退出游戏
        if (m.message == WM_KEYDOWN && m.vkcode == VK_ESCAPE) {
            closegraph();
            exit(0);
        }
    }

    _getch();
    closegraph();
    return 0;
}