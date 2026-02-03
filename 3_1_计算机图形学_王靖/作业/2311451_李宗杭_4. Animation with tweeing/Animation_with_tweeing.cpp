#include <vector>
#include <utility>
#include <cmath>
#include <algorithm>
#include <GL/glut.h>
using namespace std;

// 全局参数：插值进度、方向控制、暂停状态
float t = 0.0f;
bool increasing = true;
bool paused = false;
const double PI = 3.14159265358979323846;

// 计算两点之间的欧氏距离平方
float distanceSquared(const pair<float, float>& p1, const pair<float, float>& p2) {
    float dx = p1.first - p2.first;
    float dy = p1.second - p2.second;
    return dx * dx + dy * dy;
}

// 计算点集的质心
pair<float, float> computeCentroid(const vector<pair<float, float>>& points) {
    float sumX = 0.0f, sumY = 0.0f;
    for (const auto& p : points) {
        sumX += p.first;
        sumY += p.second;
    }
    return { sumX / points.size(), sumY / points.size() };
}

// 计算点相对于质心的极角
float computeAngle(const pair<float, float>& point, const pair<float, float>& centroid) {
    float dx = point.first - centroid.first;
    float dy = point.second - centroid.second;
    return atan2(dy, dx);
}

// 生成心形坐标点
vector<pair<float, float>> generateHeart(int numPoints) {
    vector<pair<float, float>> points;
    points.reserve(numPoints);

    for (int i = 0; i < numPoints; i++) {
        float t = 2.0f * PI * i / numPoints;

        // 标准心形参数方程
        float x = 16.0f * pow(sin(t), 3);
        float y = 13.0f * cos(t) - 5.0f * cos(2.0f * t)
            - 2.0f * cos(3.0f * t) - cos(4.0f * t);

        // 归一化到[-1, 1]范围
        x /= 18.0f;
        y /= 20.0f;

        // 轻微调整使形状更美观
        x *= 0.9f;
        y *= 0.9f;

        points.emplace_back(x, y);
    }
    // 计算质心并按极角排序
    auto centroid = computeCentroid(points);
    sort(points.begin(), points.end(),
        [centroid](const pair<float, float>& a, const pair<float, float>& b) {
            return computeAngle(a, centroid) < computeAngle(b, centroid);
        });

    return points;
}

// 生成花形坐标点
vector<pair<float, float>> generateStar(int numPoints) {
    vector<pair<float, float>> points;
    points.reserve(numPoints);

    float R1 = 0.5f;  // 基础半径
    float R2 = 0.3f;  // 波动幅度
    int n = 5;        // 星形角数

    for (int i = 0; i < numPoints; i++) {
        float theta = 2.0f * PI * i / numPoints;

        // 花形的极坐标方程
        float r = R1 + R2 * cos(n * theta);

        // 转换为直角坐标并归一化到[-1, 1]
        float x = r * cos(theta);
        float y = r * sin(theta);

        // 调整到[-1, 1]范围
        float maxRadius = R1 + R2;
        x /= maxRadius;
        y /= maxRadius;

        // 美观
        x *= 0.85f;
        y *= 0.85f;

        points.emplace_back(x, y);
    }
    // 排序
    auto centroid = computeCentroid(points);
    sort(points.begin(), points.end(),
        [centroid](const pair<float, float>& a, const pair<float, float>& b) {
            return computeAngle(a, centroid) < computeAngle(b, centroid);
        });

    return points;
}

// 多边形A和B（顶点数相同）
vector<pair<float, float>> polygonA;
vector<pair<float, float>> polygonB;

// 顶点插值绘制函数
void drawTween(const vector<pair<float, float>>& A,
    const vector<pair<float, float>>& B,
    float tt) {
    glBegin(GL_LINE_LOOP);
    glColor3f(0.0f, 0.8f, 1.0f); // 插值图形颜色
    for (size_t i = 0; i < A.size(); ++i) {
        // 线性插值计算中间顶点
        float x = (1.0f - tt) * A[i].first + tt * B[i].first;
        float y = (1.0f - tt) * A[i].second + tt * B[i].second;
        glVertex2f(x, y);
    }
    glEnd();
}

// 显示回调函数（双缓冲设置）
void display() {
    // 清空颜色缓冲
    glClear(GL_COLOR_BUFFER_BIT);

    // 绘制插值动画帧
    drawTween(polygonA, polygonB, t);

    // 交换前后缓冲（双缓冲技术，避免闪烁）
    glutSwapBuffers();
}

// 定时器回调函数（控制动画进度）
void timer(int value) {
    if (!paused) {
        // 更新插值进度t
        if (increasing) {
            t += 0.01f;
            if (t >= 1.0f) {
                t = 1.0f;
                increasing = false; // 反向：从B变回A
            }
        }
        else {
            t -= 0.01f;
            if (t <= 0.0f) {
                t = 0.0f;
                increasing = true; // 正向：从A变到B
            }
        }
        // 请求重新绘制
        glutPostRedisplay();
    }
    // 16ms后再次触发定时器（约60FPS）
    glutTimerFunc(16, timer, 0);
}

// 键盘回调函数（按下任意键暂停/继续）
void keyboard(unsigned char key, int x, int y) {
    paused = !paused;
}

// 初始化OpenGL设置
void init() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // 背景色
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // 正交投影
    gluOrtho2D(-1.0f, 1.0f, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
}

// 窗口大小改变时的回调
void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // 保持宽高比，避免图形拉伸
    float aspect = (float)width / (float)height;
    if (aspect >= 1.0f) {
        gluOrtho2D(-aspect, aspect, -1.0f, 1.0f);
    }
    else {
        gluOrtho2D(-1.0f, 1.0f, -1.0f / aspect, 1.0f / aspect);
    }
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    // 初始化GLUT
    glutInit(&argc, argv);
    // 设置双缓冲、RGB颜色模式
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    // 创建窗口
    glutInitWindowSize(500, 500);
    glutCreateWindow("Polygon Tween Animation");

    // 注册回调函数
    glutDisplayFunc(display);
    glutTimerFunc(16, timer, 0);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);

    // 生成图形
    int numPoints = 500;
    polygonA = generateHeart(numPoints);
    polygonB = generateStar(numPoints);

    init();
    glutMainLoop();
    return 0;
}