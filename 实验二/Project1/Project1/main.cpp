#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include "App.h"

#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "freeglut.lib")

static App* g_app = nullptr;

static const char* g_models[3] = {
    "bunny_color.obj",
    "cow_color.obj",
    "v1_color.obj"
};

// 回调转发
void displayCB() { if (g_app) g_app->onDisplay(); }
void reshapeCB(int w, int h) { if (g_app) g_app->onReshape(w, h); }
void keyboardCB(unsigned char k, int x, int y)
{
    if (g_app) g_app->onKeyboard(k, x, y);
}
void specialCB(int k, int x, int y)
{
    if (g_app) g_app->onSpecialKey(k, x, y);
}
void mouseCB(int b, int s, int x, int y)
{
    if (g_app) g_app->onMouse(b, s, x, y);
}
void motionCB(int x, int y)
{
    if (g_app) g_app->onMotion(x, y);
}

// 右键菜单回调
void menuCB(int item)
{
    if (!g_app) return;

    switch (item)
    {
    case 1:
        if (!g_app->reloadModel(g_models[0]))
            std::cerr << "Failed to load model: " << g_models[0] << "\n";
        break;
    case 2:
        if (!g_app->reloadModel(g_models[1]))
            std::cerr << "Failed to load model: " << g_models[1] << "\n";
        break;
    case 3:
        if (!g_app->reloadModel(g_models[2]))
            std::cerr << "Failed to load model: " << g_models[2] << "\n";
        break;
    case 99:
        std::exit(0); // 退出
        break;
    default:
        break;
    }

    glutPostRedisplay();  // 切换模型后，手动触发重绘
}


int main(int argc, char** argv)
{
    std::cout << "选择模型 (1-3): ";
    int choice = 0;
    std::cin >> choice;
    if (choice < 1 || choice > 3)
    {
        std::cerr << "无效输入\n";
        return 1;
    }
    const char* filename = g_models[choice - 1];

    // 初始化 GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    int width = 800, height = 600;
    glutInitWindowSize(width, height);
    glutCreateWindow("OBJ Viewer (OOP)");

    // 初始化 GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        std::cerr << "GLEW init error: " << glewGetErrorString(err) << std::endl;
        return 1;
    }

    // 创建 App
    App app(width, height, "OBJ Viewer (OOP)", filename);
    if (!app.init())
    {
        return 1;
    }
    g_app = &app;

    // 注册回调
    glutDisplayFunc(displayCB);
    glutReshapeFunc(reshapeCB);
    glutKeyboardFunc(keyboardCB);
    glutSpecialFunc(specialCB);
    glutMouseFunc(mouseCB);
    glutMotionFunc(motionCB);

    // 创建右键菜单
    int menuId = glutCreateMenu(menuCB);
    glutAddMenuEntry("Bunny", 1);
    glutAddMenuEntry("Cow", 2);
    glutAddMenuEntry("V1", 3);
    glutAddMenuEntry("Exit", 99);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutMainLoop();
    return 0;
}
