#ifndef FLYSCAPE_MESH_H
#define FLYSCAPE_MESH_H

#include <vector>
#include <GL/glew.h>
#include "Math3D.h"

struct Vertex {
    Vec3 pos;
    Vec3 nrm;
    Vec2 uv;
};

class Mesh {
public:
    Mesh() = default;
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    // ✅ 新增：允许移动
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    void upload(const std::vector<Vertex>& verts, const std::vector<unsigned int>& indices);
    void uploadNoIndex(const std::vector<Vertex>& verts);

    void draw() const;

    bool hasIndex() const { return hasIndex_; }
    size_t indexCount() const { return indexCount_; }
    size_t vertexCount() const { return vertexCount_; }

private:
    GLuint vao_ = 0, vbo_ = 0, ebo_ = 0;
    bool hasIndex_ = false;
    size_t indexCount_ = 0;
    size_t vertexCount_ = 0;
};

#endif // FLYSCAPE_MESH_H