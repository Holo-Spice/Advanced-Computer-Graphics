#ifndef FLYSCAPE_SHADER_H
#define FLYSCAPE_SHADER_H

#include <string>
#include <unordered_map>
#include <GL/glew.h>
#include "Math3D.h"

class Shader {
public:
    Shader() = default;
    ~Shader();

    bool loadFromFiles(const std::string& vsPath, const std::string& fsPath);
    void use() const;

    GLint uniform(const std::string& name);

    void setMat4(const std::string& name, const Mat4& v);
    void setMat3(const std::string& name, const Mat3& v);
    void setVec3(const std::string& name, const Vec3& v);
    void setInt(const std::string& name, int v);
    void setFloat(const std::string& name, float v);

    GLuint id() const { return prog_; }

private:
    GLuint prog_ = 0;
    std::unordered_map<std::string, GLint> cache_;

    static std::string readText(const std::string& path);
    static GLuint compile(GLenum type, const std::string& src);
};
#endif // FLYSCAPE_SHADER_H