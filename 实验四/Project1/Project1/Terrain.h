#ifndef FLYSCAPE_TERRAIN_H
#define FLYSCAPE_TERRAIN_H

#include <vector>
#include <string>
#include "Mesh.h"
#include "Texture.h"
#include "ObjLoader.h"

struct Triangle {
    Vec3 a, b, c;
    Vec3 n;
};

class Terrain {
public:
    bool load(const std::string& objPath, const std::string& ppmPath);

    void drawSmooth() const;
    void drawFlat() const;

    const Texture2D& texture() const { return tex_; }

    // 碰撞：给定 xz 查询高度；返回是否命中
    bool heightAt(float x, float z, float& outY, Vec3& outN) const;

    Vec3 minBound() const { return minB_; }
    Vec3 maxBound() const { return maxB_; }

private:
    Mesh smoothMesh_;
    Mesh flatMesh_;
    Texture2D tex_;

    std::vector<Triangle> tris_;
    Vec3 minB_{ 0,0,0 }, maxB_{ 0,0,0 };

    // 简单网格加速（XZ 平面分桶）
    int gridN_ = 64;
    Vec3 gridMin_{ 0,0,0 }, gridMax_{ 0,0,0 };
    float cellW_ = 1, cellD_ = 1;
    std::vector<std::vector<int>> cells_; // gridN_*gridN_ -> triangle indices

    void buildCollision(const ObjModel& obj);
    void buildMeshes(const ObjModel& obj);

    void buildGrid();
    int cellIndex(int gx, int gz) const { return gz * gridN_ + gx; }
};
#endif // FLYSCAPE_TERRAIN_H