#ifndef FLYSCAPE_MATH_H
#define FLYSCAPE_MATH_H

#include <cmath>
#include <array>

struct Vec2 {
    float x = 0, y = 0;
};

struct Vec3 {
    float x = 0, y = 0, z = 0;

    Vec3() = default;
    Vec3(float X, float Y, float Z) :x(X), y(Y), z(Z) {}

    Vec3 operator+(const Vec3& r) const { return { x + r.x,y + r.y,z + r.z }; }
    Vec3 operator-(const Vec3& r) const { return { x - r.x,y - r.y,z - r.z }; }
    Vec3 operator*(float s) const { return { x * s,y * s,z * s }; }
    Vec3 operator/(float s) const { return { x / s,y / s,z / s }; }

    Vec3& operator+=(const Vec3& r) { x += r.x; y += r.y; z += r.z; return *this; }
};

inline float dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}
inline float length(const Vec3& v) { return std::sqrt(dot(v, v)); }
inline Vec3 normalize(const Vec3& v) {
    float len = length(v);
    if (len <= 1e-8f) return { 0,0,0 };
    return v / len;
}

struct Quat {
    float w = 1, x = 0, y = 0, z = 0;

    static Quat fromAxisAngle(const Vec3& axis, float rad) {
        Vec3 a = normalize(axis);
        float s = std::sin(rad * 0.5f);
        return { std::cos(rad * 0.5f), a.x * s, a.y * s, a.z * s };
    }

    Quat operator*(const Quat& r) const {
        return {
            w * r.w - x * r.x - y * r.y - z * r.z,
            w * r.x + x * r.w + y * r.z - z * r.y,
            w * r.y - x * r.z + y * r.w + z * r.x,
            w * r.z + x * r.y - y * r.x + z * r.w
        };
    }

    Quat& normalizeSelf() {
        float n = std::sqrt(w * w + x * x + y * y + z * z);
        if (n > 1e-8f) { w /= n; x /= n; y /= n; z /= n; }
        return *this;
    }
};

inline Vec3 rotate(const Quat& q, const Vec3& v) {
    // v' = q * (0,v) * q^-1
    Quat p{ 0, v.x, v.y, v.z };
    Quat qi{ q.w, -q.x, -q.y, -q.z };
    Quat r = q * p * qi;
    return { r.x, r.y, r.z };
}

struct Mat4 {
    // column-major
    std::array<float, 16> m{};

    static Mat4 identity() {
        Mat4 r;
        r.m = { 1,0,0,0,
               0,1,0,0,
               0,0,1,0,
               0,0,0,1 };
        return r;
    }
};

inline Mat4 mul(const Mat4& A, const Mat4& B) {
    Mat4 R;
    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            float s = 0;
            for (int k = 0; k < 4; k++) {
                s += A.m[k * 4 + r] * B.m[c * 4 + k];
            }
            R.m[c * 4 + r] = s;
        }
    }
    return R;
}

inline Mat4 translate(const Vec3& t) {
    Mat4 r = Mat4::identity();
    r.m[12] = t.x; r.m[13] = t.y; r.m[14] = t.z;
    return r;
}

inline Mat4 scale(const Vec3& s) {
    Mat4 r = Mat4::identity();
    r.m[0] = s.x; r.m[5] = s.y; r.m[10] = s.z;
    return r;
}

inline Mat4 fromQuat(const Quat& q) {
    Quat n = q; n.normalizeSelf();
    float w = n.w, x = n.x, y = n.y, z = n.z;

    Mat4 r = Mat4::identity();
    r.m[0] = 1 - 2 * y * y - 2 * z * z;
    r.m[1] = 2 * x * y + 2 * w * z;
    r.m[2] = 2 * x * z - 2 * w * y;

    r.m[4] = 2 * x * y - 2 * w * z;
    r.m[5] = 1 - 2 * x * x - 2 * z * z;
    r.m[6] = 2 * y * z + 2 * w * x;

    r.m[8] = 2 * x * z + 2 * w * y;
    r.m[9] = 2 * y * z - 2 * w * x;
    r.m[10] = 1 - 2 * x * x - 2 * y * y;
    return r;
}

inline Mat4 perspective(float fovyRad, float aspect, float zNear, float zFar) {
    float f = 1.0f / std::tan(fovyRad * 0.5f);
    Mat4 r{};
    r.m = { f / aspect,0,0,0,
            0,f,0,0,
            0,0,(zFar + zNear) / (zNear - zFar),-1,
            0,0,(2 * zFar * zNear) / (zNear - zFar),0 };
    return r;
}

inline Mat4 viewFromPose(const Vec3& pos, const Quat& orient) {
    // View = R^T * T^-1
    Mat4 R = fromQuat(orient);
    // transpose 3x3
    Mat4 Rt = Mat4::identity();
    Rt.m[0] = R.m[0]; Rt.m[1] = R.m[4]; Rt.m[2] = R.m[8];
    Rt.m[4] = R.m[1]; Rt.m[5] = R.m[5]; Rt.m[6] = R.m[9];
    Rt.m[8] = R.m[2]; Rt.m[9] = R.m[6]; Rt.m[10] = R.m[10];

    Mat4 T = translate({ -pos.x,-pos.y,-pos.z });
    return mul(Rt, T);
}

struct Mat3 {
    std::array<float, 9> m{};
};

inline Mat3 normalMatFromModel(const Mat4& model) {
    // 假设无非均匀缩放；若有非均匀缩放需 inverse-transpose
    Mat3 r{};
    r.m = {
        model.m[0], model.m[1], model.m[2],
        model.m[4], model.m[5], model.m[6],
        model.m[8], model.m[9], model.m[10]
    };
    return r;
}
#endif // FLYSCAPE_MATH_H
