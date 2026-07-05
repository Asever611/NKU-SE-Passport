#include <graphics.h>
#include <conio.h>
#include <windows.h>
#include <iostream>
#include <cstring>
using namespace std;

// 棋盘基础定义
#define N 15                  // 15路棋盘
#define block_width 40        // 棋格宽度
#define margin 50             // 棋盘边距

// 全局变量定义
int chessBox[N][N] = { 0 };     // 棋盘状态：0=空，1=玩家黑棋，-1=AI白棋
int humanScore[N][N] = { 0 };   // 玩家每个空点的评估分数
int aiScore[N][N] = { 0 };      // AI每个空点的评估分数
int humanMaxScore = 0;        // 玩家的最高评估分数
int aiMaxScore = 0;           // AI的最高评估分数
int aiRow = 7, aiCol = 7;     // AI落子位置
int HumanPlayer = 1;          // 玩家标识
int AIPlayer = -1;            // AI标识
int totalChess = 0;           // 总落子数

// 位置权重表
int scoreMap[N][N] = {
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

// 计算单个位置的棋型分数
int evaluate(int row, int col, int color) {
    if (chessBox[row][col] != 0) return 0; // 非空位置无分数
    int score = 0;
    int dir[4][2] = { {0, 1}, {1, 0}, {1, 1}, {1, -1} }; // 横、竖、正斜、反斜四个方向

    for (int k = 0; k < 4; k++) {
        int cnt = 0;    // 连续同色棋子数
        int empty = 0;  // 两端空位数量（0/1/2）

        // 正向扫描（当前方向的前方）
        for (int i = 1; i <= 4; i++) {
            int r = row + dir[k][0] * i;
            int c = col + dir[k][1] * i;
            if (r < 0 || r >= N || c < 0 || c >= N) break;
            if (chessBox[r][c] == color) cnt++;
            else if (chessBox[r][c] == 0) { empty++; break; }
            else break; // 遇到敌方棋子，终止扫描
        }

        // 反向扫描（当前方向的后方）
        for (int i = 1; i <= 4; i++) {
            int r = row - dir[k][0] * i;
            int c = col - dir[k][1] * i;
            if (r < 0 || r >= N || c < 0 || c >= N) break;
            if (chessBox[r][c] == color) cnt++;
            else if (chessBox[r][c] == 0) { empty++; break; }
            else break;
        }

        // 根据棋型匹配分数
        if (cnt >= 5) score += 100000;    // 连五
        else if (cnt == 4 && empty == 2) score += 10000; // 活四
        else if (cnt == 4 && empty == 1) score += 1000;  // 冲四
        else if (cnt == 3 && empty == 2) score += 500;   // 活三
        else if (cnt == 3 && empty == 1) score += 100;   // 眠三
        else if (cnt == 2 && empty == 2) score += 50;    // 活二
        else if (cnt == 2 && empty == 1) score += 10;   // 眠二
        else score += 1; // 单子基础分
    }

    // 叠加位置权重
    score += scoreMap[row][col];
    return score;
}

// 计算双方最高分并决策AI落子
void calculateScore() {
    // 初始化最高分和对应位置
    humanMaxScore = -1;
    aiMaxScore = -1;
    int bestHumanRow = 0, bestHumanCol = 0;
    int bestAiRow = 0, bestAiCol = 0;

    // 遍历棋盘所有空点，计算双方分数
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (chessBox[i][j] == 0) {
                // 计算玩家和AI在该位置的分数
                humanScore[i][j] = evaluate(i, j, HumanPlayer);
                aiScore[i][j] = evaluate(i, j, AIPlayer);

                // 更新玩家的最高分和位置
                if (humanScore[i][j] > humanMaxScore) {
                    humanMaxScore = humanScore[i][j];
                    bestHumanRow = i;
                    bestHumanCol = j;
                }

                // 更新AI的最高分和位置
                if (aiScore[i][j] > aiMaxScore) {
                    aiMaxScore = aiScore[i][j];
                    bestAiRow = i;
                    bestAiCol = j;
                }
            }
        }
    }

    // 进攻优先，防守次之
    if (aiMaxScore >= humanMaxScore) {
        // AI进攻：选择自己分数最高的位置
        aiRow = bestAiRow;
        aiCol = bestAiCol;
    }
    else {
        // AI防守：拦截玩家分数最高的位置
        aiRow = bestHumanRow;
        aiCol = bestHumanCol;
    }
}

// 绘制棋盘
void drawBoard() {
    int winW = (N - 1) * block_width + margin * 2;
    int winH = (N - 1) * block_width + margin * 2;
    initgraph(winW, winH);
    HWND hwnd = GetHWnd();
    SetWindowText(hwnd, L"智能五子棋");
    setbkcolor(RGB(255, 205, 150));
    cleardevice();

    // 绘制网格线
    setlinecolor(BLACK);
    for (int i = 0; i < N; i++) {
        line(margin, margin + i * block_width, margin + (N - 1) * block_width, margin + i * block_width);
        line(margin + i * block_width, margin, margin + i * block_width, margin + (N - 1) * block_width);
    }

    // 绘制星位
    setfillcolor(BLACK);
    solidcircle(margin + 3 * block_width, margin + 3 * block_width, 3);
    solidcircle(margin + 11 * block_width, margin + 3 * block_width, 3);
    solidcircle(margin + 3 * block_width, margin + 11 * block_width, 3);
    solidcircle(margin + 11 * block_width, margin + 11 * block_width, 3);
    solidcircle(margin + 7 * block_width, margin + 7 * block_width, 4);
}

// 绘制棋子
void drawChess(int row, int col, int color) {
    int x = margin + col * block_width;
    int y = margin + row * block_width;
    if (color == 1) { // 黑棋（玩家）
        setfillcolor(BLACK);
        solidcircle(x, y, block_width / 2 - 2);
    }
    else if (color == -1) { // 白棋（AI）
        setfillcolor(WHITE);
        solidcircle(x, y, block_width / 2 - 2);
        setlinecolor(BLACK);
        circle(x, y, block_width / 2 - 2);
    }
}

// 鼠标点击落子
bool checkClick(int& clickRow, int& clickCol) {
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

// 胜负判定
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

// 主函数
int main() {
    // 初始化游戏
    memset(chessBox, 0, sizeof(chessBox));
    totalChess = 0;
    drawBoard();
    int currentPlayer = HumanPlayer;
    int clickRow, clickCol;

    while (true) {
        if (currentPlayer == HumanPlayer) {
            // 玩家回合：鼠标落子
            checkClick(clickRow, clickCol);
            chessBox[clickRow][clickCol] = HumanPlayer;
            drawChess(clickRow, clickCol, HumanPlayer);
            totalChess++;

            // 判定玩家胜利
            if (judgeWin(clickRow, clickCol, HumanPlayer)) {
                MessageBox(GetHWnd(), L"玩家获胜！", L"游戏结束", MB_OK);
                break;
            }
            currentPlayer = AIPlayer;
        }
        else {
            // AI回合：调用calculateScore()决策
            if (totalChess == 0) {
                // AI先手直接下天元
                aiRow = 7; aiCol = 7;
            }
            else {
                calculateScore();
            }

            // AI落子
            chessBox[aiRow][aiCol] = AIPlayer;
            drawChess(aiRow, aiCol, AIPlayer);
            totalChess++;

            // 判定AI胜利
            if (judgeWin(aiRow, aiCol, AIPlayer)) {
                MessageBox(GetHWnd(), L"AI获胜！", L"游戏结束", MB_OK);
                break;
            }
            currentPlayer = HumanPlayer;
        }

        // 平局判定
        if (totalChess >= N * N) {
            MessageBox(GetHWnd(), L"平局！", L"游戏结束", MB_OK);
            break;
        }
    }

    closegraph();
    return 0;
}