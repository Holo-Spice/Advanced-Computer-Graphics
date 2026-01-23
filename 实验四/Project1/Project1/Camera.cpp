#include "Camera.h"

void Camera::reset() {
    pos = { 0, 8, 30 };
    orient = {};
    speed = 12.0f; 
}

Vec3 Camera::forward() const { return normalize(rotate(orient, { 0,0,-1 })); }
Vec3 Camera::up()      const { return normalize(rotate(orient, { 0,-1, 0 })); }
Vec3 Camera::right()   const { return normalize(rotate(orient, { 1,0, 0 })); }

void Camera::addYaw(float rad) {
    Quat q = Quat::fromAxisAngle(up(), rad);
    orient = (q * orient).normalizeSelf();
}
void Camera::addPitch(float rad) {
    Quat q = Quat::fromAxisAngle(right(), rad);
    orient = (q * orient).normalizeSelf();
}
void Camera::addRoll(float rad) {
    Quat q = Quat::fromAxisAngle(forward(), rad);
    orient = (q * orient).normalizeSelf();
}

Mat4 Camera::view() const {
    return viewFromPose(pos, orient);
}

void Camera::advance(float dt) {
    pos += forward() * (speed * dt);
}
