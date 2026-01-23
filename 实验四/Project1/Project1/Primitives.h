#ifndef FLYSCAPE_PRIMITIVES_H
#define FLYSCAPE_PRIMITIVES_H

#include <vector>
#include "Mesh.h"
#include "Math3D.h"

inline void makeCube(std::vector<Vertex>& out) {
    out.clear();
    // 36 verts (flat-ish normals per face)
    struct P { Vec3 p; Vec3 n; Vec2 uv; };
    const float s = 0.5f;
    auto add = [&](Vec3 p, Vec3 n, Vec2 uv) { out.push_back({ p,n,uv }); };

    // +X
    add({ s,-s,-s }, { 1,0,0 }, { 0,0 }); add({ s, s,-s }, { 1,0,0 }, { 1,0 }); add({ s, s, s }, { 1,0,0 }, { 1,1 });
    add({ s,-s,-s }, { 1,0,0 }, { 0,0 }); add({ s, s, s }, { 1,0,0 }, { 1,1 }); add({ s,-s, s }, { 1,0,0 }, { 0,1 });
    // -X
    add({ -s,-s, s }, { -1,0,0 }, { 0,0 }); add({ -s, s, s }, { -1,0,0 }, { 1,0 }); add({ -s, s,-s }, { -1,0,0 }, { 1,1 });
    add({ -s,-s, s }, { -1,0,0 }, { 0,0 }); add({ -s, s,-s }, { -1,0,0 }, { 1,1 }); add({ -s,-s,-s }, { -1,0,0 }, { 0,1 });
    // +Y
    add({ -s, s,-s }, { 0,1,0 }, { 0,0 }); add({ -s, s, s }, { 0,1,0 }, { 0,1 }); add({ s, s, s }, { 0,1,0 }, { 1,1 });
    add({ -s, s,-s }, { 0,1,0 }, { 0,0 }); add({ s, s, s }, { 0,1,0 }, { 1,1 }); add({ s, s,-s }, { 0,1,0 }, { 1,0 });
    // -Y
    add({ -s,-s, s }, { 0,-1,0 }, { 0,0 }); add({ -s,-s,-s }, { 0,-1,0 }, { 0,1 }); add({ s,-s,-s }, { 0,-1,0 }, { 1,1 });
    add({ -s,-s, s }, { 0,-1,0 }, { 0,0 }); add({ s,-s,-s }, { 0,-1,0 }, { 1,1 }); add({ s,-s, s }, { 0,-1,0 }, { 1,0 });
    // +Z
    add({ s,-s, s }, { 0,0,1 }, { 0,0 }); add({ s, s, s }, { 0,0,1 }, { 0,1 }); add({ -s, s, s }, { 0,0,1 }, { 1,1 });
    add({ s,-s, s }, { 0,0,1 }, { 0,0 }); add({ -s, s, s }, { 0,0,1 }, { 1,1 }); add({ -s,-s, s }, { 0,0,1 }, { 1,0 });
    // -Z
    add({ -s,-s,-s }, { 0,0,-1 }, { 0,0 }); add({ -s, s,-s }, { 0,0,-1 }, { 0,1 }); add({ s, s,-s }, { 0,0,-1 }, { 1,1 });
    add({ -s,-s,-s }, { 0,0,-1 }, { 0,0 }); add({ s, s,-s }, { 0,0,-1 }, { 1,1 }); add({ s,-s,-s }, { 0,0,-1 }, { 1,0 });
}

inline void makeSphere(std::vector<Vertex>& out, int slices = 24, int stacks = 16) {
    out.clear();
    auto sph = [](float u, float v) {
        float th = u * 2.0f * 3.1415926f;
        float ph = v * 3.1415926f;
        float x = std::sin(ph) * std::cos(th);
        float y = std::cos(ph);
        float z = std::sin(ph) * std::sin(th);
        return Vec3{ x,y,z };
        };

    for (int j = 0; j < stacks; j++) {
        float v0 = (float)j / stacks;
        float v1 = (float)(j + 1) / stacks;
        for (int i = 0; i < slices; i++) {
            float u0 = (float)i / slices;
            float u1 = (float)(i + 1) / slices;

            Vec3 p00 = sph(u0, v0), p10 = sph(u1, v0), p01 = sph(u0, v1), p11 = sph(u1, v1);

            // two triangles
            out.push_back({ p00, normalize(p00), {u0,v0} });
            out.push_back({ p01, normalize(p01), {u0,v1} });
            out.push_back({ p11, normalize(p11), {u1,v1} });

            out.push_back({ p00, normalize(p00), {u0,v0} });
            out.push_back({ p11, normalize(p11), {u1,v1} });
            out.push_back({ p10, normalize(p10), {u1,v0} });
        }
    }
}
#endif // FLYSCAPE_PRIMITIVES_H
