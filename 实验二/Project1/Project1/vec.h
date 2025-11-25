#pragma once

struct Vec3 {
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float xx, float yy, float zz) : x(xx), y(yy), z(zz) {}
};

inline Vec3 operator+(const Vec3& a, const Vec3& b) {
    return Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}
inline Vec3 operator-(const Vec3& a, const Vec3& b) {
    return Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}
inline Vec3 operator*(const Vec3& a, float s) {
    return Vec3(a.x * s, a.y * s, a.z * s);
}
inline Vec3 operator*(float s, const Vec3& a) {
    return a * s;
}
