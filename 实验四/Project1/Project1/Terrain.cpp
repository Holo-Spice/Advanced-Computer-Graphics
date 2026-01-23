#include "Terrain.h"
#include <iostream>
#include <unordered_map>
#include <algorithm>

static Vec3 faceNormal(const Vec3& a, const Vec3& b, const Vec3& c) {
    return normalize(cross(b - a, c - a));
}

static bool pointInTriXZ(const Vec3& p, const Triangle& t) {
    // 在 XZ 投影上用重心判断
    Vec3 a{ t.a.x,0,t.a.z }, b{ t.b.x,0,t.b.z }, c{ t.c.x,0,t.c.z };
    Vec3 v0 = b - a, v1 = c - a, v2 = Vec3{ p.x,0,p.z } - a;
    float d00 = dot(v0, v0);
    float d01 = dot(v0, v1);
    float d11 = dot(v1, v1);
    float d20 = dot(v2, v0);
    float d21 = dot(v2, v1);
    float denom = d00 * d11 - d01 * d01;
    if (std::abs(denom) < 1e-8f) return false;
    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.0f - v - w;
    return (u >= 0 && v >= 0 && w >= 0);
}

static float interpYfromTriXZ(const Vec3& p, const Triangle& t) {
    // 用平面方程：n·(X - a) = 0，解 Y
    // n.x*(x-ax)+n.y*(y-ay)+n.z*(z-az)=0
    // y = ay - (n.x*(x-ax)+n.z*(z-az))/n.y
    if (std::abs(t.n.y) < 1e-6f) return t.a.y;
    float y = t.a.y - (t.n.x * (p.x - t.a.x) + t.n.z * (p.z - t.a.z)) / t.n.y;
    return y;
}

bool Terrain::load(const std::string& objPath, const std::string& ppmPath) {
    ObjModel obj;
    if (!loadOBJ_V_VT_F(objPath, obj)) return false;

    if (!tex_.loadPPM_P6(ppmPath, true)) {
        std::cerr << "Terrain texture load failed.\n";
        return false;
    }

    buildMeshes(obj);
    buildCollision(obj);
    buildGrid();
    return true;
}

void Terrain::drawSmooth() const { smoothMesh_.draw(); }
void Terrain::drawFlat() const { flatMesh_.draw(); }

void Terrain::buildMeshes(const ObjModel& obj) {
    // 先计算 smooth normals：按 position index 累加面法向
    std::vector<Vec3> acc(obj.positions.size(), { 0,0,0 });
    for (const auto& face : obj.faces) {
        // fan triangulation
        for (size_t i = 1; i + 1 < face.size(); i++) {
            int ia = face[0].v - 1;
            int ib = face[i].v - 1;
            int ic = face[i + 1].v - 1;
            Vec3 a = obj.positions[ia];
            Vec3 b = obj.positions[ib];
            Vec3 c = obj.positions[ic];
            Vec3 n = faceNormal(a, b, c);
            acc[ia] += n; acc[ib] += n; acc[ic] += n;
        }
    }
    for (auto& n : acc) n = normalize(n);

    // 如果没有 vt，做个平面映射
    Vec3 minB{ obj.positions[0].x, obj.positions[0].y, obj.positions[0].z };
    Vec3 maxB = minB;
    for (const auto& p : obj.positions) {
        minB.x = std::min(minB.x, p.x); minB.y = std::min(minB.y, p.y); minB.z = std::min(minB.z, p.z);
        maxB.x = std::max(maxB.x, p.x); maxB.y = std::max(maxB.y, p.y); maxB.z = std::max(maxB.z, p.z);
    }
    minB_ = minB; maxB_ = maxB;

    auto genUV = [&](const Vec3& p)->Vec2 {
        float sx = (p.x - minB.x) / std::max(1e-6f, (maxB.x - minB.x));
        float sz = (p.z - minB.z) / std::max(1e-6f, (maxB.z - minB.z));
        return { sx * 8.0f, sz * 8.0f }; // 平铺
        };

    // smooth mesh：用 map 去重 (posIndex, uvIndex)
    struct Key { int v; int vt; };
    struct KeyHash {
        size_t operator()(const Key& k) const { return (size_t)k.v * 1315423911u ^ (size_t)k.vt; }
    };
    struct KeyEq {
        bool operator()(const Key& a, const Key& b) const { return a.v == b.v && a.vt == b.vt; }
    };

    std::unordered_map<Key, unsigned int, KeyHash, KeyEq> map;
    std::vector<Vertex> verts;
    std::vector<unsigned int> indices;

    auto getIndex = [&](int vIdx, int vtIdx)->unsigned int {
        Key k{ vIdx, vtIdx };
        auto it = map.find(k);
        if (it != map.end()) return it->second;

        Vec3 p = obj.positions[vIdx];
        Vec2 uv{};
        if (vtIdx >= 0 && vtIdx < (int)obj.texcoords.size()) uv = obj.texcoords[vtIdx];
        else uv = genUV(p);

        Vertex vx{ p, acc[vIdx], uv };
        unsigned int id = (unsigned int)verts.size();
        verts.push_back(vx);
        map.emplace(k, id);
        return id;
        };

    for (const auto& face : obj.faces) {
        for (size_t i = 1; i + 1 < face.size(); i++) {
            int va = face[0].v - 1;
            int vb = face[i].v - 1;
            int vc = face[i + 1].v - 1;
            int ta = face[0].vt - 1;
            int tb = face[i].vt - 1;
            int tc = face[i + 1].vt - 1;

            indices.push_back(getIndex(va, ta));
            indices.push_back(getIndex(vb, tb));
            indices.push_back(getIndex(vc, tc));
        }
    }
    smoothMesh_.upload(verts, indices);

    // flat mesh：三角化 + 每三角形复制顶点，法线为面法线
    std::vector<Vertex> flatVerts;
    flatVerts.reserve(indices.size());
    for (const auto& face : obj.faces) {
        for (size_t i = 1; i + 1 < face.size(); i++) {
            int ia = face[0].v - 1;
            int ib = face[i].v - 1;
            int ic = face[i + 1].v - 1;

            Vec3 a = obj.positions[ia], b = obj.positions[ib], c = obj.positions[ic];
            Vec3 n = faceNormal(a, b, c);

            auto uvOf = [&](int vtIdx, const Vec3& p)->Vec2 {
                if (vtIdx >= 0 && vtIdx < (int)obj.texcoords.size()) return obj.texcoords[vtIdx];
                return genUV(p);
                };

            int ta = face[0].vt - 1;
            int tb = face[i].vt - 1;
            int tc = face[i + 1].vt - 1;

            flatVerts.push_back({ a,n, uvOf(ta,a) });
            flatVerts.push_back({ b,n, uvOf(tb,b) });
            flatVerts.push_back({ c,n, uvOf(tc,c) });
        }
    }
    flatMesh_.uploadNoIndex(flatVerts);
}

void Terrain::buildCollision(const ObjModel& obj) {
    tris_.clear();
    tris_.reserve(obj.faces.size() * 2);

    for (const auto& face : obj.faces) {
        for (size_t i = 1; i + 1 < face.size(); i++) {
            int ia = face[0].v - 1;
            int ib = face[i].v - 1;
            int ic = face[i + 1].v - 1;
            Triangle t;
            t.a = obj.positions[ia];
            t.b = obj.positions[ib];
            t.c = obj.positions[ic];
            t.n = faceNormal(t.a, t.b, t.c);
            tris_.push_back(t);
        }
    }

    gridMin_ = minB_;
    gridMax_ = maxB_;
    cellW_ = (gridMax_.x - gridMin_.x) / gridN_;
    cellD_ = (gridMax_.z - gridMin_.z) / gridN_;
    if (cellW_ < 1e-6f) cellW_ = 1.0f;
    if (cellD_ < 1e-6f) cellD_ = 1.0f;
}

void Terrain::buildGrid() {
    cells_.clear();
    cells_.resize((size_t)gridN_ * gridN_);

    auto clampi = [&](int v) { return std::max(0, std::min(gridN_ - 1, v)); };

    for (int ti = 0; ti < (int)tris_.size(); ++ti) {
        const auto& t = tris_[ti];
        float minx = std::min({ t.a.x,t.b.x,t.c.x });
        float maxx = std::max({ t.a.x,t.b.x,t.c.x });
        float minz = std::min({ t.a.z,t.b.z,t.c.z });
        float maxz = std::max({ t.a.z,t.b.z,t.c.z });

        int gx0 = clampi((int)((minx - gridMin_.x) / cellW_));
        int gx1 = clampi((int)((maxx - gridMin_.x) / cellW_));
        int gz0 = clampi((int)((minz - gridMin_.z) / cellD_));
        int gz1 = clampi((int)((maxz - gridMin_.z) / cellD_));

        for (int gz = gz0; gz <= gz1; ++gz) {
            for (int gx = gx0; gx <= gx1; ++gx) {
                cells_[(size_t)cellIndex(gx, gz)].push_back(ti);
            }
        }
    }
}

bool Terrain::heightAt(float x, float z, float& outY, Vec3& outN) const {
    if (x < gridMin_.x || x > gridMax_.x || z < gridMin_.z || z > gridMax_.z) return false;

    int gx = (int)((x - gridMin_.x) / cellW_);
    int gz = (int)((z - gridMin_.z) / cellD_);
    gx = std::max(0, std::min(gridN_ - 1, gx));
    gz = std::max(0, std::min(gridN_ - 1, gz));

    const auto& bucket = cells_[(size_t)cellIndex(gx, gz)];
    if (bucket.empty()) return false;

    Vec3 p{ x,0,z };
    bool hit = false;
    float bestY = -1e30f;
    Vec3 bestN{ 0,1,0 };

    for (int ti : bucket) {
        const Triangle& t = tris_[(size_t)ti];
        if (!pointInTriXZ(p, t)) continue;
        float y = interpYfromTriXZ(p, t);
        if (!hit || y > bestY) {
            hit = true;
            bestY = y;
            bestN = t.n;
        }
    }

    if (!hit) return false;
    outY = bestY;
    outN = bestN;
    return true;
}
