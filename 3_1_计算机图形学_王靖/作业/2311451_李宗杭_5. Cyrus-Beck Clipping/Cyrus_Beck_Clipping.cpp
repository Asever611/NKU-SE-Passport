#include <vector>
#include <utility>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <GL/glut.h>
using namespace std;

// 定义点结构
struct Point {
    float x, y;
    Point(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}
};

// 全局变量
vector<Point> clipPolygon;       // 裁剪窗口
vector<pair<Point, Point>> lines;// 待裁剪线段集合
bool isClipping = false;         // 是否已终止顶点添加
int windowWidth = 800, windowHeight = 600;

// 向量点积计算
float dotProduct(const Point& a, const Point& b) {
    return a.x * b.x + a.y * b.y;
}

// 向量减法
Point subtract(const Point& a, const Point& b) {
    return Point(a.x - b.x, a.y - b.y);
}

// 向量数乘
Point multiply(const Point& a, float k) {
    return Point(a.x * k, a.y * k);
}

// 检查点是否在凸多边形内部（辅助判断，用于平行边情况）
bool isPointInsideConvexPolygon(const Point& p, const vector<Point>& poly) {
    int n = poly.size();
    for (int i = 0; i < n; ++i) {
        Point edge = subtract(poly[(i + 1) % n], poly[i]);
        Point normal(-edge.y, edge.x); // 内法向量（凸多边形逆时针顶点顺序）
        Point pToEdge = subtract(p, poly[i]);
        if (dotProduct(normal, pToEdge) < 0) {
            return false;
        }
    }
    return true;
}

// 线段裁剪核心算法
bool cyrusBeckClip(const Point& P0, const Point& P1, const vector<Point>& clipPoly, Point& P0_new, Point& P1_new) {
    int n = clipPoly.size();
    if (n < 3) return false; // 裁剪窗口至少3个顶点

    float t_E = 0.0f; // 进入参数
    float t_L = 1.0f; // 离开参数
    Point D = subtract(P1, P0); // 线段方向向量

    // 遍历裁剪窗口的每条边
    for (int i = 0; i < n; ++i) {
        Point PEi = clipPoly[i];                          // 边i上的点（取顶点）
        Point Ei_next = clipPoly[(i + 1) % n];                // 边i的下一个顶点
        Point edge = subtract(Ei_next, PEi);              // 边i的方向向量
        Point Ni(-edge.y, edge.x);                        // 边i的内法向量（逆时针顶点顺序）

        float NdotD = dotProduct(Ni, D);                  // Ni · D
        Point P0_minus_PEi = subtract(P0, PEi);           // P0 - PEi
        float NdotP0PEi = dotProduct(Ni, P0_minus_PEi);   // Ni · (P0 - PEi)

        // 情况1：线段与边平行
        if (NdotD == 0.0f) {
            // 线段在边的外部 → 拒绝整个线段
            if (NdotP0PEi < 0.0f) {
                return false;
            }
            continue; // 平行且在内部，继续检查其他边
        }

        // 交点参数t
        float t = (-NdotP0PEi) / NdotD;

        // 情况2：线段从外向内穿越
        if (NdotD > 0.0f) {
            if (t > t_L) {
                return false;
            }
            t_E = max(t_E, t); // 更新进入参数
        }
        // 情况3：线段从内向外穿越
        else {
            if (t < t_E) { 
                return false;
            }
            t_L = min(t_L, t); // 更新离开参数
        }
    }

    // 是否存在有效裁剪线段
    if (t_E > t_L) {
        return false;
    }

    // 裁剪后的线段端点
    P0_new = Point(P0.x + t_E * D.x, P0.y + t_E * D.y);
    P1_new = Point(P0.x + t_L * D.x, P0.y + t_L * D.y);
    return true;
}

// 随机生成线段
void generateRandomLines(int count = 10) {
    lines.clear();
    srand(time(0));
    for (int i = 0; i < count; ++i) {
        Point p0(
            rand() % windowWidth,
            rand() % windowHeight
        );
        Point p1(
            rand() % windowWidth,
            rand() % windowHeight
        );
        lines.emplace_back(p0, p1);
    }
}

// 显示回调函数
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // 绘制裁剪窗口
    if (clipPolygon.size() >= 2) {
        glColor3f(0.0f, 1.0f, 0.0f);
        glBegin(GL_LINE_LOOP);
        for (const auto& p : clipPolygon) {
            glVertex2f(p.x, p.y);
        }
        glEnd();
    }

    // 绘制原始线段和裁剪后线段
    if (isClipping && !lines.empty()) {
        for (const auto& line : lines) {
            Point P0 = line.first;
            Point P1 = line.second;

            // 绘制原始线段
            glColor3f(1.0f, 0.0f, 0.0f);
            glBegin(GL_LINES);
            glVertex2f(P0.x, P0.y);
            glVertex2f(P1.x, P1.y);
            glEnd();

            // 裁剪线段
            Point P0_new, P1_new;
            if (cyrusBeckClip(P0, P1, clipPolygon, P0_new, P1_new)) {
                // 绘制裁剪后线段
                glColor3f(0.0f, 0.0f, 1.0f);
                glLineWidth(2.0f);
                glBegin(GL_LINES);
                glVertex2f(P0_new.x, P0_new.y);
                glVertex2f(P1_new.x, P1_new.y);
                glEnd();
                glLineWidth(1.0f); // 恢复线宽
            }
        }
    }

    glutSwapBuffers();
}

// 鼠标回调函数（点击添加顶点）
void mouse(int button, int state, int x, int y) {
    if (!isClipping && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        y = windowHeight - y;
        clipPolygon.emplace_back((float)x, (float)y);
        cout << "添加顶点 (" << x << ", " << y << ")" << endl;
        glutPostRedisplay();
    }
}

// 键盘回调函数（按下'c'键终止顶点添加并生成线段）
void keyboard(unsigned char key, int x, int y) {
    if (key == 'c' || key == 'C') {
        if (clipPolygon.size() >= 3) {
            isClipping = true;
            generateRandomLines(15); // 生成15条随机线段
        }
        glutPostRedisplay();
    }
    else if (key == 'r' || key == 'R') {
        // 重新创建裁剪窗口
        clipPolygon.clear();
        lines.clear();
        isClipping = false;
        glutPostRedisplay();
    }
}

// 窗口大小调整回调
void reshape(int width, int height) {
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // 正交投影
    gluOrtho2D(0.0f, (float)width, 0.0f, (float)height);
    glMatrixMode(GL_MODELVIEW);
}

// 初始化OpenGL设置
void init() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // 背景色
    glEnable(GL_LINE_SMOOTH); // 抗锯齿
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Cyrus-Beck Clipping Algorithm");

    // 注册回调函数
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);

    init();

    glutMainLoop();
    return 0;
}