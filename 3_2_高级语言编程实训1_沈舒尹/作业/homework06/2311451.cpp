#include <graphics.h>
#include <time.h>    
#include <conio.h>   
#include <stdlib.h>  

#define BLOCK_WIDTH 30    // 单个小方块的宽度
#define num_X 12          // 游戏区域横向最大小方块数量
#define num_Y 20          // 纵向
#define x_origin 40       // 游戏区域左上角X坐标
#define y_origin 40       // 左上角Y坐标

// 定义7种俄罗斯方块的4种旋转形态
// [7种方块][4种旋转角度][4个小方块][x/y偏移]
static int Block[7][4][4][2] = {
	{/* 类型0：T型方块 */
		{{0, 0}, {-1, 0}, {1, 0}, {0, -1}},
		{{0, 0}, {1, 0}, {0, 1}, {0, -1}},
		{{0, 0}, {-1, 0}, {1, 0}, {0, 1}},
		{{0, 0}, {-1, 0}, {0, 1}, {0, -1}}
	},
	{/* 类型1：反S型方块 */
		{{0, 0},{0, -1},{1, -1},{-1, 0}},
		{{0, 0},{0, -1},{1, 0},{1, 1}},
		{{0, 0},{-1, 0},{0, -1},{1, -1}},
		{{0, 0},{0, -1},{1, 0},{1, 1}}
	},
	{/* 类型2：S型方块 */
		{{0, 0},{-1, -1},{0, -1},{1, 0}},
		{{0, 0},{1, 0},{0, 1},{1, -1}},
		{{0, 0},{-1, -1},{0, -1},{1, 0}},
		{{0, 0},{1, -1},{1, 0},{0, 1}}
	},
	{/* 类型3：I型长条方块 */
		{{0, 0}, {-2, 0}, {-1, 0}, {1, 0}},
		{{0, 0}, {0, -2}, {0, -1}, {0, 1}},
		{{0, 0}, {-2, 0}, {-1, 0}, {1, 0}},
		{{0, 0}, {0, -2}, {0, -1}, {0, 1}}
	},
	{/* 类型4：O型正方形方块 */
		{{0, 0}, {-1, -1}, {0, -1}, {-1, 0}},
		{{0, 0}, {-1, -1}, {0, -1}, {-1, 0}},
		{{0, 0}, {-1, -1}, {0, -1}, {-1, 0}},
		{{0, 0}, {-1, -1}, {0, -1}, {-1, 0}}
	},
	{/* 类型5：L型方块 */
		{{0, 0}, {1, -1}, {-1, 0}, {1, 0}},
		{{0, 0}, {0, -1}, {0, 1}, {1, 1}},
		{{0, 0}, {-1, 0}, {1, 0}, {-1, 1}},
		{{0, 0}, {0, -1}, {0, 1}, {-1, -1}}
	},
	{/* 类型6：J型方块 */
		{{0, 0}, {-1, 0}, {1, 0}, {1, 1}},
		{{0, 0}, {0, 1}, {0, -1}, {-1, 1}},
		{{0, 0}, {-1, 0}, {1, 0}, {-1, -1}},
		{{0, 0}, {0, 1}, {0, -1}, {1, -1}}
	}
};

// 7种方块对应的颜色
COLORREF block_color[7] = {
	RGB(113, 142, 1900),
	RGB(178, 215, 223),
	RGB(99, 163, 171),
	RGB(148, 86, 87),
	RGB(197, 137, 181),
	RGB(2241, 131, 113),
	RGB(178, 171, 186)
};

// 当前下落方块的属性
int current_x, current_y;  // 当前方块在游戏网格中的坐标
int type, rotate;           // type：方块类型；rotate：旋转状态
int LandedBlock[num_X][num_Y]; // 存储已落地方块的类型，0表示空格
int LandFlag;              // 方块落地标记：0=未落地，1=已落地

bool canGoLeft(int x, int y, int Type, int Rotate);    // 判断能否左移
bool canGoRight(int x, int y, int Type, int Rotate);   // 判断能否右移
bool canFall(int x, int y, int Type, int Rotate);      // 判断能否下落
bool canRotate(int x, int y, int Type, int Rotate);    // 判断能否旋转
void landBlock(int x, int y, int Type, int Rotate);    // 处理方块落地
void drawLandedBlocks();                               // 绘制所有已落地方块

// 绘制单个小方块
void drawUnitBlock(int x, int y, COLORREF color)
{
	int left, top, right, bottom;
	// 将网格坐标转换为屏幕像素坐标
	left = x_origin + x * BLOCK_WIDTH;
	top = y_origin + y * BLOCK_WIDTH;
	right = left + BLOCK_WIDTH;
	bottom = top + BLOCK_WIDTH;

	setlinecolor(RGB(245, 245, 245));
	setfillcolor(color);
	fillrectangle(left, top, right, bottom); 
}

// 绘制完整的俄罗斯方块
void drawTetrisBlock(int x, int y, int Type, int Rotate, COLORREF color)
{
	int new_X, new_Y;  // 小方块在游戏网格中的实际坐标
	// 循环绘制4个小方块
	for (int i = 0; i < 4; i++)
	{
		// 计算每个小方块的实际坐标 = 方块中心点 + 偏移量
		new_X = x + Block[Type][Rotate][i][0];
		new_Y = y + Block[Type][Rotate][i][1];
		drawUnitBlock(new_X, new_Y, color);
	}
}

// 随机生成新的俄罗斯方块
void genBlock()
{
	type = rand() % 7;    // 随机生成0-6的方块类型
	rotate = rand() % 4;  // 随机生成0-3的旋转状态
	current_x = 5;       // 初始横向位置（居中）
	current_y = 2;       // 初始纵向位置（顶部）
	LandFlag = 0;        // 重置落地标记为未落地
}

// 判断方块是否可以向左移动
bool canGoLeft(int x, int y, int Type, int Rotate)
{
	int i;
	int willBeX, willBeY;  // 左移后的坐标
	// 遍历方块的4个小方块
	for (i = 0; i < 4; i++)
	{
		willBeX = x + Block[Type][Rotate][i][0] - 1;
		willBeY = y + Block[Type][Rotate][i][1];

		// 判断是否超出左边界
		if (willBeX < 0)
			return false;

		// 判断是否与已落地方块碰撞
		if (willBeY >= 0 && willBeY < num_Y && LandedBlock[willBeX][willBeY] != 0)
			return false;
	}
	return true;  // 所有小方块都无碰撞，可以左移
}

// 判断方块是否可以向右移动
bool canGoRight(int x, int y, int Type, int Rotate)
{
	int i;
	int willBeX, willBeY;
	for (i = 0; i < 4; i++)
	{
		willBeX = x + Block[Type][Rotate][i][0] + 1;
		willBeY = y + Block[Type][Rotate][i][1];

		// 判断是否超出右边界
		if (willBeX >= num_X)
			return false;

		// 判断是否与已落地方块碰撞
		if (willBeY >= 0 && willBeY < num_Y && LandedBlock[willBeX][willBeY] != 0)
			return false;
	}
	return true;
}

// 判断方块是否可以向下下落
bool canFall(int x, int y, int Type, int Rotate)
{
	int i;
	int willBeX, willBeY;
	for (i = 0; i < 4; i++)
	{
		willBeX = x + Block[Type][Rotate][i][0];
		willBeY = y + Block[Type][Rotate][i][1] + 1;

		// 判断是否超出底部边界
		if (willBeY >= num_Y)
			return false;

		// 判断是否与已落地方块碰撞
		if (willBeY >= 0 && willBeX >= 0 && willBeX < num_X && LandedBlock[willBeX][willBeY] != 0)
			return false;
	}
	return true;
}

// 判断方块是否可以旋转
bool canRotate(int x, int y, int Type, int Rotate)
{
	int i;
	int willBeX, willBeY;
	int NewRotate = (Rotate + 1) % 4;  // 计算旋转后的状态
	for (i = 0; i < 4; i++)
	{
		willBeX = x + Block[Type][NewRotate][i][0];
		willBeY = y + Block[Type][NewRotate][i][1];

		// 判断旋转后是否超出游戏边界
		if (willBeX < 0 || willBeX >= num_X || willBeY >= num_Y)
			return false;

		// 判断旋转后是否与已落地方块碰撞
		if (willBeY >= 0 && LandedBlock[willBeX][willBeY] != 0)
			return false;
	}
	return true;
}

// 处理方块落地：将方块存入已落地数组并绘制
void landBlock(int x, int y, int Type, int Rotate)
{
	for (int i = 0; i < 4; i++)
	{
		int lx = x + Block[Type][Rotate][i][0];
		int ly = y + Block[Type][Rotate][i][1];
		// 坐标合法时，将方块类型存入数组
		if (lx >= 0 && lx < num_X && ly >= 0 && ly < num_Y)
		{
			LandedBlock[lx][ly] = Type + 1;
			drawUnitBlock(lx, ly, block_color[Type]);
		}
	}
}

// 绘制所有已落地的方块
void drawLandedBlocks()
{
	for (int x = 0; x < num_X; x++)
	{
		for (int y = 0; y < num_Y; y++)
		{
			if (LandedBlock[x][y] == 0)
				// 空格：绘制浅灰色背景方块
				drawUnitBlock(x, y, RGB(245, 245, 245));
			else
				// 有方块：绘制对应颜色的方块
				drawUnitBlock(x, y, block_color[LandedBlock[x][y] - 1]);
		}
	}
}

// 根据键盘输入移动方块
void moveBlock(char input)
{
	// 用背景色覆盖原方块，实现擦除效果
	drawTetrisBlock(current_x, current_y, type, rotate, RGB(245, 245, 245));

	if (input == 'a') // 左移
	{
		if (canGoLeft(current_x, current_y, type, rotate))
			current_x--;
	}
	else if (input == 'd') // 右移
	{
		if (canGoRight(current_x, current_y, type, rotate))
			current_x++;
	}
	else if (input == 's') // 加速下落
	{
		if (canFall(current_x, current_y, type, rotate))
			current_y++;
		else
			LandFlag = 1;  // 无法下落，标记为落地
	}
	else if (input == 'w') // 旋转
	{
		if (canRotate(current_x, current_y, type, rotate))
			rotate = (rotate + 1) % 4;
	}

	// 如果方块已落地
	if (LandFlag)
	{
		landBlock(current_x, current_y, type, rotate);  // 执行落地逻辑
		genBlock();                                     // 生成新方块
	}

	// 重新绘制所有方块
	drawLandedBlocks();
	drawTetrisBlock(current_x, current_y, type, rotate, block_color[type]);
}

int main()
{
	// 初始化图形窗口，尺寸根据游戏区域自动计算
	initgraph(x_origin * 2 + num_X * BLOCK_WIDTH + 2, y_origin * 2 + num_Y * BLOCK_WIDTH + 2);
	setbkcolor(RGB(245, 245, 245));
	cleardevice();                 
	setlinecolor(BLACK);           
	// 绘制游戏区域外框
	rectangle(x_origin - 1, y_origin - 1, x_origin + num_X * BLOCK_WIDTH + 1, y_origin + num_Y * BLOCK_WIDTH + 1);

	srand((unsigned int)time(NULL));
	genBlock();                     // 生成第一个方块
	drawLandedBlocks();             // 绘制初始空白游戏区域
	drawTetrisBlock(current_x, current_y, type, rotate, block_color[type]); // 绘制第一个方块

	char input;
	while (1)
	{
		input = _getch();
		if (input != 'q')
		{
			moveBlock(input);
		}
		else  // q键：退出游戏
		{
			break;
		}
	}

	_getch();
	closegraph();
	return 0;
}