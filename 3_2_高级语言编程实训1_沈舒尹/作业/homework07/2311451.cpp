#include <graphics.h>
#include <conio.h>
#include <windows.h>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>

using std::string;
using std::vector;

// -------------------- 画布与布局参数 --------------------
const int BOARD_W = 12;
const int BOARD_H = 20;
const int BLOCK_SIZE = 28;

const int LEFT_MARGIN = 30;
const int TOP_MARGIN = 30;

const int BOARD_PIXEL_W = BOARD_W * BLOCK_SIZE;
const int BOARD_PIXEL_H = BOARD_H * BLOCK_SIZE;

const int PANEL_GAP = 24;
const int RIGHT_PANEL_W = 280;

const int WINDOW_W = LEFT_MARGIN * 2 + BOARD_PIXEL_W + PANEL_GAP + RIGHT_PANEL_W;
const int WINDOW_H = TOP_MARGIN * 2 + BOARD_PIXEL_H;

const int BOARD_X = LEFT_MARGIN;
const int BOARD_Y = TOP_MARGIN;
const int PANEL_X = BOARD_X + BOARD_PIXEL_W + PANEL_GAP;
const int PANEL_Y = TOP_MARGIN;

// 主题色
const COLORREF BG_COLOR = RGB(246, 248, 252);
const COLORREF GRID_COLOR = RGB(225, 230, 240);
const COLORREF BORDER_COLOR = RGB(90, 100, 120);
const COLORREF TEXT_COLOR = RGB(50, 60, 80);

// 右侧功能区与弹窗配色
const COLORREF PANEL_BORDER_COLOR = RGB(88, 118, 190);
const COLORREF PANEL_BOX_COLOR = RGB(122, 150, 216);
const COLORREF PANEL_TITLE_COLOR = RGB(52, 90, 190);
const COLORREF PANEL_LABEL_COLOR = RGB(45, 96, 150);
const COLORREF PANEL_VALUE_COLOR = RGB(120, 52, 180);
const COLORREF PANEL_HINT_COLOR = RGB(45, 120, 95);

const COLORREF OVERLAY_BORDER_COLOR = RGB(120, 138, 196);
const COLORREF OVERLAY_TITLE_COLOR = RGB(44, 78, 170);
const COLORREF OVERLAY_TEXT_COLOR = RGB(88, 70, 150);

// 排行榜持久化文件（保存于程序运行目录）
const char* SCORE_FILE = "scores.txt";

// 俄罗斯方块 7 种形状，4 个旋转状态，每个状态 4 个单元，相对坐标 (x, y)
static const int Block[7][4][4][2] = {
    // T
    {
        {{0, 0}, {-1, 0}, {1, 0}, {0, -1}},
        {{0, 0}, {0, -1}, {0, 1}, {1, 0}},
        {{0, 0}, {-1, 0}, {1, 0}, {0, 1}},
        {{0, 0}, {0, -1}, {0, 1}, {-1, 0}}
    },
    // S
    {
        {{0, 0}, {-1, 0}, {0, -1}, {1, -1}},
        {{0, 0}, {0, -1}, {1, 0}, {1, 1}},
        {{0, 0}, {-1, 0}, {0, -1}, {1, -1}},
        {{0, 0}, {0, -1}, {1, 0}, {1, 1}}
    },
    // Z
    {
        {{0, 0}, {-1, -1}, {0, -1}, {1, 0}},
        {{0, 0}, {1, -1}, {1, 0}, {0, 1}},
        {{0, 0}, {-1, -1}, {0, -1}, {1, 0}},
        {{0, 0}, {1, -1}, {1, 0}, {0, 1}}
    },
    // I
    {
        {{0, 0}, {-2, 0}, {-1, 0}, {1, 0}},
        {{0, 0}, {0, -2}, {0, -1}, {0, 1}},
        {{0, 0}, {-2, 0}, {-1, 0}, {1, 0}},
        {{0, 0}, {0, -2}, {0, -1}, {0, 1}}
    },
    // O
    {
        {{0, 0}, {-1, 0}, {0, -1}, {-1, -1}},
        {{0, 0}, {-1, 0}, {0, -1}, {-1, -1}},
        {{0, 0}, {-1, 0}, {0, -1}, {-1, -1}},
        {{0, 0}, {-1, 0}, {0, -1}, {-1, -1}}
    },
    // L
    {
        {{0, 0}, {-1, 0}, {1, 0}, {1, -1}},
        {{0, 0}, {0, -1}, {0, 1}, {1, 1}},
        {{0, 0}, {-1, 1}, {-1, 0}, {1, 0}},
        {{0, 0}, {-1, -1}, {0, -1}, {0, 1}}
    },
    // J
    {
        {{0, 0}, {-1, 0}, {1, 0}, {-1, -1}},
        {{0, 0}, {0, -1}, {0, 1}, {1, -1}},
        {{0, 0}, {-1, 0}, {1, 0}, {1, 1}},
        {{0, 0}, {-1, 1}, {0, -1}, {0, 1}}
    }
};

static const COLORREF BlockColor[7] = {
    RGB(125, 126, 255), // T
    RGB(110, 214, 153), // S
    RGB(255, 158, 128), // Z
    RGB(104, 198, 255), // I
    RGB(255, 211, 87),  // O
    RGB(255, 140, 176), // L
    RGB(178, 142, 255)  // J
};

// 当前可控方块
struct Piece {
    int x;
    int y;
    int type;
    int rot;
};

// 游戏状态机
enum GameState {
    WAIT_START, // 欢迎页
    PLAYING,    // 游戏进行中
    GAME_OVER   // 本局结束
};

// 已落地方块网格：0 表示空，1~7 对应方块类型
int board[BOARD_W][BOARD_H] = { 0 };
Piece cur{}, nxt{};
GameState state = WAIT_START;

int score = 0;
int difficulty = 5; // 1~10
DWORD lastFallTick = 0;

// rankByDiff[d]：难度 d 的历史 Top 分数（降序 5 条）
vector<int> rankByDiff[11]; // 使用下标 1~10

int fallIntervalMs(int diff) {
    // 根据难度返回自动下落间隔
    // 难度越高，间隔越短，速度越快
    // diff=1 -> 700ms，diff=10 -> 160ms
    return 700 - (diff - 1) * 60;
}

bool isCellInside(int x, int y) {
    // 仅检查左右和底边
    return x >= 0 && x < BOARD_W && y < BOARD_H;
}

bool canPlace(const Piece& p) {
    // 判断方块 p 的 4 个单元是否都可以合法放置
    // 1) 不越过左右边界与底边
    // 2) 与已落地方块不重叠
    for (int i = 0; i < 4; ++i) {
        int nx = p.x + Block[p.type][p.rot][i][0];
        int ny = p.y + Block[p.type][p.rot][i][1];

        if (!isCellInside(nx, ny)) return false;
        if (ny >= 0 && board[nx][ny] != 0) return false;
    }
    return true;
}

void clearBoard() {
    // 清空已落地方块数组
    for (int x = 0; x < BOARD_W; ++x) {
        for (int y = 0; y < BOARD_H; ++y) {
            board[x][y] = 0;
        }
    }
}

Piece randomPiece() {
    // 生成一个随机方块：类型随机、旋转随机、出生点固定
    Piece p;
    p.type = rand() % 7;
    p.rot = rand() % 4;
    p.x = BOARD_W / 2;
    p.y = 1;
    return p;
}

void loadRank() {
    // 从文件加载历史成绩
    std::ifstream fin(SCORE_FILE);
    if (!fin.is_open()) return;

    int d, s;
    while (fin >> d >> s) {
        if (d >= 1 && d <= 10) rankByDiff[d].push_back(s);
    }
    fin.close();

    // 每个难度按降序并截断到 Top5
    for (int d = 1; d <= 10; ++d) {
        std::sort(rankByDiff[d].begin(), rankByDiff[d].end(), std::greater<int>());
        if (rankByDiff[d].size() > 5) rankByDiff[d].resize(5);
    }
}

void saveRank() {
    // 全量覆盖写回排行榜文件
    std::ofstream fout(SCORE_FILE, std::ios::trunc);
    if (!fout.is_open()) return;

    for (int d = 1; d <= 10; ++d) {
        for (int s : rankByDiff[d]) {
            fout << d << " " << s << "\n";
        }
    }
    fout.close();
}

void pushScoreToRank(int diff, int s) {
    // 向指定难度榜单插入一条成绩，并维护 Top5
    rankByDiff[diff].push_back(s);
    std::sort(rankByDiff[diff].begin(), rankByDiff[diff].end(), std::greater<int>());
    if (rankByDiff[diff].size() > 5) rankByDiff[diff].resize(5);
    saveRank();
}

void drawUnitBlockPixel(int px, int py, int size, COLORREF color) {
    // 绘制一个像素坐标下的方块单元
    setlinecolor(RGB(240, 240, 245));
    setfillcolor(color);
    solidrectangle(px, py, px + size - 1, py + size - 1);
}

void drawBoardGrid() {
    // 绘制网格线
    setlinecolor(GRID_COLOR);
    for (int i = 0; i <= BOARD_W; ++i) {
        int x = BOARD_X + i * BLOCK_SIZE;
        line(x, BOARD_Y, x, BOARD_Y + BOARD_PIXEL_H);
    }
    for (int i = 0; i <= BOARD_H; ++i) {
        int y = BOARD_Y + i * BLOCK_SIZE;
        line(BOARD_X, y, BOARD_X + BOARD_PIXEL_W, y);
    }
}

void drawBoardBorder() {
    // 绘制游戏区外框
    setlinecolor(BORDER_COLOR);
    setlinestyle(PS_SOLID, 2);
    rectangle(BOARD_X - 1, BOARD_Y - 1, BOARD_X + BOARD_PIXEL_W + 1, BOARD_Y + BOARD_PIXEL_H + 1);
    setlinestyle(PS_SOLID, 1);
}

void drawBoardBlocks() {
    // 根据 board[][] 重绘全部已落地方块
    for (int x = 0; x < BOARD_W; ++x) {
        for (int y = 0; y < BOARD_H; ++y) {
            int px = BOARD_X + x * BLOCK_SIZE;
            int py = BOARD_Y + y * BLOCK_SIZE;
            if (board[x][y] == 0) {
                drawUnitBlockPixel(px, py, BLOCK_SIZE, RGB(250, 251, 255));
            }
            else {
                drawUnitBlockPixel(px, py, BLOCK_SIZE, BlockColor[board[x][y] - 1]);
            }
        }
    }
}

void drawPieceOnBoard(const Piece& p, COLORREF colorOverride = CLR_INVALID) {
    // 在游戏区绘制“活动方块”
    // colorOverride 可用于特殊颜色覆盖
    for (int i = 0; i < 4; ++i) {
        int nx = p.x + Block[p.type][p.rot][i][0];
        int ny = p.y + Block[p.type][p.rot][i][1];
        if (ny < 0) continue;
        if (nx < 0 || nx >= BOARD_W || ny >= BOARD_H) continue;
        COLORREF c = (colorOverride == CLR_INVALID) ? BlockColor[p.type] : colorOverride;
        int px = BOARD_X + nx * BLOCK_SIZE;
        int py = BOARD_Y + ny * BLOCK_SIZE;
        drawUnitBlockPixel(px, py, BLOCK_SIZE, c);
    }
}

void drawNextPiecePreview() {
    // 右侧“下一个方块”预览框
    int boxX = PANEL_X + 20;
    int boxY = PANEL_Y + 80;
    int boxW = RIGHT_PANEL_W - 40;
    int boxH = 130;

    setlinecolor(PANEL_BOX_COLOR);
    rectangle(boxX, boxY, boxX + boxW, boxY + boxH);

    int mini = 20;
    int cx = boxX + boxW / 2;
    int cy = boxY + boxH / 2;

    for (int i = 0; i < 4; ++i) {
        int ox = Block[nxt.type][0][i][0];
        int oy = Block[nxt.type][0][i][1];
        int px = cx + ox * mini - mini / 2;
        int py = cy + oy * mini - mini / 2;
        drawUnitBlockPixel(px, py, mini, BlockColor[nxt.type]);
    }

    setbkmode(TRANSPARENT);
    settextcolor(PANEL_LABEL_COLOR);
    settextstyle(20, 0, _T("微软雅黑"));
    outtextxy(boxX, boxY - 32, _T("下一个方块"));
}

void drawRankPanel() {
    // 右侧排行榜区域：按当前难度显示 Top5
    int x = PANEL_X + 20;
    int y = PANEL_Y + 260;
    int w = RIGHT_PANEL_W - 40;
    int h = 180;

    setlinecolor(PANEL_BOX_COLOR);
    rectangle(x, y, x + w, y + h);

    setbkmode(TRANSPARENT);
    settextcolor(PANEL_LABEL_COLOR);
    settextstyle(17, 0, _T("微软雅黑"));

    TCHAR title[64];
    _stprintf_s(title, _T("历史前五（难度 %d）"), difficulty);
    outtextxy(x + 10, y + 8, title);

    const vector<int>& v = rankByDiff[difficulty];
    for (int i = 0; i < 5; ++i) {
        TCHAR line[64];
        if (i < (int)v.size()) _stprintf_s(line, _T("第%d名：%d"), i + 1, v[i]);
        else _stprintf_s(line, _T("第%d名：---"), i + 1);
        settextcolor((i % 2 == 0) ? PANEL_VALUE_COLOR : PANEL_LABEL_COLOR);
        outtextxy(x + 20, y + 40 + i * 26, line);
    }
}

void drawRightPanel() {
    // 右侧功能信息总面板
    setlinecolor(PANEL_BORDER_COLOR);
    rectangle(PANEL_X, PANEL_Y, PANEL_X + RIGHT_PANEL_W, PANEL_Y + BOARD_PIXEL_H);

    setbkmode(TRANSPARENT);
    settextcolor(PANEL_TITLE_COLOR);

    settextstyle(28, 0, _T("微软雅黑"));
    LPCTSTR gameTitle = _T("俄罗斯方块");
    int titleW = textwidth(gameTitle);
    outtextxy(PANEL_X + (RIGHT_PANEL_W - titleW) / 2, PANEL_Y + 12, gameTitle);

    drawNextPiecePreview();

    settextstyle(19, 0, _T("微软雅黑"));
    TCHAR s1[64], s2[64];
    _stprintf_s(s1, _T("当前分数：%d"), score);
    _stprintf_s(s2, _T("当前难度：%d"), difficulty);
    settextcolor(PANEL_VALUE_COLOR);
    outtextxy(PANEL_X + 20, PANEL_Y + 225, s1);
    settextcolor(PANEL_LABEL_COLOR);
    outtextxy(PANEL_X + 20, PANEL_Y + 450, s2);

    drawRankPanel();

    // 操作说明
    settextstyle(19, 0, _T("微软雅黑"));
    settextcolor(PANEL_HINT_COLOR);
    outtextxy(PANEL_X + 16, PANEL_Y + 490, _T("操作：A/D左右移动，W旋转，S加速下落"));
    outtextxy(PANEL_X + 16, PANEL_Y + 512, _T("Q退出，空格开始/重开"));
    outtextxy(PANEL_X + 16, PANEL_Y + 534, _T("↑/↓ 调整难度（仅未开始或结束时）"));
}

void drawOverlayMessage(const TCHAR* title, const TCHAR* line1, const TCHAR* line2 = nullptr) {
    // 左侧游戏区中央弹窗（欢迎页 / 结束页）
    int x1 = BOARD_X + 35;
    int y1 = BOARD_Y + BOARD_PIXEL_H / 2 - 85;
    int x2 = BOARD_X + BOARD_PIXEL_W - 35;
    int y2 = BOARD_Y + BOARD_PIXEL_H / 2 + 85;

    setfillcolor(RGB(255, 255, 255));
    setlinecolor(OVERLAY_BORDER_COLOR);
    solidrectangle(x1, y1, x2, y2);
    rectangle(x1, y1, x2, y2);

    setbkmode(TRANSPARENT);
    settextcolor(OVERLAY_TITLE_COLOR);
    settextstyle(34, 0, _T("微软雅黑"));

    int tw1 = textwidth(title);
    outtextxy((x1 + x2 - tw1) / 2, y1 + 30, title);

    settextstyle(20, 0, _T("微软雅黑"));
    settextcolor(OVERLAY_TEXT_COLOR);
    int tw2 = textwidth(line1);
    outtextxy((x1 + x2 - tw2) / 2, y1 + 92, line1);

    if (line2 != nullptr) {
        int tw3 = textwidth(line2);
        outtextxy((x1 + x2 - tw3) / 2, y1 + 122, line2);
    }
}

void drawScene() {
    // 每一帧完整重绘
    setbkcolor(BG_COLOR);
    cleardevice();

    drawBoardBlocks();
    if (state == PLAYING) drawPieceOnBoard(cur);
    drawBoardGrid();
    drawBoardBorder();
    drawRightPanel();

    if (state == WAIT_START) {
        drawOverlayMessage(_T("欢迎来到俄罗斯方块"), _T("按 空格 开始游戏"));
    }
    else if (state == GAME_OVER) {
        TCHAR line1[64];
        _stprintf_s(line1, _T("本局得分：%d"), score);
        drawOverlayMessage(_T("游戏结束"), line1, _T("按 空格 重新开始"));
    }
}

bool isOccupiedCompletely(int row) {
    // 判断一整行是否被占满
    for (int x = 0; x < BOARD_W; ++x) {
        if (board[x][row] == 0) return false;
    }
    return true;
}

void removeRow(int row) {
    // 消除第 row 行：把其上方所有行整体下移一行
    for (int y = row; y >= 1; --y) {
        for (int x = 0; x < BOARD_W; ++x) {
            board[x][y] = board[x][y - 1];
        }
    }
    for (int x = 0; x < BOARD_W; ++x) board[x][0] = 0;
}

void removeWholeRows() {
    // 从底到顶扫描，发现满行就删除
    // 删除后 y++ 回查当前行
    for (int y = BOARD_H - 1; y >= 0; --y) {
        if (isOccupiedCompletely(y)) {
            removeRow(y);
            score += 1;
            y++; // 继续检查当前行
        }
    }
}

void landCurrentPiece() {
    // 活动方块落地写入 board[][]，随后执行满行消除
    for (int i = 0; i < 4; ++i) {
        int nx = cur.x + Block[cur.type][cur.rot][i][0];
        int ny = cur.y + Block[cur.type][cur.rot][i][1];
        if (ny >= 0 && nx >= 0 && nx < BOARD_W && ny < BOARD_H) {
            board[nx][ny] = cur.type + 1;
        }
    }
    removeWholeRows();
}

void spawnNextPieceOrGameOver() {
    // 把“下一块”变为当前块，再生成新的下一块
    // 若新当前块无法放置，判定游戏结束
    cur = nxt;
    nxt = randomPiece();
    if (!canPlace(cur)) {
        state = GAME_OVER;
        pushScoreToRank(difficulty, score);
    }
}

void startNewGame() {
    // 开始（重开）一局：清空棋盘和分数，初始化当前块/下一块
    clearBoard();
    score = 0;
    cur = randomPiece();
    nxt = randomPiece();
    if (!canPlace(cur)) {
        state = GAME_OVER;
        pushScoreToRank(difficulty, score);
    }
    else {
        state = PLAYING;
        lastFallTick = GetTickCount();
    }
}

void tryMove(int dx, int dy) {
    // 尝试平移
    Piece np = cur;
    np.x += dx;
    np.y += dy;
    if (canPlace(np)) cur = np;
}

void tryRotate() {
    // 尝试顺时针旋转；若原地失败，进行简单墙踢
    Piece np = cur;
    np.rot = (np.rot + 1) % 4;
    if (canPlace(np)) {
        cur = np;
        return;
    }

    // 简单墙踢：左右尝试一格
    np = cur;
    np.rot = (np.rot + 1) % 4;
    np.x -= 1;
    if (canPlace(np)) {
        cur = np;
        return;
    }

    np = cur;
    np.rot = (np.rot + 1) % 4;
    np.x += 1;
    if (canPlace(np)) cur = np;
}

void stepFall() {
    // 下落一步：能下落则更新位置；否则落地并生成下一块
    Piece np = cur;
    np.y += 1;
    if (canPlace(np)) {
        cur = np;
    }
    else {
        landCurrentPiece();
        spawnNextPieceOrGameOver();
    }
}

void onKeyPlaying(int ch) {
    // 按键控制，游戏进行时
    if (ch == 'a' || ch == 'A') {
        tryMove(-1, 0);
    }
    else if (ch == 'd' || ch == 'D') {
        tryMove(1, 0);
    }
    else if (ch == 's' || ch == 'S') {
        stepFall();
        lastFallTick = GetTickCount();
    }
    else if (ch == 'w' || ch == 'W') {
        tryRotate();
    }
    else if (ch == 'q' || ch == 'Q') {
        closegraph();
        exit(0);
    }
}

void onKeyPausedOrOver(int ch) {
    // 非进行状态：
    // 1) Q 退出
    // 2) 空格开始/重开
    // 3) 上下方向键调整难度
    if (ch == 'q' || ch == 'Q') {
        closegraph();
        exit(0);
    }
    if (ch == ' ') {
        startNewGame();
        return;
    }

    // 上下箭头
    if (ch == 224 || ch == 0) {
        int c2 = _getch();
        if (c2 == 72) {          // Up
            if (difficulty < 10) difficulty++;
        }
        else if (c2 == 80) {   // Down
            if (difficulty > 1) difficulty--;
        }
    }
}

void processInput() {
    // 统一输入分发：根据 state 调用对应处理函数
    if (!_kbhit()) return;

    int ch = _getch();
    if (state == PLAYING) {
        onKeyPlaying(ch);
    }
    else {
        onKeyPausedOrOver(ch);
    }
}

void autoFall() {
    // 自动下落计时器：到间隔就执行一次 stepFall()
    if (state != PLAYING) return;
    DWORD now = GetTickCount();
    if (now - lastFallTick >= (DWORD)fallIntervalMs(difficulty)) {
        stepFall();
        lastFallTick = now;
    }
}

int main() {
    srand((unsigned int)time(nullptr));
    loadRank();

    initgraph(WINDOW_W, WINDOW_H);
    BeginBatchDraw();

    while (true) {
        processInput();
        autoFall();
        drawScene();
        FlushBatchDraw();
        Sleep(10);
    }

    EndBatchDraw();
    closegraph();
    return 0;
}
