#include <graphics.h>
#include <conio.h>
#include <windows.h>
#include <tchar.h>
#include <algorithm>
#include <array>
#include <ctime>
#include <deque>
#include <string>
#include <vector>

using namespace std;

// ========================= 基础常量 =========================
const int WINDOW_WIDTH = 1000;                  // 窗口宽
const int WINDOW_HEIGHT = 640;                  // 窗口高
const int BLOCK_SIZE = 20;                      // 单位格边长（像素）
const int GRID_W = WINDOW_WIDTH / BLOCK_SIZE;   // 地图网格宽
const int GRID_H = WINDOW_HEIGHT / BLOCK_SIZE;  // 地图网格高
const int INIT_SNAKE_LEN = 3;                   // 初始蛇长
const DWORD FOOD_SPAWN_INTERVAL = 2200;         // 食物刷新间隔（毫秒）
const DWORD ROUND_TIME_LIMIT_MS = 180000;       // 每局时长：3分钟

// 方向定义
enum Direction {
    DIR_UP = 1,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
};

// 游戏状态
enum GameState {
    STATE_START_MENU = 0,
    STATE_RUNNING,
    STATE_GAME_OVER
};

// 游戏模式（单人/双人）
enum GameMode {
    MODE_SINGLE = 0,
    MODE_DOUBLE
};

// 坐标结构（游戏网格坐标）
struct Cell {
    int x;
    int y;
};

// 食物结构
struct Food {
    int x;
    int y;
    bool golden;    // true=金色食物，false=普通食物（青色）
};

// ========================= 全局变量 =========================
GameState g_state = STATE_START_MENU;
GameMode g_selectedMode = MODE_SINGLE;      // 菜单当前选择模式
GameMode g_roundMode = MODE_SINGLE;         // 当前局模式

int g_difficulty = 1;                   // 当前难度（1-10）
int g_currentScore = INIT_SNAKE_LEN;    // 单人实时分数=实时蛇长
int g_currentScore1 = INIT_SNAKE_LEN;   // 双人模式玩家1（WASD）分数
int g_currentScore2 = INIT_SNAKE_LEN;   // 双人模式玩家2（方向键）分数
Direction g_dir = DIR_RIGHT;            // 当前方向
Direction g_dir2 = DIR_LEFT;            // 玩家2方向（双人）
bool g_quit = false;                    // 是否退出主循环
deque<Cell> g_snake;                    // 动态蛇身
deque<Cell> g_snake2;                   // 双人模式第二条蛇
deque<Cell> g_walls;                    // 墙体列表
vector<Food> g_foods;                   // 食物列表
DWORD g_lastMoveTick = 0;               // 上一次蛇移动的时间戳
DWORD g_lastFoodSpawnTick = 0;          // 上一次生成食物的时间戳
DWORD g_roundStartTick = 0;             // 当前局开始时间
vector<int> g_scoreHistory[11];         // 历史分数（区分难度）
int g_dualWinsByDifficulty[11][2] = {}; // 双人模式按难度获胜局数
int g_dualWinsTotal[2] = { 0, 0 };      // 双人模式总获胜局数

// 上一局结果
GameMode g_lastRoundMode = MODE_SINGLE;
int g_lastSingleScore = INIT_SNAKE_LEN;
wstring g_lastWinnerText = L"-";

// 双人模式最近一步结束原因
bool g_dualLastP1Dead = false;
bool g_dualLastP2Dead = false;
bool g_dualLastHeadOn = false;

// ========================= 工具函数 =========================
// 生成指定范围内的随机整数
int randInt(int l, int r) {
    return l + rand() % (r - l + 1);
}

// 判断两个方向是否为相反方向
bool isOpposite(Direction a, Direction b) {
    return (a == DIR_UP && b == DIR_DOWN) ||
        (a == DIR_DOWN && b == DIR_UP) ||
        (a == DIR_LEFT && b == DIR_RIGHT) ||
        (a == DIR_RIGHT && b == DIR_LEFT);
}

// 判断两个网格坐标是否相同
bool sameCell(const Cell& a, const Cell& b) {
    return a.x == b.x && a.y == b.y;
}

// 判断指定坐标是否在任意蛇身上
bool inSnake(int x, int y) {
    for (const auto& s : g_snake) {
        if (s.x == x && s.y == y) return true;
    }
    for (const auto& s : g_snake2) {
        if (s.x == x && s.y == y) return true;
    }
    return false;
}

// 判断指定坐标是否为墙
bool inWalls(int x, int y) {
    for (const auto& w : g_walls) {
        if (w.x == x && w.y == y) return true;
    }
    return false;
}

// 判断指定坐标是否已有食物
bool inFoods(int x, int y) {
    for (const auto& f : g_foods) {
        if (f.x == x && f.y == y) return true;
    }
    return false;
}

// 菜单显示文本：当前模式
wstring modeText(GameMode mode) {
    return (mode == MODE_SINGLE) ? L"单人模式" : L"双人模式";
}

// 将剩余毫秒格式化为 mm:ss
wstring formatRemainTime(DWORD remainMs) {
    DWORD totalSec = remainMs / 1000;
    int mm = (int)(totalSec / 60);
    int ss = (int)(totalSec % 60);

    wstring s = L"";
    if (mm < 10) s += L"0";
    s += to_wstring(mm);
    s += L":";
    if (ss < 10) s += L"0";
    s += to_wstring(ss);
    return s;
}

// 在窗口指定y坐标水平居中绘制一行文字
// 参数：y坐标、文字内容、字体大小、颜色
void drawCenterTextLine(int y, const wstring& text, int fontSize, COLORREF color) {
    settextstyle(fontSize, 0, _T("微软雅黑"));
    settextcolor(color);
    setbkmode(TRANSPARENT); // 设置背景透明

    // 设置文字绘制矩形区域
    RECT rc = { 0, y, WINDOW_WIDTH, y + fontSize + 8 };
    // 居中、垂直居中、单行绘制
    drawtext(text.c_str(), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

// 在指定矩形内绘制单行文字（居中）
void drawTextInRect(int l, int t, int r, int b, const wstring& text, int fontSize, COLORREF color) {
    settextstyle(fontSize, 0, _T("微软雅黑"));
    settextcolor(color);
    setbkmode(TRANSPARENT);

    RECT rc = { l, t, r, b };
    drawtext(text.c_str(), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

// 绘制基础场景（gray=true 表示灰度风格）
void drawSceneBase(bool gray) {
    // 背景
    if (gray) setbkcolor(RGB(170, 170, 170));
    else      setbkcolor(RGB(232, 244, 232));
    cleardevice();

    // 边框
    setlinecolor(gray ? RGB(80, 80, 80) : RGB(60, 120, 80));
    setlinestyle(PS_SOLID, 2);
    rectangle(0, 0, WINDOW_WIDTH - 1, WINDOW_HEIGHT - 1);
}

// 绘制一个单位格
void drawCellBlock(int gx, int gy, COLORREF color, COLORREF border) {
    int l = gx * BLOCK_SIZE;
    int t = gy * BLOCK_SIZE;
    int r = l + BLOCK_SIZE - 1;
    int b = t + BLOCK_SIZE - 1;

    setfillcolor(color);
    setlinecolor(border);
    fillrectangle(l, t, r, b);
}

// 绘制食物
void drawFoodCircle(const Food& f, bool gray) {
    // 计算圆心
    int cx = f.x * BLOCK_SIZE + BLOCK_SIZE / 2;
    int cy = f.y * BLOCK_SIZE + BLOCK_SIZE / 2;
    int radius = BLOCK_SIZE / 2 - 2;    // 半径略小于格子

    COLORREF fill = RGB(0, 220, 220);   // 普通食物：青色
    COLORREF line = RGB(0, 170, 170);

    if (f.golden) {
        fill = RGB(255, 210, 0);        // 金色食物
        line = RGB(220, 170, 0);
    }

    if (gray) { // 灰度模式
        fill = f.golden ? RGB(210, 210, 210) : RGB(140, 140, 140);
        line = f.golden ? RGB(170, 170, 170) : RGB(100, 100, 100);
    }

    setfillcolor(fill);
    setlinecolor(line);
    fillcircle(cx, cy, radius);
}

// 绘制整条蛇
void drawSnake(bool gray) {
    for (int i = 0; i < (int)g_snake.size(); ++i) {
        const Cell& s = g_snake[i];
        if (i == 0) {
            // 蛇头
            if (gray) drawCellBlock(s.x, s.y, RGB(95, 95, 95), RGB(70, 70, 70));
            else      drawCellBlock(s.x, s.y, RGB(255, 120, 120), RGB(220, 90, 90));
        }
        else {
            // 蛇身粉色渐变
            if (gray) {
                int v = 50 + (i * 90) / (g_snake.size() - 1);
                drawCellBlock(s.x, s.y, RGB(v, v, v), RGB(v - 15, v - 15, v - 15));
            }
            else {
                int g = 180 + (i * 60) / (g_snake.size() - 1);
                int b = 180 + (i * 60) / (g_snake.size() - 1);
                drawCellBlock(s.x, s.y, RGB(255, g, b), RGB(230, g - 20, b - 20));
            }
        }
    }
}

// 绘制第二条蛇（绿色渐变）
void drawSnake2(bool gray) {
    for (int i = 0; i < (int)g_snake2.size(); ++i) {
        const Cell& s = g_snake2[i];
        if (i == 0) {
            // 蛇头
            if (gray) drawCellBlock(s.x, s.y, RGB(95, 95, 95), RGB(70, 70, 70));
            else      drawCellBlock(s.x, s.y, RGB(120, 220, 150), RGB(80, 185, 115));
        }
        else {
            if (gray) {
                int v = 55 + (i * 85) / max(1, (int)g_snake2.size() - 1);
                drawCellBlock(s.x, s.y, RGB(v, v, v), RGB(v - 15, v - 15, v - 15));
            }
            else {
                int r = 175 + (i * 55) / max(1, (int)g_snake2.size() - 1);
                int g = 230 + (i * 20) / max(1, (int)g_snake2.size() - 1);
                int b = 185 + (i * 55) / max(1, (int)g_snake2.size() - 1);
                drawCellBlock(s.x, s.y, RGB(r, g, b), RGB(r - 25, g - 25, b - 25));
            }
        }
    }
}

// 绘制墙
void drawWalls(bool gray) {
    for (const auto& w : g_walls) {
        if (gray) drawCellBlock(w.x, w.y, RGB(55, 55, 55), RGB(25, 25, 25));
        else      drawCellBlock(w.x, w.y, RGB(20, 20, 20), RGB(0, 0, 0));
    }
}

// 绘制所有食物
void drawFoods(bool gray) {
    for (const auto& f : g_foods) {
        drawFoodCircle(f, gray);
    }
}

// 绘制单人实时分数
void drawSingleScore() {
    settextstyle(26, 0, _T("Consolas"));
    settextcolor(RGB(25, 70, 35));
    setbkmode(TRANSPARENT);

    wstring text = L"Score: " + to_wstring(g_currentScore);
    RECT rc = { WINDOW_WIDTH - 250, 15, WINDOW_WIDTH - 20, 60 };
    drawtext(text.c_str(), &rc, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
}

// 绘制双人分数 + 顶部倒计时
void drawDualHud() {
    setbkmode(TRANSPARENT);

    settextstyle(24, 0, _T("Consolas"));

    settextcolor(RGB(210, 70, 90));
    wstring left = L"WASD: " + to_wstring(g_currentScore1);
    RECT rcL = { 16, 10, 320, 52 };
    drawtext(left.c_str(), &rcL, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    settextcolor(RGB(45, 140, 90));
    wstring right = L"Arrows: " + to_wstring(g_currentScore2);
    RECT rcR = { WINDOW_WIDTH - 320, 10, WINDOW_WIDTH - 16, 52 };
    drawtext(right.c_str(), &rcR, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

    DWORD elapsed = GetTickCount() - g_roundStartTick;
    DWORD remain = (elapsed >= ROUND_TIME_LIMIT_MS) ? 0 : (ROUND_TIME_LIMIT_MS - elapsed);

    settextstyle(30, 0, _T("Consolas"));
    settextcolor(RGB(80, 80, 210));
    wstring tm = L"Time " + formatRemainTime(remain);
    RECT rcM = { WINDOW_WIDTH / 2 - 150, 8, WINDOW_WIDTH / 2 + 150, 52 };
    drawtext(tm.c_str(), &rcM, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

// 计算移动延迟（毫秒）：难度越高、蛇越长，速度越快（延迟越小）
int calcMoveDelayMs() {
    // 难度1固定速度，不随长度变化
    if (g_difficulty == 1) return 250;

    // 难度8~10初始速度更快
    int baseDelay = 250;
    if (g_difficulty >= 8) {
        // 8:200, 9:150, 10:100
        baseDelay = 200 - (g_difficulty - 8) * 50;
    }

    // 难度2~10都带长度加速，难度越高加速度越大
    int lenGain = (int)g_snake.size() - INIT_SNAKE_LEN;
    if (lenGain < 0) lenGain = 0;

    int accPerLen = g_difficulty + 1;  // 难度越高每增长1格减少更多延迟
    int delay = baseDelay - lenGain * accPerLen;

    // 下限，避免过快导致几乎不可玩
    int minDelay = (g_difficulty >= 9) ? 40 : 55;
    return max(delay, minDelay);
}

// 获取指定难度去重后的前三名分数（降序）
array<int, 3> getTop3DistinctScores(int difficulty) {
    array<int, 3> top = { -1, -1, -1 };
    vector<int> tmp = g_scoreHistory[difficulty];

    sort(tmp.begin(), tmp.end(), greater<int>());
    tmp.erase(unique(tmp.begin(), tmp.end()), tmp.end()); // 去重

    for (int i = 0; i < 3 && i < (int)tmp.size(); ++i) {
        top[i] = tmp[i];
    }
    return top;
}

// 根据难度生成墙数量（难度5及以上才有墙）
int wallCountByDifficulty() {
    if (g_difficulty <= 4) return 0;
    return (g_difficulty - 4) * 15;
}

// ========================= 核心逻辑 =========================

// 随机初始化蛇（长度3，位置不贴边，方向随机）
void initSnakeRandom() {
    g_snake.clear();

    // 随机方向
    g_dir = static_cast<Direction>(randInt(1, 4));

    // 蛇头随机出现在地图中间区域，避免开局撞墙
    int hx = randInt(5, GRID_W - 6);
    int hy = randInt(5, GRID_H - 6);

    // 蛇身朝“反方向”延伸，确保初始合法
    g_snake.push_back({ hx, hy });

    if (g_dir == DIR_UP) {
        g_snake.push_back({ hx, hy + 1 });
        g_snake.push_back({ hx, hy + 2 });
    }
    else if (g_dir == DIR_DOWN) {
        g_snake.push_back({ hx, hy - 1 });
        g_snake.push_back({ hx, hy - 2 });
    }
    else if (g_dir == DIR_LEFT) {
        g_snake.push_back({ hx + 1, hy });
        g_snake.push_back({ hx + 2, hy });
    }
    else { // DIR_RIGHT
        g_snake.push_back({ hx - 1, hy });
        g_snake.push_back({ hx - 2, hy });
    }

    g_currentScore = (int)g_snake.size();
    g_currentScore1 = g_currentScore;
}

// 初始化第二条蛇（放在地图右侧，默认向左）
void initSnake2Random() {
    g_snake2.clear();
    g_dir2 = DIR_LEFT;

    int tries = 0;
    while (tries < 200) {
        ++tries;

        int hx = randInt(GRID_W / 2 + 4, GRID_W - 6);
        int hy = randInt(5, GRID_H - 6);

        // 蛇2朝左行进，蛇身向右延伸
        Cell a{ hx, hy };
        Cell b{ hx + 1, hy };
        Cell c{ hx + 2, hy };

        if (b.x >= GRID_W - 1 || c.x >= GRID_W - 1) continue;
        if (inSnake(a.x, a.y) || inSnake(b.x, b.y) || inSnake(c.x, c.y)) continue;

        g_snake2.push_back(a);
        g_snake2.push_back(b);
        g_snake2.push_back(c);
        g_currentScore2 = (int)g_snake2.size();
        return;
    }

    // 强制放在地图最右侧固定位置
    int hy = GRID_H / 2 + 3;
    g_snake2.push_back({ GRID_W - 8, hy });
    g_snake2.push_back({ GRID_W - 7, hy });
    g_snake2.push_back({ GRID_W - 6, hy });
    g_currentScore2 = (int)g_snake2.size();
}

// 生成墙（仅在难度>=5启用）
void generateWalls() {
    g_walls.clear();

    int target = wallCountByDifficulty();   // 目标墙数量
    int tries = 0;

    // 循环生成直到达到数量或超时
    while ((int)g_walls.size() < target && tries < target * 40) {
        ++tries;
        int x = randInt(1, GRID_W - 2);
        int y = randInt(1, GRID_H - 2);

        // 避开蛇、已有墙、蛇头附近
        if (inSnake(x, y) || inWalls(x, y)) continue;
        if (abs(x - g_snake.front().x) <= 2 && abs(y - g_snake.front().y) <= 2) continue;
        if (!g_snake2.empty() && abs(x - g_snake2.front().x) <= 2 && abs(y - g_snake2.front().y) <= 2) continue;

        g_walls.push_back({ x, y });
    }
}

// 生成一个食物（5% 金色，95% 普通）
void spawnOneFood() {
    int tries = 0;
    while (tries < 500) {
        ++tries;
        int x = randInt(1, GRID_W - 2);
        int y = randInt(1, GRID_H - 2);

        if (inSnake(x, y) || inWalls(x, y) || inFoods(x, y)) continue;

        bool golden = (randInt(1, 100) <= 5); // 5% 概率
        g_foods.push_back({ x, y, golden });
        return;
    }
}

// 查询某格对应食物增长值：0=无食物，1=普通，3=金色
int foodGrowAt(int x, int y) {
    for (const auto& f : g_foods) {
        if (f.x == x && f.y == y) {
            return f.golden ? 3 : 1;
        }
    }
    return 0;
}

// 删除指定格子的一个食物
void removeFoodAt(int x, int y) {
    for (int i = 0; i < (int)g_foods.size(); ++i) {
        if (g_foods[i].x == x && g_foods[i].y == y) {
            g_foods.erase(g_foods.begin() + i);
            return;
        }
    }
}

// 尝试设置方向（禁止反向）
void trySetDirection(Direction nd) {
    if (!isOpposite(g_dir, nd)) {
        g_dir = nd;
    }
}

// 尝试设置玩家2方向（禁止反向）
void trySetDirection2(Direction nd) {
    if (!isOpposite(g_dir2, nd)) {
        g_dir2 = nd;
    }
}

// 单人模式：单步移动 + 碰撞检测
bool stepSnakeSingle() {
    Cell next = g_snake.front();

    // 计算下一格
    if (g_dir == DIR_UP) next.y--;
    else if (g_dir == DIR_DOWN) next.y++;
    else if (g_dir == DIR_LEFT) next.x--;
    else if (g_dir == DIR_RIGHT) next.x++;

    // 1) 地图边界碰撞
    if (next.x < 0 || next.x >= GRID_W || next.y < 0 || next.y >= GRID_H) {
        return false;
    }

    // 2) 墙体碰撞
    if (inWalls(next.x, next.y)) {
        return false;
    }

    int grow = foodGrowAt(next.x, next.y);

    // 3) 自身碰撞（若本步不增长，允许踩到“将被移走”的尾巴）
    int selfLimit = (grow > 0) ? (int)g_snake.size() : (int)g_snake.size() - 1;
    for (int i = 0; i < selfLimit; ++i) {
        if (g_snake[i].x == next.x && g_snake[i].y == next.y) {
            return false;
        }
    }

    // 移动并处理增长
    g_snake.push_front(next);
    if (grow == 0) {
        g_snake.pop_back();
    }
    else {
        for (int i = 0; i < grow - 1; ++i) {
            g_snake.push_back(g_snake.back());
        }
        removeFoodAt(next.x, next.y);
    }

    g_currentScore = (int)g_snake.size();
    g_currentScore1 = g_currentScore;

    return true;
}

// 双人模式：同步移动；返回false表示该局结束
bool stepSnakeDouble() {
    g_dualLastP1Dead = false;
    g_dualLastP2Dead = false;
    g_dualLastHeadOn = false;

    Cell next1 = g_snake.front();
    Cell next2 = g_snake2.front();

    if (g_dir == DIR_UP) next1.y--;
    else if (g_dir == DIR_DOWN) next1.y++;
    else if (g_dir == DIR_LEFT) next1.x--;
    else if (g_dir == DIR_RIGHT) next1.x++;

    if (g_dir2 == DIR_UP) next2.y--;
    else if (g_dir2 == DIR_DOWN) next2.y++;
    else if (g_dir2 == DIR_LEFT) next2.x--;
    else if (g_dir2 == DIR_RIGHT) next2.x++;

    // 头撞头：同格相撞 或 正面相顶互换
    bool headOn = (next1.x == next2.x && next1.y == next2.y) ||
        (next1.x == g_snake2.front().x && next1.y == g_snake2.front().y &&
            next2.x == g_snake.front().x && next2.y == g_snake.front().y);

    if (headOn) {
        g_dualLastP1Dead = true;
        g_dualLastP2Dead = true;
        g_dualLastHeadOn = true;
        return false;
    }

    bool dead1 = false, dead2 = false;

    // 边界与墙碰撞
    if (next1.x < 0 || next1.x >= GRID_W || next1.y < 0 || next1.y >= GRID_H) dead1 = true;
    if (next2.x < 0 || next2.x >= GRID_W || next2.y < 0 || next2.y >= GRID_H) dead2 = true;
    if (!dead1 && inWalls(next1.x, next1.y)) dead1 = true;
    if (!dead2 && inWalls(next2.x, next2.y)) dead2 = true;

    int grow1 = dead1 ? 0 : foodGrowAt(next1.x, next1.y);
    int grow2 = dead2 ? 0 : foodGrowAt(next2.x, next2.y);

    deque<Cell> nextSnake1 = g_snake;
    deque<Cell> nextSnake2 = g_snake2;

    if (!dead1) {
        nextSnake1.push_front(next1);
        if (grow1 == 0) nextSnake1.pop_back();
        else {
            for (int i = 0; i < grow1 - 1; ++i) nextSnake1.push_back(nextSnake1.back());
        }
        // 自身碰撞
        for (int i = 1; i < (int)nextSnake1.size(); ++i) {
            if (nextSnake1[i].x == next1.x && nextSnake1[i].y == next1.y) {
                dead1 = true;
                break;
            }
        }
    }

    if (!dead2) {
        nextSnake2.push_front(next2);
        if (grow2 == 0) nextSnake2.pop_back();
        else {
            for (int i = 0; i < grow2 - 1; ++i) nextSnake2.push_back(nextSnake2.back());
        }
        // 自身碰撞
        for (int i = 1; i < (int)nextSnake2.size(); ++i) {
            if (nextSnake2[i].x == next2.x && nextSnake2[i].y == next2.y) {
                dead2 = true;
                break;
            }
        }
    }

    // 撞到对方身体（不含对方头）
    if (!dead1) {
        for (int i = 1; i < (int)nextSnake2.size(); ++i) {
            if (nextSnake2[i].x == next1.x && nextSnake2[i].y == next1.y) {
                dead1 = true;
                break;
            }
        }
    }
    if (!dead2) {
        for (int i = 1; i < (int)nextSnake1.size(); ++i) {
            if (nextSnake1[i].x == next2.x && nextSnake1[i].y == next2.y) {
                dead2 = true;
                break;
            }
        }
    }

    // 任意死亡，该局结束（不再提交移动）
    if (dead1 || dead2) {
        g_dualLastP1Dead = dead1;
        g_dualLastP2Dead = dead2;
        return false;
    }

    // 两者均存活，提交移动并吃食物
    g_snake = nextSnake1;
    g_snake2 = nextSnake2;

    if (grow1 > 0) removeFoodAt(next1.x, next1.y);
    if (grow2 > 0 && !(next2.x == next1.x && next2.y == next1.y)) removeFoodAt(next2.x, next2.y);

    g_currentScore1 = (int)g_snake.size();
    g_currentScore2 = (int)g_snake2.size();
    g_currentScore = g_currentScore1;

    return true;
}

// 记录本局分数到当前难度历史中
void recordCurrentScore() {
    g_scoreHistory[g_difficulty].push_back(g_currentScore);
}

// 结束单人局
void finishSingleRound() {
    g_lastRoundMode = MODE_SINGLE;
    g_lastSingleScore = g_currentScore;
    g_lastWinnerText = L"-";

    recordCurrentScore();
    g_state = STATE_GAME_OVER;
}

// 根据比分返回双人胜者：0=平局，1=玩家1，2=玩家2
int winnerByScoreInDual() {
    if (g_currentScore1 > g_currentScore2) return 1;
    if (g_currentScore2 > g_currentScore1) return 2;
    return 0;
}

// 结束双人局
// reasonType: 0=有人死亡, 1=时间到, 2=头撞头
void finishDualRound(int reasonType, bool p1Dead, bool p2Dead) {
    g_lastRoundMode = MODE_DOUBLE;

    int winner = 0;
    if (reasonType == 0) {
        if (p1Dead && !p2Dead) winner = 2;
        else if (p2Dead && !p1Dead) winner = 1;
        else winner = winnerByScoreInDual();
    }
    else {
        // 时间到 或 头撞头：按分数判定
        winner = winnerByScoreInDual();
    }

    if (winner == 1) {
        g_lastWinnerText = L"WASD 玩家";
        g_dualWinsByDifficulty[g_difficulty][0]++;
        g_dualWinsTotal[0]++;
    }
    else if (winner == 2) {
        g_lastWinnerText = L"方向键玩家";
        g_dualWinsByDifficulty[g_difficulty][1]++;
        g_dualWinsTotal[1]++;
    }
    else {
        g_lastWinnerText = L"平局";
    }

    g_state = STATE_GAME_OVER;
}

// 开始新游戏
void startNewGame() {
    g_roundMode = g_selectedMode;

    g_foods.clear();
    g_snake.clear();
    g_snake2.clear();

    initSnakeRandom();  // 初始化玩家1
    if (g_roundMode == MODE_DOUBLE) {
        initSnake2Random(); // 初始化玩家2
    }

    generateWalls();    // 生成墙
    spawnOneFood();     // 初始生成一个食物

    g_currentScore = (int)g_snake.size();
    g_currentScore1 = (int)g_snake.size();
    g_currentScore2 = (g_roundMode == MODE_DOUBLE) ? (int)g_snake2.size() : 0;
    g_dualLastP1Dead = g_dualLastP2Dead = g_dualLastHeadOn = false;

    DWORD now = GetTickCount();
    g_lastMoveTick = now;
    g_lastFoodSpawnTick = now;
    g_roundStartTick = now;

    g_state = STATE_RUNNING;    // 切换到游戏运行态
}

// ========================= 页面绘制 =========================

// 启动页（灰度 + 圆角信息框）
void drawStartMenu() {
    drawSceneBase(true);

    drawWalls(true);
    drawFoods(true);
    drawSnake(true);
    if (g_selectedMode == MODE_DOUBLE) drawSnake2(true);

    // 中间大圆角框
    int boxW = 760;
    int boxH = 360;
    int l = (WINDOW_WIDTH - boxW) / 2;
    int t = (WINDOW_HEIGHT - boxH) / 2;
    int r = l + boxW;
    int b = t + boxH;

    setfillcolor(RGB(248, 253, 255));
    setlinecolor(RGB(95, 150, 210));
    solidroundrect(l, t, r, b, 25, 25);
    roundrect(l, t, r, b, 25, 25);

    drawCenterTextLine(t + 28, L"欢迎来到 Retro Snake!", 42, RGB(50, 85, 185));

    // 左中右三个信息卡片
    int gap = 16;
    int cardY = t + 96;
    int cardH = 170;
    int cardW = (boxW - gap * 4) / 3;

    int c1l = l + gap;
    int c2l = c1l + cardW + gap;
    int c3l = c2l + cardW + gap;

    setfillcolor(RGB(232, 245, 255));
    setlinecolor(RGB(120, 170, 225));
    solidroundrect(c1l, cardY, c1l + cardW, cardY + cardH, 16, 16);

    setfillcolor(RGB(234, 252, 240));
    setlinecolor(RGB(120, 195, 145));
    solidroundrect(c2l, cardY, c2l + cardW, cardY + cardH, 16, 16);

    setfillcolor(RGB(255, 243, 229));
    setlinecolor(RGB(225, 170, 110));
    solidroundrect(c3l, cardY, c3l + cardW, cardY + cardH, 16, 16);

    drawTextInRect(c1l, cardY + 10, c1l + cardW, cardY + 70, L"难度设置", 25, RGB(40, 90, 155));
    drawTextInRect(c1l, cardY + 50, c1l + cardW, cardY + 112, L"数字键 1 - 9 / 0", 22, RGB(55, 80, 125));
    drawTextInRect(c1l, cardY + 90, c1l + cardW, cardY + 148, L"当前难度: " + to_wstring(g_difficulty), 24, RGB(30, 70, 120));

    drawTextInRect(c2l, cardY + 10, c2l + cardW, cardY + 70, L"模式设置", 25, RGB(35, 120, 65));
    drawTextInRect(c2l, cardY + 50, c2l + cardW, cardY + 112, L"按 F 切换", 22, RGB(45, 105, 65));
    drawTextInRect(c2l, cardY + 90, c2l + cardW, cardY + 148, L"当前: " + modeText(g_selectedMode), 24, RGB(30, 95, 55));

    drawTextInRect(c3l, cardY + 10, c3l + cardW, cardY + 70, L"开始/退出", 25, RGB(150, 95, 30));
    drawTextInRect(c3l, cardY + 50, c3l + cardW, cardY + 114, L"空格开始游戏", 24, RGB(130, 85, 35));
    drawTextInRect(c3l, cardY + 96, c3l + cardW, cardY + 148, L"Q 退出", 24, RGB(120, 80, 30));

    drawCenterTextLine(t + 300, L"提示：双人模式下，WASD 对战 方向键，限时 3:00", 20, RGB(70, 85, 120));
}

// 运行页（原色 + 分数）
void drawRunning() {
    drawSceneBase(false);
    drawWalls(false);
    drawFoods(false);
    drawSnake(false);
    if (g_roundMode == MODE_DOUBLE) {
        drawSnake2(false);
        drawDualHud();
    }
    else {
        drawSingleScore();
    }
}

// 结束页（灰度 + 成绩框）
void drawGameOver() {
    drawSceneBase(true);
    drawWalls(true);
    drawFoods(true);
    drawSnake(true);
    if (g_lastRoundMode == MODE_DOUBLE) drawSnake2(true);

    int boxW = 820;
    int boxH = 430;
    int l = (WINDOW_WIDTH - boxW) / 2;
    int t = (WINDOW_HEIGHT - boxH) / 2;
    int r = l + boxW;
    int b = t + boxH;

    setfillcolor(RGB(250, 253, 255));
    setlinecolor(RGB(100, 155, 210));
    solidroundrect(l, t, r, b, 25, 25);
    roundrect(l, t, r, b, 25, 25);

    drawCenterTextLine(t + 22, L"游戏结束", 42, RGB(185, 65, 90));

    // 上一局结果展示
    if (g_lastRoundMode == MODE_SINGLE) {
        drawCenterTextLine(t + 78, L"本轮得分: " + to_wstring(g_lastSingleScore), 30, RGB(50, 85, 170));
    }
    else {
        drawCenterTextLine(t + 78, L"获胜者：" + g_lastWinnerText, 30, RGB(50, 120, 75));
    }

    drawCenterTextLine(t + 118, L"当前难度: " + to_wstring(g_difficulty) + L"    当前模式: " + modeText(g_selectedMode), 23, RGB(70, 90, 120));

    // 中部卡片区：根据当前选择模式显示不同内容
    int paneTop = t + 152;
    int paneBottom = b - 24;
    int gap = 18;
    int leftPaneL = l + 26;
    int rightPaneR = r - 26;
    int paneW = (rightPaneR - leftPaneL - gap) / 2;
    int leftPaneR = leftPaneL + paneW;
    int rightPaneL = leftPaneR + gap;

    // 左栏：数据区（单人排名 / 双人胜场）
    setfillcolor(RGB(236, 248, 255));
    setlinecolor(RGB(120, 175, 225));
    solidroundrect(leftPaneL, paneTop, leftPaneR, paneBottom, 18, 18);

    if (g_selectedMode == MODE_SINGLE) {
        // 单人：历史排名
        auto top = getTop3DistinctScores(g_difficulty);
        wstring first = (top[0] == -1) ? L"-" : to_wstring(top[0]);
        wstring second = (top[1] == -1) ? L"-" : to_wstring(top[1]);
        wstring third = (top[2] == -1) ? L"-" : to_wstring(top[2]);

        drawTextInRect(leftPaneL, paneTop + 14, leftPaneR, paneTop + 58, L"历史排名（当前难度）", 24, RGB(45, 100, 160));

        drawTextInRect(leftPaneL + 10, paneTop + 68, leftPaneR - 10, paneTop + 106, L"1st: " + first, 28, RGB(35, 80, 140));
        drawTextInRect(leftPaneL + 10, paneTop + 108, leftPaneR - 10, paneTop + 146, L"2nd: " + second, 28, RGB(35, 80, 140));
        drawTextInRect(leftPaneL + 10, paneTop + 148, leftPaneR - 10, paneTop + 186, L"3rd: " + third, 28, RGB(35, 80, 140));

        drawTextInRect(leftPaneL, paneBottom - 54, leftPaneR, paneBottom - 18, L"注：双人模式成绩不计入该排名", 19, RGB(70, 110, 155));
    }
    else {
        // 双人：胜场统计
        int d1 = g_dualWinsByDifficulty[g_difficulty][0];
        int d2 = g_dualWinsByDifficulty[g_difficulty][1];
        int t1 = g_dualWinsTotal[0];
        int t2 = g_dualWinsTotal[1];

        drawTextInRect(leftPaneL, paneTop + 14, leftPaneR, paneTop + 58, L"双人胜场统计", 24, RGB(45, 120, 70));
        drawTextInRect(leftPaneL + 8, paneTop + 74, leftPaneR - 8, paneTop + 116,
            L"WASD 玩家：当前难度 " + to_wstring(d1) + L" 局", 22, RGB(30, 105, 60));
        drawTextInRect(leftPaneL + 8, paneTop + 116, leftPaneR - 8, paneTop + 158,
            L"WASD 玩家：总计 " + to_wstring(t1) + L" 局", 22, RGB(30, 105, 60));
        drawTextInRect(leftPaneL + 8, paneTop + 164, leftPaneR - 8, paneTop + 206,
            L"方向键玩家：当前难度 " + to_wstring(d2) + L" 局", 22, RGB(35, 115, 70));
        drawTextInRect(leftPaneL + 8, paneTop + 206, leftPaneR - 8, paneBottom - 18,
            L"方向键玩家：总计 " + to_wstring(t2) + L" 局", 22, RGB(35, 115, 70));
    }

    // 右栏：操作区
    setfillcolor(RGB(255, 245, 232));
    setlinecolor(RGB(230, 175, 115));
    solidroundrect(rightPaneL, paneTop, rightPaneR, paneBottom, 18, 18);

    drawTextInRect(rightPaneL, paneTop + 16, rightPaneR, paneTop + 56, L"操作说明", 25, RGB(145, 92, 35));
    drawTextInRect(rightPaneL + 8, paneTop + 70, rightPaneR - 8, paneTop + 112, L"数字键 1-9/0：切换难度", 22, RGB(132, 88, 34));
    drawTextInRect(rightPaneL + 8, paneTop + 118, rightPaneR - 8, paneTop + 160, L"F：切换单人/双人", 22, RGB(132, 88, 34));
    drawTextInRect(rightPaneL + 8, paneTop + 166, rightPaneR - 8, paneTop + 208, L"空格：开始下一局", 22, RGB(132, 88, 34));
    drawTextInRect(rightPaneL + 8, paneTop + 214, rightPaneR - 8, paneBottom - 18, L"Q：退出", 24, RGB(122, 82, 32));
}

// ========================= 输入处理 =========================

// 处理难度选择按键：1-9 -> 难度1-9；0 -> 难度10
void handleDifficultyKey(char ch) {
    if (ch >= '1' && ch <= '9') {
        g_difficulty = ch - '0';
    }
    else if (ch == '0') {
        g_difficulty = 10;
    }
}

// 切换菜单选择模式（单人/双人）
void toggleSelectedMode() {
    g_selectedMode = (g_selectedMode == MODE_SINGLE) ? MODE_DOUBLE : MODE_SINGLE;
}

// 开始菜单输入处理：空格开始，Q退出，数字选难度
void handleMenuInput() {
    while (_kbhit()) {
        char ch = _getch();

        if (ch == ' ')
            startNewGame();
        else if (ch == 'q' || ch == 'Q')
            g_quit = true;
        else if (ch == 'f' || ch == 'F')
            toggleSelectedMode();
        else
            handleDifficultyKey(ch);
    }
}

// 游戏中输入处理：WASD/方向键控制，Q退出
void handleRunningInput() {
    while (_kbhit()) {
        char ch = _getch();

        // 玩家1：WASD 控制
        if (ch == 'w' || ch == 'W') trySetDirection(DIR_UP);
        else if (ch == 's' || ch == 'S') trySetDirection(DIR_DOWN);
        else if (ch == 'a' || ch == 'A') trySetDirection(DIR_LEFT);
        else if (ch == 'd' || ch == 'D') trySetDirection(DIR_RIGHT);
        else if (ch == 'q' || ch == 'Q') g_quit = true;

        // 方向键（会先读到 0 或 -32）
        else if (ch == 0 || ch == -32) {
            char key2 = _getch();
            if (g_roundMode == MODE_DOUBLE) {
                if (key2 == 72) trySetDirection2(DIR_UP);        // ↑
                else if (key2 == 80) trySetDirection2(DIR_DOWN); // ↓
                else if (key2 == 75) trySetDirection2(DIR_LEFT); // ←
                else if (key2 == 77) trySetDirection2(DIR_RIGHT);// →
            }
            else {
                if (key2 == 72) trySetDirection(DIR_UP);         // 单人下兼容方向键控制
                else if (key2 == 80) trySetDirection(DIR_DOWN);
                else if (key2 == 75) trySetDirection(DIR_LEFT);
                else if (key2 == 77) trySetDirection(DIR_RIGHT);
            }
        }
    }
}

// 游戏结束输入处理：空格重开，Q退出，数字选难度
void handleGameOverInput() {
    while (_kbhit()) {
        char ch = _getch();

        if (ch == ' ')
            startNewGame();
        else if (ch == 'q' || ch == 'Q')
            g_quit = true;
        else if (ch == 'f' || ch == 'F')
            toggleSelectedMode();
        else
            handleDifficultyKey(ch);
    }
}

// ========================= 主函数 =========================
int main() {
    srand((unsigned)time(nullptr));

    // 初始化图形窗口，显示控制台便于调试
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT, EX_SHOWCONSOLE);

    BeginBatchDraw(); // 双缓冲，减少闪烁

    // 游戏主循环：直到 g_quit = true 退出
    while (!g_quit) {
        if (g_state == STATE_START_MENU) {      // 开始菜单
            handleMenuInput();
            drawStartMenu();
            FlushBatchDraw();
            Sleep(15);
        }
        else if (g_state == STATE_RUNNING) {    // 游戏运行
            handleRunningInput();

            DWORD now = GetTickCount();

            // 双人模式倒计时结束：按比分判胜
            if (g_roundMode == MODE_DOUBLE && now - g_roundStartTick >= ROUND_TIME_LIMIT_MS) {
                finishDualRound(1, false, false);
            }

            if (g_state != STATE_RUNNING) {
                drawGameOver();
                FlushBatchDraw();
                Sleep(15);
                continue;
            }

            // 定时生成食物
            if (now - g_lastFoodSpawnTick >= FOOD_SPAWN_INTERVAL) {
                spawnOneFood();
                g_lastFoodSpawnTick = now;
            }

            // 定时移动蛇
            int moveDelay = calcMoveDelayMs();
            if (now - g_lastMoveTick >= (DWORD)moveDelay) {
                bool alive = (g_roundMode == MODE_DOUBLE) ? stepSnakeDouble() : stepSnakeSingle();
                g_lastMoveTick = now;

                if (!alive) {
                    // 单人：死亡结束；双人：有人死亡或头撞头结束
                    if (g_roundMode == MODE_SINGLE) {
                        finishSingleRound();
                    }
                    else {
                        int reasonType = g_dualLastHeadOn ? 2 : 0;
                        finishDualRound(reasonType, g_dualLastP1Dead, g_dualLastP2Dead);
                    }
                }
            }

            drawRunning();
            FlushBatchDraw();
            Sleep(5);
        }
        else if (g_state == STATE_GAME_OVER) {  // 游戏结束
            handleGameOverInput();
            drawGameOver();
            FlushBatchDraw();
            Sleep(15);
        }
    }

    EndBatchDraw();
    closegraph();
    return 0;
}
