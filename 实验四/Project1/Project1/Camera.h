#ifndef FLYSCAPE_CAMERA_H
#define FLYSCAPE_CAMERA_H

#include "Math3D.h"

class Camera {
public:
    Vec3 pos{ 0, 8, 30 };
    Quat orient{}; // 飞机姿态（同时作为相机姿态）

    float speed = 8.0f; // units/s

    void reset();
    void addYaw(float rad);   // around local up
    void addPitch(float rad); // around local right
    void addRoll(float rad);  // around local forward

    Vec3 forward() const;
    Vec3 up() const;
    Vec3 right() const;

    Mat4 view() const;

    void advance(float dt); // 自动前进
};
#endif // FLYSCAPE_CAMERA_H
