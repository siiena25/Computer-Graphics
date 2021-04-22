#include <cmath>
#include <vector>
#include <cstdlib>
#include <iostream>
#include "glfw/include/GLFW/glfw3.h"

enum class View {
	PolygonOutline, //контур многоугольника
	ClippedPolygonOutline // отсеченный многоуольник
};

struct Point {
	int x;
	int y;
};

size_t window_width = 1280;
size_t window_height = 720;

unsigned char* buffer = nullptr;

const unsigned char defColor[] = { 255, 255, 255 };
const unsigned char drawColor[] = { 0, 0, 0 };
const unsigned char drawClipColor[] = { 255, 0, 0 };
const unsigned char drawClippedColor[] = { 0, 0, 0 };

std::vector<Point> points;
std::vector<Point> clippedPoints;

const int MAX_POINTS = 20;

int x_intersect(int x1, int y1, int x2, int y2,
                int x3, int y3, int x4, int y4) {
    int num = (x1*y2 - y1*x2) * (x3-x4) - (x1-x2) * (x3*y4 - y3*x4);
    int den = (x1-x2) * (y3-y4) - (y1-y2) * (x3-x4);
    return num/den;
}

int y_intersect(int x1, int y1, int x2, int y2,
                int x3, int y3, int x4, int y4) {
    int num = (x1*y2 - y1*x2) * (y3-y4) - (y1-y2) * (x3*y4 - y3*x4);
    int den = (x1-x2) * (y3-y4) - (y1-y2) * (x3-x4);
    return num/den;
}

void clip(int poly_points[][2], int &poly_size,
          int x1, int y1, int x2, int y2) {
    int new_points[MAX_POINTS][2], new_poly_size = 0;

    for (int i = 0; i < poly_size; i++) {
        int k = (i+1) % poly_size;
        int ix = poly_points[i][0], iy = poly_points[i][1];
        int kx = poly_points[k][0], ky = poly_points[k][1];
        int i_pos = (x2-x1) * (iy-y1) - (y2-y1) * (ix-x1);
        int k_pos = (x2-x1) * (ky-y1) - (y2-y1) * (kx-x1);

        if (i_pos < 0  && k_pos < 0) {
            new_points[new_poly_size][0] = kx;
            new_points[new_poly_size][1] = ky;
            new_poly_size++;
        } else if (i_pos >= 0  && k_pos < 0) {
            new_points[new_poly_size][0] = x_intersect(x1, y1, x2, y2, ix, iy, kx, ky);
            new_points[new_poly_size][1] = y_intersect(x1, y1, x2, y2, ix, iy, kx, ky);
            new_poly_size++;

            new_points[new_poly_size][0] = kx;
            new_points[new_poly_size][1] = ky;
            new_poly_size++;
        } else if (i_pos < 0  && k_pos >= 0) {
            new_points[new_poly_size][0] = x_intersect(x1, y1, x2, y2, ix, iy, kx, ky);
            new_points[new_poly_size][1] = y_intersect(x1, y1, x2, y2, ix, iy, kx, ky);
            new_poly_size++;
        } else {
            //No points are added
        }
    }

    poly_size = new_poly_size;
    for (int i = 0; i < poly_size; i++) {
        poly_points[i][0] = new_points[i][0];
        poly_points[i][1] = new_points[i][1];
    }
}

View view = View::PolygonOutline;

void SetProjection() {
	const float matrix[4 * 4] = {
		2.0f / (float)window_width, 0, 0, 0,
		0, 2.0f / (float)window_height, 0, 0,
		0, 0, 0, 0,
		-1, -1, 0, 1
	};
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(&matrix[0]);
	glMatrixMode(GL_MODELVIEW);
}

inline size_t GetBufferSize() {
	return window_width * window_height * 4;
}

void ClearBuffer() {
	const size_t size = GetBufferSize();
	for (size_t i = 0; i < size; i += 4) {
		buffer[i + 0] = defColor[0];
		buffer[i + 1] = defColor[1];
		buffer[i + 2] = defColor[2];
	}
}

void InitializeBuffer() {
	size_t size = GetBufferSize();
	buffer = new unsigned char[size];
	ClearBuffer();
}

void ResetBuffer() {
	delete[] buffer;
	InitializeBuffer();
}

void Reset() {
	ResetBuffer();
	points.clear();
	clippedPoints.clear();
}

inline int GetPointPosition(const Point& point) {
	return (int)((point.x + (point.y * window_width)) * 4);
}

void PutPixel(int x, int y, const unsigned char color[3]) {
	if (x < 0 || x >= int(window_width) || y < 0 || y >= int(window_height)) {
		return;
	}

	Point point = {x, y};
	int position = GetPointPosition(point);
	buffer[position + 0] = color[0];
	buffer[position + 1] = color[1];
	buffer[position + 2] = color[2];
}

void AddLine(int xa, int ya, int xb, int yb, const unsigned char color[3]) {
	const int dx = std::abs(xb - xa);
	const int dy = std::abs(yb - ya);
	const int sx = xb >= xa ? 1 : -1;
	const int sy = yb >= ya ? 1 : -1;
	if (dy <= dx) {
		const int d1 = dy << 1;
		const int d2 = (dy - dx) << 1;
		int d = (dy << 1) - dx;
		int x = xa;
		int y = ya;
		PutPixel(xa, ya, color);
		for (int i = 0; i <= dx; i++, x += sx) {
			if (d > 0) {
				d += d2;
				y += sy;
			} else {
				d += d1;
			}

			PutPixel(x, y, color);
		}
	} else {
		const int d1 = dx << 1;
		const int d2 = (dx - dy) << 1;
		int d = (dx << 1) - dy;
		int x = xa;
		int y = ya;
		PutPixel(xa, ya, color);
		for (int i = 0; i <= dy; i++, y += sy) {
			if (d > 0) {
				d += d2;
				x += sx;
			} else {
				d += d1;
			}

			PutPixel(x, y, color);
		}
	}
}

void DrawPolygonOutline(std::vector<Point> &curPoints, const unsigned char color[3]) {
    if (curPoints.size() <= 1) {
        return;
    }

    void (*addLine)(int, int, int, int, const unsigned char[3]) = AddLine;
    size_t size = curPoints.size();
    for (size_t i = 0; i < size - 1; i++) {
        addLine(curPoints[i].x, curPoints[i].y, curPoints[i + 1].x, curPoints[i + 1].y, color);
    }

    if (size >= 3) {
        addLine(curPoints[size - 1].x, curPoints[size - 1].y, curPoints[0].x, curPoints[0].y, color);
    }
}

void suthHodgClip(int poly_points[][2], int poly_size, int clipper_points[][2], int clipper_size) {
    for (int i = 0; i < clipper_size; i++) {
        int k = (i + 1) % clipper_size;
        clip(poly_points, poly_size, clipper_points[i][0],
             clipper_points[i][1], clipper_points[k][0],
             clipper_points[k][1]);
    }

    std::vector<Point> curPoints;
    for (int i = 0; i < poly_size; i++) {
        Point point = { poly_points[i][0], poly_points[i][1] };
        curPoints.push_back(point);
    }

    DrawPolygonOutline(curPoints, drawClippedColor);
}

void DrawClippedPolygonOutline() {
    int poly_size = points.size();
    int poly_points[20][2];
    for (int i = 0; i < poly_size; i++) {
        poly_points[i][0] = points[i].x;
        poly_points[i][1] = points[i].y;
    }

    int clipped_size = clippedPoints.size();
    int clipped_points[20][2];
    for (int i = 0; i < clipped_size; i++) {
        clipped_points[i][0] = clippedPoints[i].x;
        clipped_points[i][1] = clippedPoints[i].y;
    }

    suthHodgClip(poly_points, poly_size, clipped_points, clipped_size);
}

void UpdateBuffer() {
    ClearBuffer();
    switch (view) {
        case View::PolygonOutline:
            DrawPolygonOutline(points, drawColor);
            DrawPolygonOutline(clippedPoints, drawClipColor);
            break;
        case View::ClippedPolygonOutline:
            DrawClippedPolygonOutline();
            break;
    }
}

void SetView(View curView) {
	if (view == curView) return;
	view = curView;
	UpdateBuffer();
}

void AddPoint(int x, int y) {
	points.push_back({ x, y });
	UpdateBuffer();
}

void AddClippedPoint(int x, int y) {
    clippedPoints.push_back({ x, y });
    UpdateBuffer();
}

void WindowSizeCallback(GLFWwindow* window, int width, int height) {
    window_width = width;
    window_height = height;
	glViewport(0, 0, width, height);
	SetProjection();
	Reset();
}

void MouseClickCallback(GLFWwindow* window, int button, int action, int mods) {
	if (action != GLFW_PRESS) return;

	double xPos, yPos;
	glfwGetCursorPos(window, &xPos, &yPos);
	int x = int(std::floor(xPos));
	int y = (int)window_height - int(std::floor(yPos));

    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            AddPoint(x, y);
            return;
        case GLFW_MOUSE_BUTTON_RIGHT:
            AddClippedPoint(x, y);
            return;
        default:
            return;
    }
}

void KeyCallback(GLFWwindow* window, int button, int scancode, int action, int mods) {
	if (action != GLFW_PRESS) {
		return;
	}

	switch (button) {
        case GLFW_KEY_O:
            SetView(View::PolygonOutline);
            return;
        case GLFW_KEY_C:
            SetView(View::ClippedPolygonOutline);
            return;
        case GLFW_KEY_R:
            Reset();
            return;
        case GLFW_KEY_ESCAPE:
            exit(0);
        default:
            return;
	}
}

int main() {
	if (!glfwInit()) return EXIT_FAILURE;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = glfwCreateWindow((int)window_width, (int)window_height, "Lab 4", nullptr, nullptr);

	glfwMakeContextCurrent(window);
	glfwSetWindowSizeCallback(window, WindowSizeCallback);
	glfwSetMouseButtonCallback(window, MouseClickCallback);
	glfwSetKeyCallback(window, KeyCallback);

	SetProjection();
	InitializeBuffer();

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		glDrawPixels((int)window_width, (int)window_height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return EXIT_SUCCESS;
}
