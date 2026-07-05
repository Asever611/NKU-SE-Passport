#include <graphics.h>
#include <conio.h>
#include <math.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <time.h>
using namespace std;

#define N 15
#define block_width 40
#define margin 50
#define DEPTH 4  // 搜索深度

int chessBox[N][N] = { 0 };
double checkBound = block_width * 0.4;
int clickRow, clickCol;
int aiRow, aiCol;
int HumanPlayer = 1;      // 玩家执黑
int AIPlayer = -1;        // AI执白
int currentPlayer;
int total_count = 0;
bool gameOver = false;

// 位置权重表
static int scoreMap[N][N] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,1,2,2,2,2,2,2,2,2,2,2,2,1,0},
    {0,1,2,3,3,3,3,3,3,3,3,3,2,1,0},
    {0,1,2,3,4,4,4,4,4,4,4,3,2,1,0},
    {0,1,2,3,4,5,5,5,5,5,4,3,2,1,0},
    {0,1,2,3,4,5,6,6,6,5,4,3,2,1,0},
    {0,1,2,3,4,5,6,7,6,5,4,3,2,1,0},
    {0,1,2,3,4,5,6,6,6,5,4,3,2,1,0},
    {0,1,2,3,4,5,5,5,5,5,4,3,2,1,0},
    {0,1,2,3,4,4,4,4,4,4,4,3,2,1,0},
    {0,1,2,3,3,3,3,3,3,3,3,3,2,1,0},
    {0,1,2,2,2,2,2,2,2,2,2,2,2,1,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

struct maxScorePoint {
    int row;
    int col;
};

// 函数声明
void drawBoard();
void initGame();
bool checkClick();
bool judgeWin(int row, int col, int color);
void drawChess(int row, int col, int color);
void human_play();
void update_chessBox();
int evaluate(int row, int col, int color);
int MaxValue(int row, int col, int depth);
int MinValue(int row, int col, int depth);
maxScorePoint getMinMaxValue(int depth);
void AI_player();

// 绘制棋盘
void drawBoard() {
    int winW = (N - 1) * block_width + margin * 2;
    int winH = (N - 1) * block_width + margin * 2;
    initgraph(winW, winH);
    HWND hwnd = GetHWnd();
    SetWindowText(hwnd, L"五子棋 MinMax AI");
    setbkcolor(RGB(255, 205, 150));
    cleardevice();

    setlinecolor(BLACK);
    for (int i = 0; i < N; i++) {
        line(margin, margin + i * block_width,
            margin + (N - 1) * block_width, margin + i * block_width);
        line(margin + i * block_width, margin,
            margin + i * block_width, margin + (N - 1) * block_width);
    }

    setfillcolor(BLACK);
    solidcircle(margin + 3 * block_width, margin + 3 * block_width, 3);
    solidcircle(margin + 11 * block_width, margin + 3 * block_width, 3);
    solidcircle(margin + 3 * block_width, margin + 11 * block_width, 3);
    solidcircle(margin + 11 * block_width, margin + 11 * block_width, 3);
    solidcircle(margin + 7 * block_width, margin + 7 * block_width, 4);
}

// 初始化游戏
void initGame() {
    memset(chessBox, 0, sizeof(chessBox));
    total_count = 0;
    gameOver = false;
    currentPlayer = HumanPlayer;  // 玩家先手
    drawBoard();
}

// 检查鼠标点击
bool checkClick() {
    ExMessage msg;
    while (true) {
        msg = getmessage(EM_MOUSE);
        if (msg.message == WM_LBUTTONDOWN) {
            int col = (msg.x - margin + block_width / 2) / block_width;
            int row = (msg.y - margin + block_width / 2) / block_width;
            if (row >= 0 && row < N && col >= 0 && col < N && chessBox[row][col] == 0) {
                clickRow = row;
                clickCol = col;
                return true;
            }
        }
    }
}

// 判断胜负
bool judgeWin(int row, int col, int color) {
    int dir[4][2] = { {0,1},{1,0},{1,1},{1,-1} };
    for (int k = 0; k < 4; k++) {
        int count = 1;
        for (int i = 1; i <= 4; i++) {
            int r = row + dir[k][0] * i;
            int c = col + dir[k][1] * i;
            if (r >= 0 && r < N && c >= 0 && c < N && chessBox[r][c] == color) count++;
            else break;
        }
        for (int i = 1; i <= 4; i++) {
            int r = row - dir[k][0] * i;
            int c = col - dir[k][1] * i;
            if (r >= 0 && r < N && c >= 0 && c < N && chessBox[r][c] == color) count++;
            else break;
        }
        if (count >= 5) return true;
    }
    return false;
}

// 绘制棋子
void drawChess(int row, int col, int color) {
    int x = margin + col * block_width;
    int y = margin + row * block_width;
    if (color == 1) {
        setfillcolor(BLACK);
        solidcircle(x, y, block_width / 2 - 2);
    }
    else if (color == -1) {
        setfillcolor(WHITE);
        solidcircle(x, y, block_width / 2 - 2);
        setlinecolor(BLACK);
        circle(x, y, block_width / 2 - 2);
    }
}

// 玩家回合
void human_play() {
    checkClick();
    chessBox[clickRow][clickCol] = HumanPlayer;
    drawChess(clickRow, clickCol, HumanPlayer);
    total_count++;

    if (judgeWin(clickRow, clickCol, HumanPlayer)) {
        MessageBox(GetHWnd(), L"玩家获胜！", L"游戏结束", MB_OK);
        gameOver = true;
        return;
    }
    if (total_count >= N * N) {
        MessageBox(GetHWnd(), L"平局！", L"游戏结束", MB_OK);
        gameOver = true;
        return;
    }
    currentPlayer = AIPlayer;
}

// AI落子后更新棋盘
void update_chessBox() {
    chessBox[aiRow][aiCol] = AIPlayer;
    drawChess(aiRow, aiCol, AIPlayer);
    total_count++;

    if (judgeWin(aiRow, aiCol, AIPlayer)) {
        MessageBox(GetHWnd(), L"AI获胜！", L"游戏结束", MB_OK);
        gameOver = true;
        return;
    }
    if (total_count >= N * N) {
        MessageBox(GetHWnd(), L"平局！", L"游戏结束", MB_OK);
        gameOver = true;
        return;
    }
    currentPlayer = HumanPlayer;
}

// 评估函数：计算在(row, col)位置落下color棋子后该点的价值
int evaluate(int row, int col, int color) {
    int score = 0;
    int dir[4][2] = { {0, 1}, {1, 0}, {1, 1}, {1, -1} };

    // 确定起始连子数：若该点已经是color则从1开始，否则从0开始（用于空点预评估）
    int startCnt = (chessBox[row][col] == color) ? 1 : 0;

    for (int k = 0; k < 4; k++) {
        int cnt = startCnt;
        int empty = 0;

        // 正方向扫描
        for (int i = 1; i <= 4; i++) {
            int r = row + dir[k][0] * i;
            int c = col + dir[k][1] * i;
            if (r < 0 || r >= N || c < 0 || c >= N) break;
            if (chessBox[r][c] == color) cnt++;
            else if (chessBox[r][c] == 0) { empty++; break; }
            else break;
        }
        // 反方向扫描
        for (int i = 1; i <= 4; i++) {
            int r = row - dir[k][0] * i;
            int c = col - dir[k][1] * i;
            if (r < 0 || r >= N || c < 0 || c >= N) break;
            if (chessBox[r][c] == color) cnt++;
            else if (chessBox[r][c] == 0) { empty++; break; }
            else break;
        }

        // 根据棋型给分
        if (cnt >= 5) score += 100000;
        else if (cnt == 4 && empty == 2) score += 10000;
        else if (cnt == 4 && empty == 1) score += 1000;
        else if (cnt == 3 && empty == 2) score += 500;
        else if (cnt == 3 && empty == 1) score += 100;
        else if (cnt == 2 && empty == 2) score += 50;
        else if (cnt == 2 && empty == 1) score += 10;
        else score += 1;
    }

    score += scoreMap[row][col];
    return score;
}

// Max层：AI走棋
int MaxValue(int row, int col, int depth) {
    if (depth <= 0) {
        return evaluate(row, col, AIPlayer) - evaluate(row, col, HumanPlayer);
    }
    int maxValue = INT_MIN;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (chessBox[i][j] == 0) {
                chessBox[i][j] = AIPlayer;
                int score = MinValue(i, j, depth - 1);
                if (score > maxValue) maxValue = score;
                chessBox[i][j] = 0;
            }
        }
    }
    return maxValue;
}

// Min层：玩家走棋
int MinValue(int row, int col, int depth) {
    if (depth <= 0) {
        return evaluate(row, col, AIPlayer) - evaluate(row, col, HumanPlayer);
    }
    int minValue = INT_MAX;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (chessBox[i][j] == 0) {
                chessBox[i][j] = HumanPlayer;
                int score = MaxValue(i, j, depth - 1);
                if (score < minValue) minValue = score;
                chessBox[i][j] = 0;
            }
        }
    }
    return minValue;
}

// 获取MinMax最佳落子
maxScorePoint getMinMaxValue(int depth) {
    maxScorePoint bestPoint[N * N] = { 0 };
    int maxValue = INT_MIN;
    int k = 0;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (chessBox[i][j] == 0) {
                chessBox[i][j] = AIPlayer;
                int score = MinValue(i, j, depth - 1);
                if (score == maxValue) {
                    bestPoint[k].row = i;
                    bestPoint[k].col = j;
                    k++;
                }
                else if (score > maxValue) {
                    maxValue = score;
                    k = 0;
                    memset(bestPoint, 0, sizeof(bestPoint));
                    bestPoint[k].row = i;
                    bestPoint[k].col = j;
                    k++;
                }
                chessBox[i][j] = 0;
            }
        }
    }

    int index = rand() % k;
    return bestPoint[index];
}

// AI走棋总控
void AI_player() {
    // 第一手黑棋下天元
    if (total_count == 0 && AIPlayer == 1) {
        aiRow = 7; aiCol = 7;
        update_chessBox();
        return;
    }

    // 1. AI能否直接获胜
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (chessBox[i][j] == 0) {
                chessBox[i][j] = AIPlayer;
                if (judgeWin(i, j, AIPlayer)) {
                    aiRow = i; aiCol = j;
                    chessBox[i][j] = 0;  // 恢复，实际落子由update完成
                    update_chessBox();
                    return;
                }
                chessBox[i][j] = 0;
            }
        }
    }

    // 2. 拦截玩家获胜
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (chessBox[i][j] == 0) {
                chessBox[i][j] = HumanPlayer;
                if (judgeWin(i, j, HumanPlayer)) {
                    aiRow = i; aiCol = j;
                    chessBox[i][j] = 0;
                    update_chessBox();
                    return;
                }
                chessBox[i][j] = 0;
            }
        }
    }

    // 3. MinMax搜索
    maxScorePoint AIPoint = getMinMaxValue(DEPTH);
    aiRow = AIPoint.row;
    aiCol = AIPoint.col;
    update_chessBox();
}

// 主函数
int main() {
    srand((unsigned int)time(NULL));
    initGame();

    while (!gameOver) {
        if (currentPlayer == HumanPlayer) {
            human_play();
        }
        else {
            AI_player();
        }
    }

    _getch();
    closegraph();
    return 0;
}