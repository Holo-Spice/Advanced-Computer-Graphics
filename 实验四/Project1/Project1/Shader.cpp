#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

Shader::~Shader() {
    if (prog_) glDeleteProgram(prog_);
}

std::string Shader::readText(const std::string& path) {
    std::ifstream f(path, std::ios::in);
    if (!f) return {};
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

GLuint Shader::compile(GLenum type, const std::string& src) {
    GLuint s = glCreateShader(type);
    const char* c = src.c_str();
    glShaderSource(s, 1, &c, nullptr);
    glCompileShader(s);

    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0; glGetShaderiv(s, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');
        glGetShaderInfoLog(s, len, nullptr, log.data());
        std::cerr << "Shader compile failed:\n" << log << "\n";
        glDeleteShader(s);
        return 0;
    }
    return s;
}

bool Shader::loadFromFiles(const std::string& vsPath, const std::string& fsPath) {
    std::string vsrc = readText(vsPath);
    std::string fsrc = readText(fsPath);
    if (vsrc.empty() || fsrc.empty()) {
        std::cerr << "Cannot read shader files.\n";
        return false;
    }

    GLuint vs = compile(GL_VERTEX_SHADER, vsrc);
    GLuint fs = compile(GL_FRAGMENT_SHADER, fsrc);
    if (!vs || !fs) return false;

    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);

    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len = 0; glGetProgramiv(p, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');
        glGetProgramInfoLog(p, len, nullptr, log.data());
        std::cerr << "Program link failed:\n" << log << "\n";
        glDeleteProgram(p);
        return false;
    }

    if (prog_) glDeleteProgram(prog_);
    prog_ = p;
    cache_.clear();
    return true;
}

void Shader::use() const { glUseProgram(prog_); }

GLint Shader::uniform(const std::string& name) {
    auto it = cache_.find(name);
    if (it != cache_.end()) return it->second;
    GLint loc = glGetUniformLocation(prog_, name.c_str());
    cache_[name] = loc;
    return loc;
}

void Shader::setMat4(const std::string& name, const Mat4& v) {
    glUniformMatrix4fv(uniform(name), 1, GL_FALSE, v.m.data());
}
void Shader::setMat3(const std::string& name, const Mat3& v) {
    glUniformMatrix3fv(uniform(name), 1, GL_FALSE, v.m.data());
}
void Shader::setVec3(const std::string& name, const Vec3& v) {
    glUniform3f(uniform(name), v.x, v.y, v.z);
}
void Shader::setInt(const std::string& name, int v) {
    glUniform1i(uniform(name), v);
}
void Shader::setFloat(const std::string& name, float v) {
    glUniform1f(uniform(name), v);
}
