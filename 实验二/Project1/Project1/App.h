#pragma once
#include <string>
#include "Model.h"
#include "ShaderProgram.h"
class App
{
public:
    App(int width,
        int height,
        const std::string& title,
        const std::string& objFile);

    bool init();

    void onDisplay();
    void onReshape(int w, int h);
    void onKeyboard(unsigned char key, int x, int y);
    void onSpecialKey(int key, int x, int y);
    void onMouse(int button, int state, int x, int y);
    void onMotion(int x, int y);
    bool reloadModel(const std::string& objFile);  

private:
    void initGLStates();
    void initShader();

    Model m_model;
    ShaderProgram m_shader;

    int m_width;
    int m_height;
    std::string m_title;

    // 变换控制
    float m_transX;
    float m_transY;
    float m_zoom;
    float m_yaw;
    float m_pitch;

    bool  m_rotating;
    int   m_lastMouseX;
    int   m_lastMouseY;
};

