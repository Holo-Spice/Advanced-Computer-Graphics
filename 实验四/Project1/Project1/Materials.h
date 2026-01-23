#ifndef FLYSCAPE_MATERIALS_H
#define FLYSCAPE_MATERIALS_H

#include <vector>
#include "Math3D.h"

struct MaterialCPU {
    Vec3 ambient;
    Vec3 diffuse;
    Vec3 specular;
    float shininess = 32.0f;
};

inline std::vector<MaterialCPU> materialPresets() {
    return {
        {{0.08f,0.08f,0.08f},{0.70f,0.70f,0.70f},{0.90f,0.90f,0.90f},64.0f}, // metal-ish
        {{0.10f,0.08f,0.06f},{0.80f,0.60f,0.30f},{0.20f,0.20f,0.20f},16.0f}, // wood-ish
        {{0.05f,0.05f,0.10f},{0.20f,0.30f,0.80f},{0.90f,0.90f,0.90f},64.0f}, // plastic blue
        {{0.06f,0.10f,0.06f},{0.25f,0.70f,0.25f},{0.15f,0.15f,0.15f},8.0f},  // matte green
    };
}
#endif // FLYSCAPE_MATERIALS_H
