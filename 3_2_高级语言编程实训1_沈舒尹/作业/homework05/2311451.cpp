#include <graphics.h>
#include <conio.h>
// 方块单位宽度
#define BLOCK_WIDTH 30
// 绘制原点坐标
#define X_origin 50
#define Y_origin 50
// 窗口大小
#define WIN_WIDTH 600
#define WIN_HEIGHT 900

// 7种方块 × 4种旋转 × 4个小方块 × 2维坐标
int blocks[7][4][4][2] = {
    //"T"型方块
    {{{0, 0}, {-1, 0}, {0, -1}, {1, 0}},
     {{0, 0}, {0, -1}, {1, 0}, {0, 1}},
     {{0, 0}, {1, 0}, {0, 1}, {-1, 0}},
     {{0, 0}, {0, 1}, {-1, 0}, {0, -1}}},
    //反"Z"型方块
    {{{0, 0}, {-1, 0}, {0, -1}, {1, -1}},
     {{0, 0}, {0, -1}, {1, 0}, {1, 1}},
     {{0, 0}, {1, 0}, {0, 1}, {-1, 1}},
     {{0, 0}, {0, 1}, {-1, 0}, {-1, -1}}},
    //"Z"型方块
    {{{0, 0}, {-1, -1}, {0, -1}, {1, 0}},
     {{0, 0}, {1, -1}, {1, 0}, {0, 1}},
     {{0, 0}, {1, 1}, {0, 1}, {-1, 0}},
     {{0, 0}, {-1, 1}, {-1, 0}, {0, -1}}},
    //"一"型方块
    {{{0, 0}, {-2, 0}, {-1, 0}, {1, 0}},
     {{0, 0}, {0, -2}, {0, -1}, {0, 1}},
     {{0, 0}, {2, 0}, {1, 0}, {-1, 0}},
     {{0, 0}, {0, 2}, {0, 1}, {0, -1}}},
    //"方块"型方块
    {{{0, 0}, {-1, 0}, {-1, -1}, {0, -1}},
     {{0, 0}, {0, -1}, {1, -1}, {1, 0}},
     {{0, 0}, {1, 0}, {1, 1}, {0, 1}},
     {{0, 0}, {0, 1}, {-1, 1}, {-1, 0}}},
    //"L"型方块
    {{{0, 0}, {-1, 0}, {1, 0}, {1, -1}},
     {{0, 0}, {0, -1}, {0, 1}, {1, 1}},
     {{0, 0},{1, 0}, {-1, 0}, {-1, 1}},
     {{0, 0}, {0, 1}, {0, -1}, {-1, -1}}},
    //反"L"型方块
    {{{0, 0}, {-1, 0}, {1, 0}, {1, 1}},
     {{0, 0}, {0, -1}, {0, 1}, {-1, 1}},
     {{0, 0}, {1, 0}, {-1, 0}, {-1, -1}},
     {{0, 0}, {0, 1}, {0, -1}, {1, -1}}}
};

// 7种方块对应的颜色
COLORREF blockColors[7] = {
    RGB(128, 0, 128),  // T型 - 紫色
    RGB(255, 0, 0),    // 反Z型 - 红色
    RGB(255, 165, 0),  // Z型 - 橙色
    RGB(0, 255, 255),  // 一型 - 青色
    RGB(255, 255, 0),  // 方块型 - 黄色
    RGB(0, 0, 255),    // L型 - 蓝色
    RGB(0, 255, 0)     // 反L型 - 绿色
};

// 绘制单个单位方块
void drawUnitBlock(int x, int y, COLORREF color)
{
    // 计算方块坐标
    int left = x - BLOCK_WIDTH / 2;
    int top = y - BLOCK_WIDTH / 2;

    // 设置填充颜色
    setfillcolor(color);
    // 绘制填充矩形
    fillrectangle(left, top, left + BLOCK_WIDTH - 1, top + BLOCK_WIDTH - 1);
    // 设置边框颜色
    setlinecolor(RGB(0, 0, 0));
    // 绘制矩形边框
    rectangle(left, top, left + BLOCK_WIDTH - 1, top + BLOCK_WIDTH - 1);
}

// 绘制单个旋转状态的俄罗斯方块
// 参数：方块类型i，旋转状态j，基准位置baseX, baseY
void drawTetrisBlocks(int i, int j, int baseX, int baseY)
{
    // 遍历当前方块的4个小单位方块
    for (int k = 0; k < 4; k++) {
        // 计算每个小方块的实际坐标
        int x = baseX + blocks[i][j][k][0] * BLOCK_WIDTH;
        int y = baseY + blocks[i][j][k][1] * BLOCK_WIDTH;
        // 绘制单个小方块
        drawUnitBlock(x, y, blockColors[i]);
    }
}

int main() {
    // 初始化图形窗口
    initgraph(WIN_WIDTH, WIN_HEIGHT);
    // 设置背景色
    setbkcolor(RGB(240, 240, 240));
    cleardevice();

    const int COLS = 4;          // 每行显示4个方块
    const int BLOCK_SPACING = 4; // 方块之间的间距格子数
    int totalWidth = COLS * BLOCK_SPACING * BLOCK_WIDTH;
    int totalHeight = 7 * BLOCK_SPACING * BLOCK_WIDTH;

    // 计算整体居中的起始坐标
    int startX = (WIN_WIDTH - totalWidth) / 2 + BLOCK_WIDTH * 2;
    int startY = (WIN_HEIGHT - totalHeight) / 2 + BLOCK_WIDTH * 2;

    // 循环绘制所有方块（7种 × 4种旋转）
    int row = 0, col = 0;
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 4; j++) {
            int baseX = startX + col * BLOCK_SPACING * BLOCK_WIDTH;
            int baseY = startY + row * BLOCK_SPACING * BLOCK_WIDTH;

            drawTetrisBlocks(i, j, baseX, baseY);

            col++;
            if (col >= COLS) {
                col = 0;
                row++;
            }
        }
    }

    _getch();
    closegraph();
    return 0;
}