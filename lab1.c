#include <stdlib.h>
#include <GL/freeglut.h>
#include <math.h>

float red=1.0f, blue=1.0f, green=1.0f;

void renderScene(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Очистка буфера цвета и глубины

    //рисуем круг
    float x, y;
    float cnt = 40;
    float l = 0.5;
    float a = M_PI * 2 / cnt;
    glBegin(GL_TRIANGLE_FAN); //определяет границы, внутри которых заданы вершины примитива или группы примитивов.работает с буфером накопления
    glColor3f(red,green,blue); //устанавливаем цвет
    glVertex2f(0, 0); //устанавливаем вершины
    for (int i = -1; i < cnt; i++) {
        x = sin(a * i) * l;
        y = cos(a * i) * l;
        glVertex2f(x, y);
    }
    glEnd();//завершаем рисование

    glutSwapBuffers();//Выполняет подкачку буфера на слое, используемом для текущего окна.
    // Способствует тому, чтобы содержимое заднего буфера слоя, используемого текущим окном, стало содержимым переднего буфера.
    // Затем содержимое заднего буфера становится неопределенным.
}

void processNormalKeys(unsigned char key, int x, int y) {
    if (key == 27) //ASCII-код Esc, при нажатии бесконечный цикл прерывается и происходит выход из приложения
        exit(0);
}

void processSpecialKeys(int key, int x, int y) {
    switch(key) {
        case GLUT_KEY_UP : //при нажатии стрелки вверх цвет фигуры меняется на красный
            red = 1.0f;
            green = 0.0f;
            blue = 0.0f;
            break;
        case GLUT_KEY_DOWN : //при нажатии стрелки вверх цвет фигуры меняется на зелёный
            red = 0.0f;
            green = 1.0f;
            blue = 0.0f;
            break;
    }
}

int main(int argc, char **argv) {
    // инициализация
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(400,400);
    glutCreateWindow("Changing color");
    // регистрация
    glutDisplayFunc(renderScene);
    glutIdleFunc(renderScene);
    glutKeyboardFunc(processNormalKeys);
    glutSpecialFunc(processSpecialKeys);
    // основной цикл
    glutMainLoop();
    return 1;
}