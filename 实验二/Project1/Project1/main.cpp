#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "freeglut.lib")
int main(int argc, char** argv) {
    // 初始化 GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    int width = 800, height = 600;
    glutInitWindowSize(width, height);
    glutCreateWindow("OBJ Viewer (OOP)");

    // 初始化 GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "GLEW init error: " << glewGetErrorString(err) << std::endl;
        return 1;
    }
    const char* filename = "model1.obj";
}