#include <graphics.h>
#include <math.h>   // 用于三角函数相关计算
#include <conio.h>  // 用于 _kbhit()、_getch()
#include <windows.h>// 用于 Sleep()、GetTickCount()

int main()
{
    int window_width = 640, window_height = 480;    // 窗口大小
    int count = 60;                     // 要绘制的圆形总数
    float DEGREE_SPAN = 6.2830 / count; // 每个圆形之间的弧度间隔
    int R_CENTER = 80;                  // 中心轨迹圆的半径
    int R = 130;                        // 小圆自身的半径
    int i = 0, center_x, center_y;      // 循环计数器，小圆的圆心坐标
    float time_phase = 0;               // 时间相位变量

    // 初始化图形窗口
    initgraph(window_width, window_height);
    // 启用双缓冲绘图
    setbkmode(TRANSPARENT);

    // 设置图形窗口的背景色
    setbkcolor(BLACK);
    cleardevice(); // 初始化背景

    // 记录上一帧的时间，用于计算帧间隔
    ULONGLONG last_time = GetTickCount();

    // 主循环，实现动画效果，直到检测到任意按键按下时退出
    while (!_kbhit())
    {
        // 处理窗口消息（窗口拖动不中断）
        ExMessage msg;
        // 循环处理所有待处理的窗口消息（鼠标移动、窗口拖动等）
        while (peekmessage(&msg, EX_MOUSE | EX_KEY)) {}

        // 计算当前帧与上一帧的时间差，保证动画速度稳定
        ULONGLONG current_time = GetTickCount();
        float delta_time = (current_time - last_time) / 1000.0f; // 转换为秒
        last_time = current_time;

        // 开始批量绘制（绘制到内存缓冲区）
        BeginBatchDraw(); 
        cleardevice();    // 清空缓冲区

        // 循环绘制60个圆形，逐帧更新位置和颜色
        for (i = 0; i < count; i++)
        {
            // 计算第 i 个小圆的圆心坐标
            center_x = window_width / 2 + R_CENTER * cos(DEGREE_SPAN * i);
            center_y = window_height / 2 + R_CENTER * sin(DEGREE_SPAN * i);

            // 结合圆形位置和时间相位计算颜色相位
            float phase = (float)i / count * 2 * 3.14159 + time_phase;

            // RGB三色分量循环渐变：利用sin函数的周期性生成0~1的波动值
            // 180为基础亮度，75为波动幅度（避免过暗）
            // 三个颜色分量的相位差 2.0944（2PAI/3）和 4.18879（4PAI/3），保证RGB循环渐变
            int r = (int)(180 + 75 * sin(phase));
            int g = (int)(180 + 75 * sin(phase + 2.0944));
            int b = (int)(180 + 75 * sin(phase + 4.18879));

            // 颜色值边界修正，确保RGB分量在 0~255 范围内
            r = r < 0 ? 0 : (r > 255 ? 255 : r);
            g = g < 0 ? 0 : (g > 255 ? 255 : g);
            b = b < 0 ? 0 : (b > 255 ? 255 : b);

            // 设置线条颜色并绘制圆形
            setlinecolor(RGB(r, g, b));
            circle(center_x, center_y, R);
        }

        // 按时间增量更新相位
        float rate = 3.0f;
        time_phase += delta_time * rate;
        // 相位取模，防止数值过大
        if (time_phase > 2 * 3.14159) time_phase -= 2 * 3.14159;

        EndBatchDraw(); // 结束批量绘制（将缓冲区内容一次性显示到窗口）

        Sleep(1);
    }

    // 等待用户按下任意键关闭
    _getch();
    // 关闭图形窗口，释放资源
    closegraph();
    return 0;
}