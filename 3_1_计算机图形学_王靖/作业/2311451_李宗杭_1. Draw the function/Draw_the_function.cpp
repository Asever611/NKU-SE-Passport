
#include <cmath>
#include <iostream>
#include <GL/glut.h>
// 窗口尺寸
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// 变换参数
const float A = 20.0f;   // x轴缩放因子 - 放大x坐标
const float B = 400.0f;  // x轴平移 - 将原点移到窗口中心
const float C = 200.0f;  // y轴缩放因子
const float D = 300.0f;  // y轴平移

// 坐标变换函数
float transformX(float x) {
    return A * x + B;
}

float transformY(float y) {
    return C * y + D;
}

// 要绘制的函数
float f(float x) {
    if (fabs(x) < 1e-6) { // 避免除以0
        return 1.0f;
    }
    return sin(x) / x;
}

void drawCoordinateSystem() {
    glColor3f(0.5f, 0.5f, 0.5f); // 灰色坐标轴

    // 绘制x轴
    glBegin(GL_LINES);
    glVertex2f(transformX(-20), transformY(0));
    glVertex2f(transformX(20), transformY(0));
    glEnd();

    // 绘制y轴
    glBegin(GL_LINES);
    glVertex2f(transformX(0), transformY(-0.5));
    glVertex2f(transformX(0), transformY(1.0));
    glEnd();

    // 绘制刻度
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_LINES);

    // x轴刻度
    for (int i = -20; i <= 20; i += 5) {
        glVertex2f(transformX(i), transformY(-0.02));
        glVertex2f(transformX(i), transformY(0.02));
    }

    // y轴刻度
    for (float y = -0.5; y <= 1.0; y += 0.25) {
        glVertex2f(transformX(-0.5), transformY(y));
        glVertex2f(transformX(0.5), transformY(y));
    }
    glEnd();
}

void drawFunction() {
    glColor3f(0.0f, 0.0f, 1.0f); // 蓝色函数曲线
    glLineWidth(2.0f);

    glBegin(GL_LINE_STRIP);

    float step = 0.1f; // 采样步长
    for (float x = -20.0f; x <= 20.0f; x += step) {
        float y = f(x);
        glVertex2f(transformX(x), transformY(y));
    }

    glEnd();
    glLineWidth(1.0f);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // 设置视口为整个窗口
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);

    drawCoordinateSystem();
    drawFunction();

    glutSwapBuffers();
}

void init() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // 白色背景
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 27: // ESC键退出
        exit(0);
        break;
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("f(x) = sin(x)/x");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);

    std::cout << "绘制函数: f(x) = sin(x)/x, x ∈ [-20, 20]" << std::endl;
    std::cout << "使用的坐标变换: sx = " << A << " * x + " << B << ", sy = " << C << " * y + " << D << std::endl;
    std::cout << "按ESC键退出" << std::endl;

    glutMainLoop();

    return 0;
}