#include "App.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

App::App(int width,
    int height,
    const std::string& title,
    const std::string& objFile)
    : m_width(width)
    , m_height(height)
    , m_title(title)
    , m_transX(0.0f)
    , m_transY(0.0f)
    , m_zoom(1.0f)
    , m_yaw(0.0f)
    , m_pitch(0.0f)
    , m_rotating(false)
    , m_lastMouseX(0)
    , m_lastMouseY(0)
{
    if (!reloadModel(objFile))
    {
        std::cerr << "Failed to load model file.\n";
    }
}

bool App::init()
{
    if (m_model.isEmpty())
    {
        std::cerr << "Model is empty, cannot init.\n";
        return false;
    }

    initGLStates();
    initShader();
    return true;
}

void App::initGLStates()
{
    glClearColor(0.6f, 0.7f, 0.9f, 1.0f);
    glEnable(GL_DEPTH_TEST);
}

void App::initShader()
{
    const std::string vsSrc =
        "#version 120\n"
        "attribute vec3 aPos;\n"
        "attribute vec3 aColor;\n"
        "varying vec3 vColor;\n"
        "uniform mat4 uMVP;\n"
        "void main() {\n"
        "  vColor = aColor;\n"
        "  gl_Position = uMVP * vec4(aPos, 1.0);\n"
        "}\n";

    const std::string fsSrc =
        "#version 120\n"
        "varying vec3 vColor;\n"
        "void main() {\n"
        "  gl_FragColor = vec4(vColor, 1.0);\n"
        "}\n";

    if (!m_shader.createFromSource(vsSrc, fsSrc))
    {
        std::cerr << "Failed to create shader program.\n";
    }
}

void App::onDisplay()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shader.use();

    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(m_transX, m_transY, 0.0f));
    model = glm::scale(model, glm::vec3(m_zoom));
    model = glm::rotate(model, glm::radians(m_pitch), glm::vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(m_yaw), glm::vec3(0, 1, 0));

    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 3.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    float aspect = static_cast<float>(m_width) / static_cast<float>(m_height);
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 10.0f);

    glm::mat4 mvp = proj * view * model;

    GLint loc = m_shader.getUniformLocation("uMVP");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mvp));

    m_model.draw();

    glUseProgram(0);
    glutSwapBuffers();
}

void App::onReshape(int w, int h)
{
    m_width = (w == 0 ? 1 : w);
    m_height = (h == 0 ? 1 : h);
    glViewport(0, 0, m_width, m_height);
    glutPostRedisplay();
}

void App::onKeyboard(unsigned char key, int, int)
{
    const float moveStep = 0.1f;
    const float zoomScale = 1.1f;

    switch (key)
    {
    case 27: // ESC
        std::exit(0);
        break;
    case 'w': case 'W':
        m_transY += moveStep; break;
    case 's': case 'S':
        m_transY -= moveStep; break;
    case 'a': case 'A':
        m_transX -= moveStep; break;
    case 'd': case 'D':
        m_transX += moveStep; break;
    case 'z':         // 放大
        m_zoom *= zoomScale;    break;
    case 'Z':                  // 缩小
        m_zoom /= zoomScale;    break;
    default:
        break;
    }
    glutPostRedisplay();
}

void App::onSpecialKey(int key, int, int)
{
    const float moveStep = 0.1f;

    switch (key)
    {
    case GLUT_KEY_UP:
        m_transY += moveStep; break;
    case GLUT_KEY_DOWN:
        m_transY -= moveStep; break;
    case GLUT_KEY_LEFT:
        m_transX -= moveStep; break;
    case GLUT_KEY_RIGHT:
        m_transX += moveStep; break;
    default:
        break;
    }
    glutPostRedisplay();
}

void App::onMouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            m_rotating = true;
            m_lastMouseX = x;
            m_lastMouseY = y;
        }
        else if (state == GLUT_UP)
        {
            m_rotating = false;
        }
    }
}

void App::onMotion(int x, int y)
{
    if (!m_rotating) return;

    float dx = static_cast<float>(x - m_lastMouseX);
    float dy = static_cast<float>(y - m_lastMouseY);

    float speed = 0.5f;
    m_yaw += dx * speed;
    m_pitch += dy * speed;

    m_lastMouseX = x;
    m_lastMouseY = y;

    glutPostRedisplay();
}

bool App::reloadModel(const std::string& objFile)
{
    if (!m_model.LoadModelFile(objFile))
    {
        std::cerr << "Failed to load model file: " << objFile << "\n";
        return false;
    }
    if (m_model.isEmpty())
    {
        std::cerr << "Model is empty.\n";
        return false;
    }

    m_model.uploadToGPU();  

    m_transX = 0.0f;
    m_transY = 0.0f;
    m_yaw = 0.0f;
    m_pitch = 0.0f;

    return true;
}
