#include <cmath>
#include <vector>
#include <cstdlib>
#include "glfw/include/GLFW/glfw3.h"

enum class View {
	PolygonOutline, //контур многоугольника
	PolygonFill, //заливка алгоритмом XOR с флагом
    SmoothPolygonOutline  //сглаживание
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

std::vector<Point> points;

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
}

inline int GetPointPosition(const Point& point) {
	return (int)((point.x + (point.y * window_width)) * 4);
}

bool IsPixelColor(const Point& point, const unsigned char color[3]) {
	int position = GetPointPosition(point);
	return buffer[position + 0] == color[0] && buffer[position + 1] == color[1] && buffer[position + 2] == color[2];
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

void PutPixel(int x, int y) {
	PutPixel(x, y, drawColor);
}

void PutPixel(int x, int y, float intensity) {
	auto colorComponent = (unsigned char)intensity;
	unsigned char color[] = { colorComponent, colorComponent, colorComponent };
	PutPixel(x, y, color);
}

void AddLine(int xa, int ya, int xb, int yb) {
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
		PutPixel(xa, ya);
		for (int i = 0; i <= dx; i++, x += sx) {
			if (d > 0) {
				d += d2;
				y += sy;
			} else {
				d += d1;
			}

			PutPixel(x, y);
		}
	} else {
		const int d1 = dx << 1;
		const int d2 = (dx - dy) << 1;
		int d = (dx << 1) - dy;
		int x = xa;
		int y = ya;
		PutPixel(xa, ya);
		for (int i = 0; i <= dy; i++, y += sy) {
			if (d > 0) {
				d += d2;
				x += sx;
			} else {
				d += d1;
			}

			PutPixel(x, y);
		}
	}
}

void AddSmoothLine(int xa, int ya, int xb, int yb) {
	const int dx = std::abs(xb - xa);
	const int dy = std::abs(yb - ya);
	const int sx = xb >= xa ? 1 : -1;
	const int sy = yb >= ya ? 1 : -1;
	if (dy <= dx) {
		const float m = 255.0f * float(dy) / (float)dx;
		const float w = 255.0f - m;
		float e = m / 2.0f;
		int x = xa;
		int y = ya;
		for (int i = 0; i < dx; i++, x += sx) {
			if (e < w) {
				e += m;
			} else {
				y += sy;
				e -= w;
			}

			if (sx * sy == -1) {
				PutPixel(x + sx, y + sy, e);
			} else {
				PutPixel(x, y, 255.0f - e);
			}
		}
	} else {
		const float m = 255 * float(dx) / (float)dy;
		const float w = 255.0f - m;
		float e = m / 2.0f;
		int x = xa;
		int y = ya;
		for (int i = 0; i < dy; i++, y += sy) {
			if (e < w) {
				e += m;
			} else {
				x += sx;
				e -= w;
			}

			if (sx * sy == 1) {
				PutPixel(x + sx, y + sy, e);
			} else {
				PutPixel(x, y, 255 - e);
			}
		}
	}
}

void DrawPolygonOutline(bool isSmooth = false) {
	if (points.size() <= 1) {
		return;
	}

	void (*addLine)(int, int, int, int) = isSmooth ? AddSmoothLine : AddLine;
	size_t size = points.size();
	for (size_t i = 0; i < size - 1; i++) {
		addLine(points[i].x, points[i].y, points[i + 1].x, points[i + 1].y);
	}

	if (size >= 3) {
		addLine(points[size - 1].x, points[size - 1].y, points[0].x, points[0].y);
	}
}

void Rasterize() {
    //алгоритм XOR с флагом
    for (int y = 0; y < window_height; y++) {
        bool flag = false;
        for (int x = 0; x < window_width; x++) {
            Point point = {x, y};
            if (IsPixelColor(point, drawColor)) {
                while (true) {
                    x++;
                    point = {x, y};
                    if (!IsPixelColor(point, drawColor)) {
                        break;
                    }
                }
                flag = !flag;
            }
            if (flag) {
                bool flag1 = false;
                int x_tmp = x;
                while (x_tmp < window_width) {
                    point = {x_tmp, y};
                    if (IsPixelColor(point, drawColor)) {
                        flag1 = true;
                    }
                    x_tmp++;
                }
                if (flag1) {
                    PutPixel(x, y);
                }
            }
        }
    }
}

void UpdateBuffer() {
	ClearBuffer();
    DrawPolygonOutline();
    if (view == View::PolygonFill) {
        Rasterize();
    }
    else if (view == View::SmoothPolygonOutline) {
        DrawPolygonOutline(true);
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

	if (button == GLFW_MOUSE_BUTTON_1) {
        AddPoint(x, y);
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
        case GLFW_KEY_F:
            SetView(View::PolygonFill);
            return;
        case GLFW_KEY_S:
            SetView(View::SmoothPolygonOutline);
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
