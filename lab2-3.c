#include <stdlib.h>
#include <GL/freeglut.h>
#include <math.h>

int clientWidth = 1920, clientHeight = 1080;
int windowPositionX = 500, windowPositionY = 500;
float cubeX = -1, cubeY = -1, cubeZ = -1;
const float fx = -3, fy = -3, fz = -3;
float angle = 0.0f, alpha = 0.1, coordX = 0.0f;
double k = 1.0;
int n = 45;

#define ESC_KEY 27

float projectionMatrix[4 * 4] = {
        1, 0, 0, 1 / fx,
        0, 1, 0, 1 / fy,
        0, 0, 1, 1 / fz,
        0, 0, 0, 1
};

void makeTriangle(const float coords[9]) {
    glVertex3fv(&coords[0]);
    glVertex3fv(&coords[3]);
    glVertex3fv(&coords[6]);
}

void makeQuad(const float points[12]) {
    makeTriangle(&points[0]);
    makeTriangle(&points[3]);
}

void makeCircle(float r, float y, size_t n) { //circle по осям Ох, Oz
    if (n < 3) {
        return;
    }

    float points[9] = {
            0, y, 0, //центр окружности
            r, y, 0 //точка на окружности при t = 0
    };

    for (size_t i = 0, isOdd = 1; i < n; i++, isOdd = !isOdd) {
        size_t offset = 3 * (isOdd + 1);
        float t = 2 * M_PI * ((i + 1) % n) / n;
        points[offset + 0] = r * cos(t); //x; парам.уравнение окружности
        points[offset + 1] = y;
        points[offset + 2] = r * sin(t); //z
        makeTriangle(points);
    }
}

void makeCylinder(float r, float h, size_t n) {
    if (n < 3) {
        return;
    }

    float points[12] = {
            r, h / 2, 0,
            r, - h / 2, 0
    };

    makeCircle(r, h / 2, n);
    makeCircle(r, - h / 2, n);

    for (size_t i = 0, isOdd = 1; i < n; i++, isOdd = !isOdd) {
        size_t offset = 6 * isOdd;
        float t = 2 * M_PI * ((i + 1) % n) / n;
        points[offset + 0] = points[offset + 3] = r * cos(t);
        points[offset + 1] = h / 2;
        points[offset + 4] = - h / 2;
        points[offset + 2] = points[offset + 5] = r * sin(t);
        makeQuad(points);
    }
}

void renderScene(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Очистка буфера цвета и глубины

    if (angle > 360) angle -= 360;

    float radius = 0.3 * k, height = 0.5 * k;

    glPushMatrix(); // сохраняет текущие координаты
    glMatrixMode(GL_PROJECTION); //GL_PROJECTION - применяем последующие матричные операции к стеку матриц проекции
    glLoadIdentity(); //заменяет текущую матрицу на единичную матрицу
    glLoadMatrixf(projectionMatrix); //заменяем текущую матрицу произвольной матрицей

    glMatrixMode(GL_MODELVIEW); ////GL_MODELVIEW - применяем последующие матричные операции к стеку матриц вида модели
    glLoadIdentity(); //заменяет текущую матрицу на единичную матрицу
    glTranslatef(coordX, 0.0f, 0.0f); //умножаем текущую матрицу на матрицу перевода (сдвигаем)
    glRotatef(angle, 1.0f, 0.0f, 0.0f); //умножает текущую матрицу на матрицу вращения (угол поворота, координаты вектора)

    glBegin(GL_TRIANGLES);
    makeCylinder(radius, height, n);
    glEnd();
    glPopMatrix(); // возвращаемся к старой системе координат

    glPushMatrix();
    glBegin(GL_POLYGON);
    glVertex3f( cubeX + alpha, cubeY - alpha, cubeZ - alpha );
    glVertex3f( cubeX + alpha,  cubeY + alpha, cubeZ - alpha );
    glVertex3f( cubeX + alpha,  cubeY + alpha,  cubeZ + alpha );
    glVertex3f( cubeX + alpha, cubeY - alpha,  cubeZ + alpha );
    glEnd();

    glBegin(GL_POLYGON);
    glVertex3f( cubeX - alpha, cubeY - alpha,  cubeZ + alpha );
    glVertex3f( cubeX - alpha,  cubeY + alpha,  cubeZ + alpha );
    glVertex3f( cubeX - alpha,  cubeY + alpha, cubeZ - alpha );
    glVertex3f( cubeX - alpha, cubeY - alpha, cubeZ - alpha );
    glEnd();

    glBegin(GL_POLYGON);
    glVertex3f(  cubeX + alpha,  cubeY + alpha,  cubeZ + alpha );
    glVertex3f(  cubeX + alpha,  cubeY + alpha, cubeZ - alpha );
    glVertex3f( cubeX - alpha,   cubeY + alpha, cubeZ - alpha );
    glVertex3f( cubeX - alpha,  cubeY + alpha,  cubeZ + alpha );
    glEnd();

    glBegin(GL_POLYGON);
    glVertex3f(  cubeX + alpha, cubeY - alpha, cubeZ - alpha );
    glVertex3f(  cubeX + alpha, cubeY - alpha,  cubeZ + alpha );
    glVertex3f( cubeX - alpha, cubeY - alpha,  cubeZ + alpha );
    glVertex3f( cubeX - alpha, cubeY - alpha, cubeZ - alpha );
    glEnd();
    glPopMatrix();

    glutSwapBuffers();//Выполняет подкачку буфера на слое, используемом для текущего окна.
    // Способствует тому, чтобы содержимое заднего буфера слоя, используемого текущим окном, стало содержимым переднего буфера.
    // Затем содержимое заднего буфера становится неопределенным.
}

void processNormalKeys(unsigned char key, int x, int y) {
    switch (key) {
        case ESC_KEY:
            exit(0);
        case 'q':
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //заполняем одним цветом
            break;
        case 'w':
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //выделяем линии
            break;
        case 'a':
            k -= 0.01;
            break;
        case 'd':
            k += 0.01;
            break;
        case 'r':
            if (n < 360) {
                n += 1;
            }
            break;
        case 't':
            if (n > 3) {
                n -= 1;
            }
            break;
        default:
            break;
    }
}

void processSpecialKeys(int key, int x, int y) {
    switch(key) {
        case GLUT_KEY_UP:
            angle += 1.0f;
            break;
        case GLUT_KEY_DOWN:
            angle -= 1.0f;
            break;
        case GLUT_KEY_LEFT:
            coordX -= 0.01f;
            break;
        case GLUT_KEY_RIGHT:
            coordX += 0.01f;
            break;
        default:
            break;
    }
}

int main(int argc, char **argv) {
    // инициализация
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(windowPositionX,windowPositionY);
    glutInitWindowSize(clientWidth, clientHeight);
    glViewport(windowPositionX, windowPositionY, clientWidth, clientHeight);
    glutCreateWindow("Kostyunina IU9-42B,Lab 2-3, 17 variant");
    // регистрация
    glutDisplayFunc(renderScene);
    glutIdleFunc(renderScene);
    glutKeyboardFunc(processNormalKeys);
    glutSpecialFunc(processSpecialKeys);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //выбирает режим растеризации многоугольника. Применяем лицевым и обратным полигонам - отображаем только ребра,линии
    glutMainLoop();
    return 1;
}