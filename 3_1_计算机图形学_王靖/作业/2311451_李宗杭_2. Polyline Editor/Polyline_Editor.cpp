#include <stdlib.h>
#include <vector>
#include <cmath>
#include <iostream>
#include <GL/glut.h>

using namespace std;

struct Point {
	float x, y;

	bool operator == (const Point& other) const {
		return x == other.x && y == other.y;
	}
};

vector<vector<Point>> polylines;
vector<Point>* currentPolyline = nullptr;
Point* movingPoint = nullptr;
Point* selectedPoint = nullptr;
char currentAction = '\0';

bool isNearPoint(const Point& p1, const Point& p2) {
	const float threshold = 0.02f;
	return abs(p1.x - p2.x) < threshold && abs(p1.y - p2.y) < threshold;
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(0.0, 0.0, 0.0);
	glLineWidth(3.0f);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	glBegin(GL_LINES);
	for (const auto& polyline : polylines) {
		for (size_t i = 0; i < polyline.size() - 1; ++i) {
			glVertex2f(polyline[i].x, polyline[i].y);
			glVertex2f(polyline[i + 1].x, polyline[i + 1].y);
		}
	}
	glEnd();
	glutSwapBuffers();
}

void mouse(int button, int state, int x, int y) {
	float fx = (float)x / (glutGet(GLUT_WINDOW_WIDTH) / 2) - 1.0f;
	float fy = 1.0f - (float)y / (glutGet(GLUT_WINDOW_HEIGHT) / 2);

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (currentAction == 'b' && currentPolyline != nullptr) {
			currentPolyline->push_back({ fx, fy });
		}
		else if (currentAction == 'm') {
			for (auto& polyline : polylines) {
				for (auto& point : polyline) {
					if (isNearPoint({ fx, fy }, point)) {
						movingPoint = &point;
						selectedPoint = &point;
						return;
					}
				}
			}
		}
		else if (currentAction == 'd') {
			for (auto& polyline : polylines) {
				for (auto it = polyline.begin(); it != polyline.end(); ++it) {
					if (isNearPoint({ fx, fy }, *it)) {
						if (it != polyline.begin() && it != polyline.end() - 1) {
							polyline.erase(it);
						}
						return;
					}
				}
			}
		}
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		movingPoint = nullptr;
	}
	glutPostRedisplay();
}

void motion(int x, int y) {
	if (movingPoint != nullptr) {
		movingPoint->x = (float)x / (glutGet(GLUT_WINDOW_WIDTH) / 2) - 1.0f;
		movingPoint->y = 1.0f - (float)y / (glutGet(GLUT_WINDOW_HEIGHT) / 2);
		glutPostRedisplay();
	}
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'b':
		polylines.emplace_back();
		currentPolyline = &polylines.back();
		currentAction = 'b';
		break;
	case 'd':
		currentAction = 'd';
		break;
	case 'm':
		currentAction = 'm';
		break;
	case 'r':
		polylines.clear();
		currentPolyline = nullptr;
		movingPoint = nullptr;
		selectedPoint = nullptr;
		currentAction = '\0';
		glutPostRedisplay();
		break;
	case 'q':
		exit(0);
		break;
	}
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Polyline Editor");

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-1.0, 1.0, -1.0, 1.0);

	glutDisplayFunc(display);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutKeyboardFunc(keyboard);

	glutMainLoop();
	return 0;
}