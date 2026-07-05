#include <graphics.h>   
#include <conio.h>      
#include <math.h>       
#include <iostream>    
#include <cstdlib>      
#include <cstring>      
#include <time.h>       
#include <windows.h>    
using namespace std;

#define N 15                  // 棋盘行列数
#define block_width 40        // 网格间距
#define margin 50             // 边距
#define DEPTH 4               // α-β 搜索深度

const int endButtonWidth = 140;
const int endButtonHeight = 30;
int chessBox[N][N] = { 0 };     // 棋盘数据：0 空位，1 黑子，-1 白子
int clickRow, clickCol;         // 玩家点击的落子位置
int aiRow, aiCol;               // AI 选择的落子位置
int HumanPlayer = 1;            // 玩家与 AI 的棋子颜色（1 黑，-1 白）
int AIPlayer = -1;
int currentPlayer;              // 当前行棋方
int total_count = 0;            // 已落子总数
bool gameOver = false;          // 游戏是否结束
bool exitRequested = false;     // 是否请求退出程序
bool forceDrawRequested = false;// 是否请求强制平局
bool humanIsBlack = true;       // 玩家是否执黑

int boardPixelSize = 0;   
int winW = 0;             
int winH = 0;             
int endButtonX = 0;       
int endButtonY = 0;       
bool windowInited = false;

// AI 思考计时
bool aiThinking = false;
clock_t aiStartTime = 0;
double aiElapsedSeconds = 0.0;
double lastAiElapsedSeconds = -1.0;

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

// 记录最佳落子点
struct maxScorePoint { int row; int col; };

// 对话框按钮
enum DialogAction {
    DialogStart = 1001,
    DialogSwitch = 1002,
    DialogExit = 1003,
    DialogNone = 0
};

// 函数声明
void drawBoard();
void initGame();
bool checkClick();
bool judgeWin(int row, int col, int color);
void drawChess(int row, int col, int color);
void human_play();
void update_chessBox();
int  evaluatePoint(int row, int col, int color);    // 单点棋型评估
int  evaluateGlobal();                              // 高速全局评估
int  MaxValue(int depth, int alpha, int beta);      // 极大层
int  MinValue(int depth, int alpha, int beta);      // 极小层
maxScorePoint getMinMaxValue(int depth, int alpha, int beta);
void AI_player();
void updatePlayersByRole();
void drawStatusBar();
void updateAiThinkingProgress(int processed, int total);
void beginAiThinking();
void endAiThinking();
bool isEndButtonClick(int x, int y);
bool pollEndButtonClick();
DialogAction showActionDialog(const wchar_t* title, const wchar_t* content);
bool runStartDialog();
bool handleGameOver(const wchar_t* resultText);
void drawCenteredText(const RECT& rect, const wchar_t* text, int fontSize);

void drawBoard() {
    // 计算棋盘与窗口尺寸
    boardPixelSize = (N - 1) * block_width + margin * 2;
    winW = boardPixelSize;
    winH = boardPixelSize + 20;
    endButtonX = winW - margin - endButtonWidth;
    endButtonY = margin + (N - 1) * block_width + (margin - endButtonHeight) / 2 + 20;

    // 首次进入时初始化窗口
    if (!windowInited) {
        initgraph(winW, winH);
        HWND hwnd = GetHWnd();
        SetWindowText(hwnd, L"五子棋 α-β");
        windowInited = true;
    }
    // 背景色与清屏
    setbkcolor(RGB(255, 205, 150));
    cleardevice();

    // 绘制棋盘网格
    setlinecolor(BLACK);
    for (int i = 0; i < N; i++) {
        line(margin, margin + i * block_width,
            margin + (N - 1) * block_width, margin + i * block_width);
        line(margin + i * block_width, margin,
            margin + i * block_width, margin + (N - 1) * block_width);
    }

    // 绘制棋盘星位
    setfillcolor(BLACK);
    solidcircle(margin + 3 * block_width, margin + 3 * block_width, 3);
    solidcircle(margin + 11 * block_width, margin + 3 * block_width, 3);
    solidcircle(margin + 3 * block_width, margin + 11 * block_width, 3);
    solidcircle(margin + 11 * block_width, margin + 11 * block_width, 3);
    solidcircle(margin + 7 * block_width, margin + 7 * block_width, 4);

    // 绘制底部状态栏
    drawStatusBar();
}

void initGame() {
    // 清空棋盘与重置状态
    memset(chessBox, 0, sizeof(chessBox));
    total_count = 0;
    gameOver = false;
    forceDrawRequested = false;
    aiThinking = false;
    aiElapsedSeconds = 0.0;
    lastAiElapsedSeconds = -1.0;
    currentPlayer = 1;
    drawBoard();
}

bool checkClick() {
    // 阻塞式等待鼠标点击，获取合法落子点
    ExMessage msg;
    while (true) {
        msg = getmessage(EM_MOUSE);
        if (msg.message == WM_LBUTTONDOWN) {
            // 点击结束按钮 -> 请求平局
            if (isEndButtonClick(msg.x, msg.y)) {
                forceDrawRequested = true;
                return false;
            }
            // 坐标转换为行列索引
            int col = (msg.x - margin + block_width / 2) / block_width;
            int row = (msg.y - margin + block_width / 2) / block_width;
            // 判断是否在棋盘内且为空
            if (row >= 0 && row < N && col >= 0 && col < N && chessBox[row][col] == 0) {
                clickRow = row;
                clickCol = col;
                return true;
            }
        }
    }
}

bool judgeWin(int row, int col, int color) {
    // 判断某一步是否形成五连
    int dir[4][2] = { {0,1},{1,0},{1,1},{1,-1} };
    for (int k = 0; k < 4; k++) {
        int count = 1;
        // 正方向计数
        for (int i = 1; i <= 4; i++) {
            int r = row + dir[k][0] * i;
            int c = col + dir[k][1] * i;
            if (r >= 0 && r < N && c >= 0 && c < N && chessBox[r][c] == color) count++;
            else break;
        }
        // 反方向计数
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

void drawChess(int row, int col, int color) {
    // 根据行列位置绘制黑/白棋子
    int x = margin + col * block_width;
    int y = margin + row * block_width;
    if (color == 1) {
        setfillcolor(BLACK);
        solidcircle(x, y, block_width / 2 - 2);
    }
    else {
        setfillcolor(WHITE);
        solidcircle(x, y, block_width / 2 - 2);
        setlinecolor(BLACK);
        circle(x, y, block_width / 2 - 2);
    }
}

void human_play() {
    // 玩家回合：点击落子
    if (!checkClick()) {
        if (forceDrawRequested) {
            forceDrawRequested = false;
            handleGameOver(L"平局！");
        }
        return;
    }
    // 更新棋盘与界面
    chessBox[clickRow][clickCol] = HumanPlayer;
    drawChess(clickRow, clickCol, HumanPlayer);
    total_count++;
    drawStatusBar();

    if (judgeWin(clickRow, clickCol, HumanPlayer)) {
        handleGameOver(L"玩家获胜！");
        return;
    }
    if (total_count >= N * N) {
        handleGameOver(L"平局！");
        return;
    }
    // 轮到 AI
    currentPlayer = AIPlayer;
}

void update_chessBox() {
    // AI 落子后更新棋盘与界面
    chessBox[aiRow][aiCol] = AIPlayer;
    drawChess(aiRow, aiCol, AIPlayer);
    total_count++;
    drawStatusBar();

    if (judgeWin(aiRow, aiCol, AIPlayer)) {
        handleGameOver(L"AI获胜！");
        return;
    }
    if (total_count >= N * N) {
        handleGameOver(L"平局！");
        return;
    }
    // 轮到玩家
    currentPlayer = HumanPlayer;
}

// 单点棋型：计算以 (row,col) 为中心，颜色为 color 的棋型分
int evaluatePoint(int row, int col, int color) {
    int score = 0;
    int dir[4][2] = { {0,1},{1,0},{1,1},{1,-1} };
    for (int k = 0; k < 4; k++) {
        int cnt = 1;          // 该点本身
        int empty = 0;
        // 正方向
        for (int i = 1; i <= 4; i++) {
            int r = row + dir[k][0] * i;
            int c = col + dir[k][1] * i;
            if (r < 0 || r >= N || c < 0 || c >= N) break;
            if (chessBox[r][c] == color) cnt++;
            else if (chessBox[r][c] == 0) { empty++; break; }
            else break;
        }
        // 反方向
        for (int i = 1; i <= 4; i++) {
            int r = row - dir[k][0] * i;
            int c = col - dir[k][1] * i;
            if (r < 0 || r >= N || c < 0 || c >= N) break;
            if (chessBox[r][c] == color) cnt++;
            else if (chessBox[r][c] == 0) { empty++; break; }
            else break;
        }

        // 根据连续棋子数量与空位情况评分
        if (cnt >= 5) score += 100000;
        else if (cnt == 4 && empty == 2) score += 10000;
        else if (cnt == 4 && empty == 1) score += 1000;
        else if (cnt == 3 && empty == 2) score += 500;
        else if (cnt == 3 && empty == 1) score += 100;
        else if (cnt == 2 && empty == 2) score += 50;
        else if (cnt == 2 && empty == 1) score += 10;
        else score += 1;
    }
    // 加入位置权重
    score += scoreMap[row][col];
    return score;
}

// 高速全局评估：遍历已有棋子，求和所有棋子的局部棋型分
int evaluateGlobal() {
    int aiScore = 0;
    int humanScore = 0;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (chessBox[i][j] == AIPlayer) {
                aiScore += evaluatePoint(i, j, AIPlayer);
            }
            else if (chessBox[i][j] == HumanPlayer) {
                humanScore += evaluatePoint(i, j, HumanPlayer);
            }
        }
    }
    // AI 分数 - 玩家分数
    return aiScore - humanScore;
}

int MaxValue(int depth, int alpha, int beta) {
    // 极大层：轮到 AI 落子
    if (depth <= 0) {
        return evaluateGlobal();
    }

    int maxValue = INT_MIN;
    // 遍历所有空位作为候选
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (chessBox[i][j] == 0) {
                chessBox[i][j] = AIPlayer;
                int score = MinValue(depth - 1, alpha, beta);
                chessBox[i][j] = 0;

                if (score > maxValue) maxValue = score;
                if (score > alpha) alpha = score;
                if (alpha >= beta) return maxValue;   // 剪枝
            }
        }
    }
    return maxValue;
}

int MinValue(int depth, int alpha, int beta) {
    // 极小层：轮到玩家落子
    if (depth <= 0) {
        return evaluateGlobal();
    }

    int minValue = INT_MAX;
    // 遍历所有空位作为候选
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (chessBox[i][j] == 0) {
                chessBox[i][j] = HumanPlayer;
                int score = MaxValue(depth - 1, alpha, beta);
                chessBox[i][j] = 0;

                if (score < minValue) minValue = score;
                if (score < beta) beta = score;
                if (alpha >= beta) return minValue;  // 剪枝
            }
        }
    }
    return minValue;
}

maxScorePoint getMinMaxValue(int depth, int alpha, int beta) {
    // 在当前局面下选择最佳落子点
    maxScorePoint bestPoint[N * N] = { 0 };
    int maxValue = INT_MIN;
    int k = 0;

    int totalCandidates = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (chessBox[i][j] == 0) totalCandidates++;
        }
    }
    int processed = 0;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (chessBox[i][j] == 0) {
                // 尝试在 (i,j) 落子并评估
                chessBox[i][j] = AIPlayer;
                int score = MinValue(depth - 1, alpha, beta);
                chessBox[i][j] = 0;

                // 更新 AI 思考进度与响应“结束对局”
                processed++;
                updateAiThinkingProgress(processed, totalCandidates);
                if (pollEndButtonClick()) {
                    forceDrawRequested = true;
                    return bestPoint[0];
                }

                // 记录最佳点
                if (score > maxValue) {
                    maxValue = score;
                    k = 0;
                    memset(bestPoint, 0, sizeof(bestPoint));
                    bestPoint[k].row = i;
                    bestPoint[k].col = j;
                    k++;
                    if (score > alpha) alpha = score;
                }
                else if (score == maxValue) {
                    bestPoint[k].row = i;
                    bestPoint[k].col = j;
                    k++;
                }
                if (alpha >= beta) {
                    int idx = rand() % k;
                    return bestPoint[idx];
                }
            }
        }
    }
    int idx = rand() % k;
    return bestPoint[idx];
}

void AI_player() {
    // AI 回合入口
    beginAiThinking();
    // 先手第一步直接落在中心
    if (total_count == 0 && AIPlayer == 1) {
        aiRow = 7; aiCol = 7;
        update_chessBox();
        endAiThinking();
        return;
    }

    // 直接获胜：寻找一步成五的点
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (pollEndButtonClick()) {
                forceDrawRequested = false;
                handleGameOver(L"平局！");
                endAiThinking();
                return;
            }
            if (chessBox[i][j] == 0) {
                chessBox[i][j] = AIPlayer;
                if (judgeWin(i, j, AIPlayer)) {
                    aiRow = i; aiCol = j;
                    chessBox[i][j] = 0;
                    update_chessBox();
                    endAiThinking();
                    return;
                }
                chessBox[i][j] = 0;
            }
        }
    }

    // 拦截玩家获胜：阻止对方一步成五
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (pollEndButtonClick()) {
                forceDrawRequested = false;
                handleGameOver(L"平局！");
                endAiThinking();
                return;
            }
            if (chessBox[i][j] == 0) {
                chessBox[i][j] = HumanPlayer;
                if (judgeWin(i, j, HumanPlayer)) {
                    aiRow = i; aiCol = j;
                    chessBox[i][j] = 0;
                    update_chessBox();
                    endAiThinking();
                    return;
                }
                chessBox[i][j] = 0;
            }
        }
    }

    // 使用 α-β 搜索选择最优点
    int alpha = INT_MIN;
    int beta = INT_MAX;
    maxScorePoint AIPoint = getMinMaxValue(DEPTH, alpha, beta);
    if (forceDrawRequested) {
        forceDrawRequested = false;
        handleGameOver(L"平局！");
        endAiThinking();
        return;
    }
    aiRow = AIPoint.row;
    aiCol = AIPoint.col;
    update_chessBox();
    endAiThinking();
}

void updatePlayersByRole() {
    // 根据“玩家是否执黑”更新棋子颜色
    HumanPlayer = humanIsBlack ? 1 : -1;
    AIPlayer = -HumanPlayer;
}

void drawStatusBar() {
    // 绘制底部状态栏
    int top = margin + (N - 1) * block_width + 20;
    int bottom = winH;
    int areaHeight = bottom - top;

    // 清空状态栏背景
    setfillcolor(RGB(255, 205, 150));
    solidrectangle(0, top, winW, winH);

    setbkmode(TRANSPARENT);
    settextcolor(BLACK);

    // 角色提示
    wchar_t roleText[128];
    if (HumanPlayer == 1) {
        swprintf(roleText, 128, L"当前角色：玩家执黑先手");
    }
    else {
        swprintf(roleText, 128, L"当前角色：AI执黑先手");
    }

    int leftAreaRight = endButtonX - 10;
    int leftAreaWidth = leftAreaRight - margin;
    int halfWidth = leftAreaWidth / 2;
    RECT roleRect = { margin, top, margin + halfWidth, bottom };
    drawCenteredText(roleRect, roleText, 20);

    // AI 思考时间提示
    wchar_t thinkText[256];
    if (aiThinking) {
        swprintf(thinkText, 256, L" ", aiElapsedSeconds);
    }
    else {
        if (lastAiElapsedSeconds >= 0.0) {
            swprintf(thinkText, 256, L"AI思考：%.2f 秒", lastAiElapsedSeconds);
        }
        else {
            swprintf(thinkText, 256, L"AI思考：-");
        }
    }

    RECT thinkRect = { margin + halfWidth, top, leftAreaRight, bottom };
    drawCenteredText(thinkRect, thinkText, 20);

    // 绘制“结束对局”按钮
    setfillcolor(RGB(200, 80, 80));
    solidrectangle(endButtonX, endButtonY, endButtonX + endButtonWidth, endButtonY + endButtonHeight);
    RECT endBtnRect = { endButtonX, endButtonY, endButtonX + endButtonWidth, endButtonY + endButtonHeight };
    settextcolor(WHITE);
    drawCenteredText(endBtnRect, L"结束对局", 20);
    settextcolor(BLACK);
}

void beginAiThinking() {
    // 开始计时并刷新状态栏
    aiThinking = true;
    aiStartTime = clock();
    aiElapsedSeconds = 0.0;
    drawStatusBar();
}

void updateAiThinkingProgress(int processed, int total) {
    // 更新 AI 思考耗时
    if (!aiThinking) return;
    clock_t now = clock();
    aiElapsedSeconds = static_cast<double>(now - aiStartTime) / CLOCKS_PER_SEC;
    drawStatusBar();
}

void endAiThinking() {
    // 结束计时并记录本次耗时
    if (!aiThinking) return;
    clock_t now = clock();
    aiElapsedSeconds = static_cast<double>(now - aiStartTime) / CLOCKS_PER_SEC;
    lastAiElapsedSeconds = aiElapsedSeconds;
    aiThinking = false;
    drawStatusBar();
}

bool isEndButtonClick(int x, int y) {
    // 判断鼠标点是否落在“结束对局”按钮范围内
    return x >= endButtonX && x <= endButtonX + endButtonWidth &&
        y >= endButtonY && y <= endButtonY + endButtonHeight;
}

bool pollEndButtonClick() {
    // 非阻塞检测鼠标点击
    ExMessage msg;
    if (peekmessage(&msg, EM_MOUSE)) {
        if (msg.message == WM_LBUTTONDOWN && isEndButtonClick(msg.x, msg.y)) {
            forceDrawRequested = true;
            return true;
        }
    }
    return false;
}

DialogAction showActionDialog(const wchar_t* title, const wchar_t* content) {
    // 绘制自定义对话框
    IMAGE backup;
    getimage(&backup, 0, 0, winW, winH);

    int dialogW = 420;
    int dialogH = 200;
    int left = (winW - dialogW) / 2;
    int top = (winH - dialogH) / 2;
    int right = left + dialogW;
    int bottom = top + dialogH;

    // 绘制对话框背景
    setfillcolor(RGB(255, 240, 210));
    solidrectangle(left, top, right, bottom);
    setlinecolor(BLACK);
    rectangle(left, top, right, bottom);

    setbkmode(TRANSPARENT);
    settextcolor(BLACK);

    // 标题与内容
    RECT titleRect = { left, top, right, top + 44 };
    RECT contentRect = { left, top + 44, right, top + 98 };
    drawCenteredText(titleRect, title, 24);
    drawCenteredText(contentRect, content, 20);

    int buttonW = 110;
    int buttonH = 36;
    int gap = 12;
    int buttonsTop = bottom - buttonH - 20;
    int totalW = buttonW * 3 + gap * 2;
    int buttonsLeft = left + (dialogW - totalW) / 2;

    // 三个按钮的矩形区域
    RECT btnStart = { buttonsLeft, buttonsTop, buttonsLeft + buttonW, buttonsTop + buttonH };
    RECT btnSwitch = { buttonsLeft + buttonW + gap, buttonsTop, buttonsLeft + buttonW * 2 + gap, buttonsTop + buttonH };
    RECT btnExit = { buttonsLeft + (buttonW + gap) * 2, buttonsTop, buttonsLeft + buttonW * 3 + gap * 2, buttonsTop + buttonH };

    // 绘制按钮的局部函数
    auto drawButton = [](RECT r, const wchar_t* text) {
        setfillcolor(RGB(200, 200, 200));
        solidrectangle(r.left, r.top, r.right, r.bottom);
        setlinecolor(BLACK);
        rectangle(r.left, r.top, r.right, r.bottom);
        settextcolor(BLACK);
        drawCenteredText(r, text, 20);
        };

    drawButton(btnStart, L"开始游戏");
    drawButton(btnSwitch, L"切换角色");
    drawButton(btnExit, L"退出游戏");

    // 等待点击按钮并返回动作
    ExMessage msg;
    while (true) {
        msg = getmessage(EM_MOUSE);
        if (msg.message == WM_LBUTTONDOWN) {
            POINT p = { msg.x, msg.y };
            if (p.x >= btnStart.left && p.x <= btnStart.right && p.y >= btnStart.top && p.y <= btnStart.bottom) {
                putimage(0, 0, &backup);
                return DialogStart;
            }
            if (p.x >= btnSwitch.left && p.x <= btnSwitch.right && p.y >= btnSwitch.top && p.y <= btnSwitch.bottom) {
                putimage(0, 0, &backup);
                return DialogSwitch;
            }
            if (p.x >= btnExit.left && p.x <= btnExit.right && p.y >= btnExit.top && p.y <= btnExit.bottom) {
                putimage(0, 0, &backup);
                return DialogExit;
            }
        }
    }
}

bool runStartDialog() {
    // 开始对话框
    while (true) {
        wchar_t content[128];
        if (HumanPlayer == 1) {
            swprintf(content, 128, L"当前角色：玩家执黑先手");
        }
        else {
            swprintf(content, 128, L"当前角色：AI执黑先手");
        }
        DialogAction action = showActionDialog(L"开始游戏", content);
        if (action == DialogStart) {
            return true;
        }
        if (action == DialogSwitch) {
            humanIsBlack = !humanIsBlack;
            updatePlayersByRole();
            drawBoard();
            continue;
        }
        exitRequested = true;
        return false;
    }
}

bool handleGameOver(const wchar_t* resultText) {
    // 游戏结束对话框
    gameOver = true;
    while (true) {
        DialogAction action = showActionDialog(L"游戏结束", resultText);
        if (action == DialogStart) {
            initGame();
            return true;
        }
        if (action == DialogSwitch) {
            humanIsBlack = !humanIsBlack;
            updatePlayersByRole();
            drawBoard();
            continue;
        }
        exitRequested = true;
        return false;
    }
}

void drawCenteredText(const RECT& rect, const wchar_t* text, int fontSize) {
    // 在指定矩形内居中文本
    settextstyle(fontSize, 0, L"微软雅黑");
    int tw = textwidth(text);
    int th = textheight(text);
    int x = (rect.left + rect.right - tw) / 2;
    int y = (rect.top + rect.bottom - th) / 2;
    outtextxy(x, y, text);
}

int main() {
    // 程序入口
    srand((unsigned int)time(NULL));
    updatePlayersByRole();
    initGame();
    if (!runStartDialog()) {
        closegraph();
        return 0;
    }
    initGame();

    // 主循环：轮流下棋
    while (!exitRequested) {
        if (!gameOver) {
            if (currentPlayer == HumanPlayer) {
                human_play();
            }
            else {
                AI_player();
            }
        }
    }

    // 退出前等待按键并关闭窗口
    if (!exitRequested) {
        _getch();
    }
    closegraph();
    return 0;
}