#include <graphics.h>
#include <conio.h>
#include <math.h>
#include <iostream>
using namespace std;

// 棋盘参数
#define N 15
#define block_width 40    // 每个格子宽度
#define margin 50         // 边距
#define board_width  (N-1)*block_width + 2*margin
#define board_height (N-1)*block_width + 2*margin

// 棋盘数据：0=空 1=黑棋 2=白棋
int chessBox[N][N] = { 0 };
double checkBound = block_width * 0.4; // 点击有效范围
int clickRow, clickCol;                // 有效点击位置

// 棋子颜色枚举
enum ChessKind {
    CHESS_EMPTY = 0,
    CHESS_BLACK = 1,
    CHESS_WHITE = 2
};

// 初始化游戏
void init() {
    initgraph(board_width, board_height); // 创建窗口
    setbkcolor(RGB(255, 205, 150));       // 棋盘底色
    cleardevice();

    // 画N*N网格线
    setlinecolor(BLACK);
    for (int i = 0; i < N; i++) {
        // 横线
        line(margin, margin + i * block_width,
            margin + (N - 1) * block_width, margin + i * block_width);
        // 竖线
        line(margin + i * block_width, margin,
            margin + i * block_width, margin + (N - 1) * block_width);
    }

    // 画四星+天元（15路标准点位）
    setfillcolor(BLACK);
    solidcircle(margin + 3 * block_width, margin + 3 * block_width, 3);
    solidcircle(margin + 11 * block_width, margin + 3 * block_width, 3);
    solidcircle(margin + 3 * block_width, margin + 11 * block_width, 3);
    solidcircle(margin + 11 * block_width, margin + 11 * block_width, 3);
    solidcircle(margin + 7 * block_width, margin + 7 * block_width, 4);

    // 画数字坐标（上侧）
    setbkmode(TRANSPARENT);
    settextcolor(BLACK);
    for (int i = 0; i < N; i++) {
        char num[5];
        sprintf_s(num, "%d", i + 1);
        outtextxy(margin + i * block_width - 8, margin - 25, num);
    }

    // 画字母坐标（左侧）
    char colAlpha[] = "ABCDEFGHIJKLMNO";
    for (int i = 0; i < N; i++) {
        outtextxy(margin - 25, margin + i * block_width - 8, colAlpha[i]);
    }
}

// 画棋子
void drawChess(int row, int col, ChessKind color) {
    if (row < 0 || row >= N || col < 0 || col >= N) return;

    int x = margin + col * block_width;
    int y = margin + row * block_width;

    if (color == CHESS_BLACK) {
        setfillcolor(BLACK);
        solidcircle(x, y, block_width / 2 - 2);
    }
    else if (color == CHESS_WHITE) {
        setfillcolor(WHITE);
        solidcircle(x, y, block_width / 2 - 2);
        setlinecolor(BLACK);
        circle(x, y, block_width / 2 - 2);
    }
}

// 检查点击是否有效
bool checkClick(ExMessage msg) {
    int x = msg.x;
    int y = msg.y;

    // 计算落在哪个格子
    int col = (x - margin) / block_width;
    int row = (y - margin) / block_width;

    // 越界直接返回
    if (row < 0 || row >= N || col < 0 || col >= N)
        return false;

    // 四个角坐标
    int leftTopX = margin + col * block_width;
    int leftTopY = margin + row * block_width;

    // 计算到四个交叉点的距离
    double disLT = sqrt(pow(x - leftTopX, 2) + pow(y - leftTopY, 2));
    double disLB = sqrt(pow(x - leftTopX, 2) + pow(y - (leftTopY + block_width), 2));
    double disRT = sqrt(pow(x - (leftTopX + block_width), 2) + pow(y - leftTopY, 2));
    double disRB = sqrt(pow(x - (leftTopX + block_width), 2) + pow(y - (leftTopY + block_width), 2));

    // 判断是否靠近交叉点
    clickRow = -1;
    clickCol = -1;

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

    // 再次越界判断
    if (clickRow < 0 || clickRow >= N || clickCol < 0 || clickCol >= N)
        return false;

    // 判断是否为空位
    if (chessBox[clickRow][clickCol] == CHESS_EMPTY)
        return true;

    return false;
}

// 测试函数：自动铺满黑白棋
void test() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            chessBox[i][j] = (i + j) % 2 + 1;
            drawChess(i, j, (ChessKind)chessBox[i][j]);
        }
    }
}

int main() {
    init();
    // test();

    int currentPlayer = CHESS_BLACK; // 黑先下

    while (1) {
        ExMessage m;
        m = getmessage(EM_MOUSE);

        if (m.message == WM_LBUTTONDOWN && checkClick(m)) {
            // 落子
            chessBox[clickRow][clickCol] = currentPlayer;
            drawChess(clickRow, clickCol, (ChessKind)currentPlayer);

            // 切换玩家
            currentPlayer = (currentPlayer == CHESS_BLACK) ? CHESS_WHITE : CHESS_BLACK;
        }
    }

    _getch();
    closegraph();
    return 0;
}