#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <GL/glut.h>
using namespace std;

const double PI = 3.14159265358979323846;
float radius = 4.0f;
int slices = 32, stacks = 16;

// 顶点结构体
struct Point3f {
    float x, y, z;
    Point3f(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
};

// 法向量结构体
typedef Point3f Normal3f;

// 面的顶点元素结构体
struct FaceVertex {
    int vertIndex;   // 顶点索引
    int normIndex;   // 法向量索引
    FaceVertex(int v = 0, int n = 0) : vertIndex(v), normIndex(n) {}
};

// 面结构体
struct Face {
    int nVerts;                  // 面的顶点数量
    vector<FaceVertex> verts;    // 面的顶点集合
    Face(int n = 0) : nVerts(n) {
        verts.resize(n);
    }
};

// Mesh 核心类
class Mesh {
private:
    vector<Point3f> pts;     // 顶点集合
    vector<Normal3f> norms;  // 法向量集合
    vector<Face> faces;      // 面集合
    int numVerts;            // 顶点总数
    int numNorms;            // 法向量总数
    int numFaces;            // 面总数

public:
    Mesh() : numVerts(0), numNorms(0), numFaces(0) {}
    void printInfo() {
        cout << "numVerts: " << numVerts << endl;
        cout << "numNorms: " << numNorms << endl;
        cout << "numFaces: " << numFaces << endl;
        if (numVerts == pts.size()) {
            cout << "pts:" << endl;
            for (int i = 0; i < numVerts; i++) {
                cout << i << ":" << pts[i].x << " " << pts[i].y << " " << pts[i].z << endl;
            }
        }
        if (numNorms == norms.size()) {
            cout << "norms:" << endl;
            for (int i = 0; i < numNorms; i++) {
                cout << i << ":" << norms[i].x << " " << norms[i].y << " " << norms[i].z << endl;
            }
        }
        if (numFaces == faces.size()) {
            cout << "faces:" << endl;
            for (int i = 0; i < numFaces; i++) {
                cout << i << ":" << faces[i].nVerts << " ";
                if (faces[i].nVerts == faces[i].verts.size()) {
                    for (int j = 0; j < faces[i].nVerts; j++) {
                        cout << faces[i].verts[j].vertIndex << " ";
                    }
                    for (int j = 0; j < faces[i].nVerts; j++) {
                        cout << faces[i].verts[j].normIndex << " ";
                    }
                }
                cout << endl;
            }
        }
    }

    // 读取Mesh文件（.txt格式）
    int readMesh(const char* filename) {
        ifstream fin(filename);
        if (!fin.is_open()) {
            cerr << "错误：无法打开文件 " << filename << endl;
            return 1; // 文件打开失败
        }

        // 读取顶点数、法向量数、面数
        fin >> numVerts >> numNorms >> numFaces;
        if (fin.fail()) {
            cerr << "错误：文件格式错误（第一行应为三个整数）" << endl;
            fin.close();
            return 2;
        }

        // 读取顶点数据
        pts.resize(numVerts);
        for (int i = 0; i < numVerts; i++) {
            fin >> pts[i].x >> pts[i].y >> pts[i].z;
            if (fin.fail()) {
                cerr << "错误：顶点数据格式错误（第" << i + 1 << "个顶点）" << endl;
                fin.close();
                return 3;
            }
        }

        // 读取法向量数据
        norms.resize(numNorms);
        for (int i = 0; i < numNorms; i++) {
            fin >> norms[i].x >> norms[i].y >> norms[i].z;
            if (fin.fail()) {
                cerr << "错误：法向量数据格式错误（第" << i + 1 << "个法向量）" << endl;
                fin.close();
                return 4;
            }
        }

        // 读取面数据
        faces.resize(numFaces);
        for (int i = 0; i < numFaces; i++) {
            fin >> faces[i].nVerts; // 读取面的顶点数
            if (fin.fail()) {
                cerr << "错误：面数据格式错误（第" << i + 1 << "个面的顶点数）" << endl;
                fin.close();
                return 5;
            }

            // 读取顶点索引
            faces[i].verts.resize(faces[i].nVerts);
            for (int j = 0; j < faces[i].nVerts; j++) {
                fin >> faces[i].verts[j].vertIndex;
                if (fin.fail()) {
                    cerr << "错误：面顶点索引错误（第" << i + 1 << "个面，第" << j + 1 << "个顶点）" << endl;
                    fin.close();
                    return 6;
                }
            }

            // 读取法向量索引
            for (int j = 0; j < faces[i].nVerts; j++) {
                fin >> faces[i].verts[j].normIndex;
                if (fin.fail()) {
                    cerr << "错误：面法向量索引错误（第" << i + 1 << "个面，第" << j + 1 << "个顶点）" << endl;
                    fin.close();
                    return 7;
                }
            }
        }

        fin.close();
        cout << "文件读取成功！顶点数：" << numVerts << "，法向量数：" << numNorms << "，面数：" << numFaces << endl;
        //printInfo();
        return 0; // 成功
    }

    // 写入Mesh文件（.txt格式）
    int writeMesh(const char* filename) {
        ofstream fout(filename);
        if (!fout.is_open()) {
            cerr << "错误：无法创建文件 " << filename << endl;
            return 1;
        }

        // 写入顶点数、法向量数、面数
        fout << numVerts << " " << numNorms << " " << numFaces << endl;

        // 写入顶点数据（保留3位小数）
        for (int i = 0; i < numVerts; i++) {
            fout.precision(3);
            fout << pts[i].x << " " << pts[i].y << " " << pts[i].z << endl;
        }

        // 写入法向量数据（单位向量，保留3位小数）
        for (int i = 0; i < numNorms; i++) {
            fout.precision(3);
            fout << norms[i].x << " " << norms[i].y << " " << norms[i].z << endl;
        }

        // 写入面数据
        for (int i = 0; i < numFaces; i++) {
            fout << faces[i].nVerts << " "; // 面的顶点数

            // 写入顶点索引
            for (int j = 0; j < faces[i].nVerts; j++) {
                fout << faces[i].verts[j].vertIndex << " ";
            }

            // 写入法向量索引
            for (int j = 0; j < faces[i].nVerts; j++) {
                fout << faces[i].verts[j].normIndex << " ";
            }
            fout << endl;
        }

        fout.close();
        cout << "文件写入成功！保存为：" << filename << endl;
        return 0;
    }

    // 生成球体（半径，经度分段数，纬度分段数）
    void generateSphere(float radius, int slices, int stacks) {
        // 清空原有数据
        pts.clear();
        norms.clear();
        faces.clear();
        numVerts = 0;
        numNorms = 0;
        numFaces = 0;

        // 1. 生成顶点和法向量（球面参数方程）
        // 纬度角：从底部（-π/2）到顶部（π/2），共stacks+1个点
        // 经度角：从0到2π，共slices个点
        for (int i = 0; i <= stacks; i++) {
            float lat = -PI / 2 + (float)i / stacks * PI; // 纬度角（-90°到90°）
            float sinLat = sin(lat);
            float cosLat = cos(lat);

            for (int j = 0; j < slices; j++) {
                float lon = (float)j / slices * 2 * PI; // 经度角（0°到360°）
                float sinLon = sin(lon);
                float cosLon = cos(lon);

                // 顶点坐标（球面参数方程）
                float x = radius * cosLat * cosLon;
                float y = radius * sinLat;
                float z = radius * cosLat * sinLon;
                pts.push_back(Point3f(x, y, z));

                // 法向量（球面法向量=顶点坐标归一化）
                float len = sqrt(x * x + y * y + z * z);
                norms.push_back(Normal3f(x / len, y / len, z / len));
            }
        }

        // 更新顶点数和法向量数（一一对应）
        numVerts = pts.size();
        numNorms = norms.size();

        // 2. 生成面（每个面是四边形，拆分为两个三角形）
        for (int i = 0; i < stacks; i++) {
            for (int j = 0; j < slices; j++) {
                // 当前四边形的四个顶点索引
                int v0 = i * slices + j;
                int v1 = (i + 1) * slices + j;
                int v2 = (i + 1) * slices + (j + 1) % slices;
                int v3 = i * slices + (j + 1) % slices;

                // 三角形1：v0 -> v1 -> v2
                Face face1(3);
                face1.verts[0] = FaceVertex(v0, v0); // 顶点索引=法向量索引（一一对应）
                face1.verts[1] = FaceVertex(v1, v1);
                face1.verts[2] = FaceVertex(v2, v2);
                faces.push_back(face1);

                // 三角形2：v0 -> v2 -> v3
                Face face2(3);
                face2.verts[0] = FaceVertex(v0, v0);
                face2.verts[1] = FaceVertex(v2, v2);
                face2.verts[2] = FaceVertex(v3, v3);
                faces.push_back(face2);
            }
        }

        // 更新面数
        numFaces = faces.size();
        cout << "球体生成成功！半径：" << radius << "，经度分段：" << slices << "，纬度分段：" << stacks
            << "，顶点数：" << numVerts << "，面数：" << numFaces << endl;
    }

    // 绘制Mesh（线框模式）
    void draw(bool isHouse) {
        glPushMatrix();
        if (isHouse) {
            glScalef(3.0f, 3.0f, 3.0f);
        }
        else {
            // 计算旋转角度：绕(1,0,-1)轴旋转45度
            float angle = 45.0f * PI / 180.0f;
            float axisX = 1.0f, axisY = 0.0f, axisZ = -1.0f;
            // 归一化旋转轴
            float len = sqrt(axisX * axisX + axisY * axisY + axisZ * axisZ);
            axisX /= len;
            axisY /= len;
            axisZ /= len;
            glRotatef(45.0f, axisX, axisY, axisZ); // 旋转45度
        }

        // 设置材质属性（让模型对光照有响应，避免全黑）
        float matAmbient[] = { 0.4f, 0.4f, 0.4f, 1.0f };  // 材质环境光
        float matDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };  // 材质漫反射
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, matAmbient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matDiffuse);

        for (int f = 0; f < numFaces; f++) {
            glBegin(GL_LINE_LOOP); // 线环模式，自动闭合
            for (int v = 0; v < faces[f].nVerts; v++) {
                int normIdx = faces[f].verts[v].normIndex;
                int vertIdx = faces[f].verts[v].vertIndex;

                // 设置法向量（光照开启时生效）
                glNormal3f(norms[normIdx].x, norms[normIdx].y, norms[normIdx].z);
                // 设置顶点
                glVertex3f(pts[vertIdx].x, pts[vertIdx].y, pts[vertIdx].z);
            }
            glEnd();
        }

        // 恢复矩阵
        glPopMatrix();
    }
};

// 全局变量
Mesh houseMesh, sphereMesh;
bool drawHouse = true; // 标记当前绘制的模型（true：仓房，false：球体）
bool openLight = false; // 光照开关
int sphereChange = 0; // 设置将要调整的球参数

// 更新光照状态
void updateLightState() {
    if (openLight) {
        // 启用光照
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);

        // 设置光源参数
        float lightPos[] = { 5.0f, 5.0f, 8.0f, 1.0f };          // 点光源位置
        float lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };      // 漫反射光
        float lightAmbient[] = { 0.4f, 0.4f, 0.4f, 1.0f };      // 环境光

        glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
        glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);

        // 全局环境光
        float globalAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
    }
    else {
        // 禁用光照
        glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0);
    }
}

// 增大球体参数
void upSphereSize() {
    if (drawHouse) {
        return;
    }
    if (sphereChange == 0) {
        cout << "半径 + 0.5" << endl;
        radius += 0.5;
    }
    else if (sphereChange == 1){
        cout << "经度分段数 + 1" << endl;
        slices += 1;
    }
    else {
        cout << "纬度分段数 + 1" << endl;
        stacks += 1;
    }
    sphereMesh.generateSphere(radius, slices, stacks);
}

// 减小球体参数
void downSphereSize() {
    if (drawHouse) {
        return;
    }
    if (sphereChange == 0) {
        if (radius == 0.5) {
            cout << "半径已达最小" << endl;
            return;
        }
        cout << "半径 - 0.5" <<endl;
        radius -= 0.5;
    }
    else if (sphereChange == 1) {
        if (slices == 3) {
            cout << "经度分段数已达最小" << endl;
            return;
        }
        cout << "经度分段数 - 1" << endl;
        slices -= 1;
    }
    else {
        if (stacks == 2) {
            cout << "纬度分段数已达最小" << endl;
            return;
        }
        cout << "纬度分段数 - 1" << endl;
        stacks -= 1;
    }
    sphereMesh.generateSphere(radius, slices, stacks);
}

// 初始化函数
void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // 背景色
    glEnable(GL_DEPTH_TEST); // 启用深度测试
    glEnable(GL_LINE_SMOOTH); // 启用线条抗锯齿
    glEnable(GL_BLEND); // 启用混合（抗锯齿需要）
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // 设置混合模式
    //glLineWidth(0.5f); // 设置线条宽度

    updateLightState();

    // 读取仓房模型文件
    if (houseMesh.readMesh("house.txt") != 0) {
        cerr << "仓房模型读取失败" << endl;
    }

    // 生成球体
    sphereMesh.generateSphere(radius, slices, stacks);
    // 将球体写入文件
    sphereMesh.writeMesh("sphere.txt");
}

// 相机设置函数
void setupCamera() {
    glMatrixMode(GL_PROJECTION); // 切换到投影矩阵
    glLoadIdentity();            // 重置为单位矩阵

    // 获取当前窗口的宽高
    int width = glutGet(GLUT_WINDOW_WIDTH);
    int height = glutGet(GLUT_WINDOW_HEIGHT);

    // 计算宽高比
    float aspect = (float)width / height;

    // 适配宽高比的正交投影
    float size = 8.0f;
    if (aspect > 1.0f) {
        // 窗口宽 > 高，扩展X轴范围
        glOrtho(-size * aspect, size * aspect, -size, size, -50.0f, 50.0f);
    }
    else {
        // 窗口高 > 宽，扩展Y轴范围
        glOrtho(-size, size, -size / aspect, size / aspect, -50.0f, 50.0f);
    }

    glMatrixMode(GL_MODELVIEW);  // 切换到模型视图矩阵
    glLoadIdentity();            // 重置为单位矩阵
    // 相机位置
    if (drawHouse) {
        gluLookAt(6.0f, 4.0f, 8.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    }
    else {
        gluLookAt(0.0f, 0.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    }
}

// 绘制函数
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清空颜色和深度缓冲区
    setupCamera(); // 设置相机

    updateLightState();

    // 绘制当前选中的模型
    if (drawHouse && houseMesh.readMesh("house.txt") == 0) {
        houseMesh.draw(drawHouse);
    }
    else {
        sphereMesh.draw(drawHouse);
    }

    glutSwapBuffers(); // 双缓冲交换
}

// 键盘交互函数
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'h':
    case 'H':
        drawHouse = true;
        cout << "当前绘制：仓房模型" << endl;
        break;
    case 's':
    case 'S':
        drawHouse = false;
        cout << "当前绘制：球体模型" << endl;
        break;
    case 'l':
    case 'L':
        openLight = !openLight;
        cout << "光照：" << (openLight ? "打开" : "关闭") << endl;
        break;
    case 'c':
    case 'C':
        if (drawHouse) {
            return;
        }
        sphereChange = (sphereChange + 1) % 3;
        cout << "设置球体参数-";
        if (sphereChange == 0) {
            cout << "半径" << endl;
        }
        else if (sphereChange == 1) {
            cout << "经度分段数" << endl;
        }
        else {
            cout << "纬度分段数" << endl;
        }
        break;
    case 27: // ESC键
        exit(0);
        break;
    }
    glutPostRedisplay(); // 重新绘制
}

void specialKey(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP:
        upSphereSize();
        break;
    case GLUT_KEY_DOWN:
        downSphereSize();
        break;
    }
    glutPostRedisplay();  // 重新绘制场景
}

// 窗口大小调整函数
void reshape(int width, int height) {
    glViewport(0, 0, width, height); // 设置视口
    setupCamera();
}

// 主函数
int main(int argc, char** argv) {
    // 初始化GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // 双缓冲+RGB+深度缓冲
    glutInitWindowSize(800, 600); // 窗口大小
    glutCreateWindow("Mesh Viewer - 仓房 & 球体"); // 窗口标题

    // 注册回调函数
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKey);

    // 初始化
    init();

    // 进入GLUT主循环
    glutMainLoop();
    return 0;
}