#pragma once
#include <string>
#include <vector>
#include "Math3D.h"

struct ObjIndex {
    int v = 0;   // 1-based
    int vt = 0;  // 1-based, 0 means not present
    int vn = 0;  // 1-based, 0 means not present
};

struct ObjModel {
    std::vector<Vec3> positions;   // v
    std::vector<Vec2> texcoords;   // vt
    std::vector<Vec3> normals;     // vn (可选)
    std::vector<std::vector<ObjIndex>> faces; // 多边形面（后续三角化）
};

bool loadOBJ_V_VT_F(const std::string& path, ObjModel& out);
