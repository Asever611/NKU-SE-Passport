
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <GL/glut.h>
using namespace std;

const int INSIDE = 0;  // 0000：窗口内
const int LEFT = 1;  // 0001：左侧
const int RIGHT = 2;  // 0010：右侧
const int BOTTOM = 4;  // 0100：下方
const int TOP = 8;  // 1000：上方

// 点结构体
struct Point {
    float x, y;
    Point(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}
};

// 线段结构体
struct Line {
    Point p1, p2;
    Line(Point p1, Point p2) : p1(p1), p2(p2) {}
};

vector<Line> testLines;       // 存储所有测试线段
const Point clipWindowMin(-0.3f, -0.3f);  // 裁剪窗口左下角
const Point clipWindowMax(0.3f, 0.3f);   // 裁剪窗口右上角


/**
 * 计算点的区域编码
 */
int computeCode(float x, float y, float xmin, float ymin, float xmax, float ymax) {
    int code = INSIDE;  // 初始化为窗口内
    // 左侧判断
    if (x < xmin)
        code |= LEFT;
    // 右侧判断
    else if (x > xmax)
        code |= RIGHT;
    // 下方判断
    if (y < ymin)
        code |= BOTTOM;
    // 上方判断
    else if (y > ymax)
        code |= TOP;
    return code;
}

/**
 * Cohen-Sutherland线段裁剪函数
 */
bool cohenSutherlandClip(Point& p1, Point& p2, float xmin, float ymin, float xmax, float ymax) {
    int code1 = computeCode(p1.x, p1.y, xmin, ymin, xmax, ymax);  // 起点编码
    int code2 = computeCode(p2.x, p2.y, xmin, ymin, xmax, ymax);  // 终点编码
    bool accept = false;  // 是否有部分在窗口内

    while (true) {
        // 完全接受（两端点都在窗口内，(code1 | code2) = 0）
        if (!(code1 | code2)) {
            accept = true;
            break;
        }
        // 完全拒绝（两端点在窗口同一外侧，(code1 & code2) != 0）
        else if (code1 & code2) {
            break;
        }
        // 相交
        else {
            accept = true;

            int outCode;    // 存储在窗口外的端点编码
            float x, y;     // 交点坐标

            // 选择在窗口外的端点（优先选p1，p1在窗口内则选p2）
            outCode = (code1 != INSIDE) ? code1 : code2;

            // 与上边界（y=ymax）求交
            if (outCode & TOP) {
                x = p1.x + (p2.x - p1.x) * (ymax - p1.y) / (p2.y - p1.y);
                y = ymax;
            }
            // 与下边界（y=ymin）求交
            else if (outCode & BOTTOM) {
                x = p1.x + (p2.x - p1.x) * (ymin - p1.y) / (p2.y - p1.y);
                y = ymin;
            }
            // 与右边界（x=xmax）求交
            else if (outCode & RIGHT) {
                y = p1.y + (p2.y - p1.y) * (xmax - p1.x) / (p2.x - p1.x);
                x = xmax;
            }
            // 4. 与左边界（x=xmin）求交
            else if (outCode & LEFT) {
                y = p1.y + (p2.y - p1.y) * (xmin - p1.x) / (p2.x - p1.x);
                x = xmin;
            }

            // 用交点替换窗口外的端点，并更新该端点的编码
            if (outCode == code1) {
                p1.x = x;
                p1.y = y;
                code1 = computeCode(p1.x, p1.y, xmin, ymin, xmax, ymax);
            }
            else {
                p2.x = x;
                p2.y = y;
                code2 = computeCode(p2.x, p2.y, xmin, ymin, xmax, ymax);
            }
        }
    }
    return accept;
}

/**
 * 生成4类测试线段
 * 1. 完全在窗外（全黑）；
 * 2. 完全在窗内（全红）；
 * 3. 一端在窗内、一端在窗外（黑+红）；
 * 4. 两端在窗外但横穿窗口（黑+红+黑）
 */
void generateTestLines() {
    testLines.clear();

    // 1 完全在窗外
    testLines.emplace_back(Point(-0.8f, 0.5f), Point(-0.1f, 0.6f));

    // 2 完全在窗内
    testLines.emplace_back(Point(-0.2f, -0.2f), Point(0.2f, -0.1f));

    // 3 一端在窗内、一端在窗外
    testLines.emplace_back(Point(0.1f, 0.1f), Point(0.2f, 0.5f));

    // 4. 两端在窗外但横穿窗口
    testLines.emplace_back(Point(-0.6f, 0.3f), Point(0.6f, -0.2f));
}

/**
 * 绘制裁剪窗口
 */
void drawClipWindow() {
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(clipWindowMin.x, clipWindowMin.y);
    glVertex2f(clipWindowMax.x, clipWindowMin.y);
    glVertex2f(clipWindowMax.x, clipWindowMax.y);
    glVertex2f(clipWindowMin.x, clipWindowMax.y);
    glEnd();
}

/**
 * 绘制所有测试线段
 */
void drawLines() {
    glLineWidth(1.5f);
    float xmin = clipWindowMin.x, ymin = clipWindowMin.y;
    float xmax = clipWindowMax.x, ymax = clipWindowMax.y;

    for (const auto& line : testLines) {
        Point p1 = line.p1;  // 原始起点
        Point p2 = line.p2;  // 原始终点
        Point clippedP1 = p1; // 裁剪后起点
        Point clippedP2 = p2; // 裁剪后终点

        // 步骤1：绘制线段的窗外部分（黑色）
        glColor3f(0.0f, 0.0f, 0.0f);
        if (cohenSutherlandClip(clippedP1, clippedP2, xmin, ymin, xmax, ymax)) {
            // 情况A：线段有部分在窗内, 绘制“原始起点→裁剪起点”和“裁剪终点→原始终点”（窗外部分）
            if (!(computeCode(p1.x, p1.y, xmin, ymin, xmax, ymax) & INSIDE)) {
                glBegin(GL_LINES);
                glVertex2f(p1.x, p1.y);
                glVertex2f(clippedP1.x, clippedP1.y);
                glEnd();
            }
            if (!(computeCode(p2.x, p2.y, xmin, ymin, xmax, ymax) & INSIDE)) {
                glBegin(GL_LINES);
                glVertex2f(clippedP2.x, clippedP2.y);
                glVertex2f(p2.x, p2.y);
                glEnd();
            }
            // 步骤2：绘制线段的窗内部分（红色）
            glColor3f(1.0f, 0.0f, 0.0f);  // 颜色：红色
            glBegin(GL_LINES);
            glVertex2f(clippedP1.x, clippedP1.y);
            glVertex2f(clippedP2.x, clippedP2.y);
            glEnd();
        }
        else {
            // 情况B：线段完全在窗外 → 直接绘制整条黑色线段
            glBegin(GL_LINES);
            glVertex2f(p1.x, p1.y);
            glVertex2f(p2.x, p2.y);
            glEnd();
        }
    }
}

/**
 * OpenGL显示回调函数, 负责绘制所有内容
 */
void display() {
    glClear(GL_COLOR_BUFFER_BIT);  // 清空颜色缓冲区
    drawClipWindow();              // 绘制裁剪窗口
    drawLines();                   // 绘制所有测试线段
    glutSwapBuffers();             // 交换双缓冲区, 避免绘制闪烁
}

/**
 * 窗口大小调整函数
 */
void reshape(int width, int height) {
    glViewport(0, 0, width, height);  // 设置视口（占满整个窗口）
    glMatrixMode(GL_PROJECTION);      // 切换到投影矩阵
    glLoadIdentity();                 // 重置投影矩阵
    gluOrtho2D(-1.0f, 1.0f, -1.0f, 1.0f);  // 正交投影（标准化坐标：x∈[-1,1], y∈[-1,1]）
    glMatrixMode(GL_MODELVIEW);       // 切换回模型视图矩阵
}

int main(int argc, char** argv) {
    // 初始化GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);  
    glutInitWindowSize(800, 600);                 
    glutCreateWindow("Cohen-Sutherland Line Clipping");  

    // 设置OpenGL环境
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);   // 背景色：白色
    generateTestLines();                    // 生成初始测试线段

    // 注册回调函数
    glutDisplayFunc(display);    // 显示回调
    glutReshapeFunc(reshape);    // 窗口调整回调

    // 进入GLUT主循环
    glutMainLoop();
    return 0;
}