#include <GL/glew.h>
#include <GL/freeglut.h>

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

#include "ObjLoader.h"
#include "Shader.h"
#include "Terrain.h"
#include "Camera.h"
#include "Materials.h"
#include "Primitives.h"
#include "Texture.h"

#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "freeglut.lib")

#ifndef TERRAIN_SET
#define TERRAIN_SET 0   // 0=Puget, 1=GC
#endif

#if TERRAIN_SET == 0
static constexpr const char* TERRAIN_OBJ = "./puget_tex.obj";
static constexpr const char* TERRAIN_PPM = "./puget_tex.ppm";
#elif TERRAIN_SET == 1
static constexpr const char* TERRAIN_OBJ = "./gc_tex.obj";
static constexpr const char* TERRAIN_PPM = "./gc_tex.ppm";
#else
#error Unknown TERRAIN_SET
#endif

static int gWinW = 1280, gWinH = 720;

static Shader  gShader;
static Terrain gTerrain;
static Camera  gCam;

static Texture2D gWhiteTex;

static bool gKeys[256]{};
static bool gSpec[256]{};

static bool gUseSmooth = true;     // 1/2
static bool gDayMode = true;       // t
static bool gSpotOn = true;        // l
static bool gFogOn = true;         // f
static int  gFogMode = 0;          // g
static bool gCollisionOn = true;   // c

static bool gAutoFly = false;      // 空格：自动前进开关
static bool gThirdPerson = false;  // v：追尾视角开关

static int  gMatIndex = 0;
static std::vector<MaterialCPU> gMats = materialPresets();

struct SceneObj {
    Mesh mesh;
    Vec3 pos;
    float scale = 1.0f;
    float radius = 1.0f; // collision sphere
};

static std::vector<SceneObj> gObjs;

// 飞机（enterprise.obj）
static SceneObj gPlane;
static bool gPlaneOk = false;

// ✅ 太阳/月亮索引
static int gSunIdx = -1;
static int gMoonIdx = -1;

static float gLastT = 0.0f;

static Mat4 lookAtRH(const Vec3& eye, const Vec3& center, const Vec3& up)
{
    Vec3 f = normalize(center - eye);
    Vec3 s = normalize(cross(f, up));
    Vec3 u = cross(s, f);

    Mat4 M = Mat4::identity();

    M.m[0] = s.x;  M.m[4] = s.y;  M.m[8] = s.z;  M.m[12] = -dot(s, eye);
    M.m[1] = u.x;  M.m[5] = u.y;  M.m[9] = u.z;  M.m[13] = -dot(u, eye);
    M.m[2] = -f.x; M.m[6] = -f.y; M.m[10] = -f.z; M.m[14] = dot(f, eye);
    M.m[3] = 0.0f; M.m[7] = 0.0f; M.m[11] = 0.0f; M.m[15] = 1.0f;

    return M;
}

static Mat4 makeModelFromBasis(const Vec3& pos, const Vec3& r, const Vec3& u, const Vec3& f, float s)
{
    Vec3 rs = r * s;
    Vec3 us = u * s;
    Vec3 fs = f * s;

    Mat4 M = Mat4::identity();

    M.m[0] = rs.x;  M.m[4] = rs.y;  M.m[8] = rs.z;   M.m[12] = pos.x;
    M.m[1] = us.x;  M.m[5] = us.y;  M.m[9] = us.z;   M.m[13] = pos.y;
    M.m[2] = -fs.x; M.m[6] = -fs.y; M.m[10] = -fs.z;  M.m[14] = pos.z;
    M.m[3] = 0.0f;  M.m[7] = 0.0f;  M.m[11] = 0.0f;  M.m[15] = 1.0f;

    return M;
}

static bool sphereCollide(const Vec3& p, const SceneObj& o, float extra = 0.6f) {
    float r = o.radius * o.scale + extra;
    Vec3 d = p - o.pos;
    return (dot(d, d) <= r * r);
}

static void applyMaterial(const MaterialCPU& m) {
    gShader.setVec3("uMat.ambient", m.ambient);
    gShader.setVec3("uMat.diffuse", m.diffuse);
    gShader.setVec3("uMat.specular", m.specular);
    gShader.setFloat("uMat.shininess", m.shininess);
}

static void updateLights() {
    Vec3 dir = normalize(Vec3{ -0.2f, -1.0f, -0.3f });
    gShader.setVec3("uSun.direction", dir);

    if (gDayMode) {
        gShader.setVec3("uSun.color", { 1.0f, 0.98f, 0.90f });
        gShader.setFloat("uSun.intensity", 1.2f);
    }
    else {
        gShader.setVec3("uSun.color", { 0.60f, 0.70f, 1.0f });
        gShader.setFloat("uSun.intensity", 0.35f);
    }

    Vec3 spos = gCam.pos + gCam.forward() * 0.8f + gCam.up() * 0.1f;
    Vec3 sdir = gCam.forward();

    gShader.setVec3("uSpot.position", spos);
    gShader.setVec3("uSpot.direction", sdir);
    gShader.setVec3("uSpot.color", { 1.0f, 1.0f, 1.0f });
    gShader.setFloat("uSpot.intensity", 2.0f);
    gShader.setFloat("uSpot.innerCos", std::cos(12.0f * 3.1415926f / 180.0f));
    gShader.setFloat("uSpot.outerCos", std::cos(20.0f * 3.1415926f / 180.0f));
    gShader.setFloat("uSpot.range", 55.0f);
    gShader.setInt("uSpot.enabled", gSpotOn ? 1 : 0);
}

static void updateFog() {
    gShader.setInt("uFogEnabled", gFogOn ? 1 : 0);
    gShader.setInt("uFogMode", gFogMode);

    if (gDayMode) gShader.setVec3("uFogColor", { 0.70f,0.85f,0.95f });
    else          gShader.setVec3("uFogColor", { 0.04f,0.05f,0.08f });

    gShader.setFloat("uFogDensity", 0.02f);
    gShader.setFloat("uFogStart", 25.0f);
    gShader.setFloat("uFogEnd", 140.0f);
}

static bool loadObjAsSceneMesh_NoTex(const std::string& path, Mesh& outMesh, float& outRadius)
{
    ObjModel obj;
    if (!loadOBJ_V_VT_F(path, obj)) return false;

    std::vector<Vec3> acc(obj.positions.size(), { 0,0,0 });
    for (const auto& face : obj.faces) {
        for (size_t i = 1; i + 1 < face.size(); i++) {
            int ia = face[0].v - 1;
            int ib = face[i].v - 1;
            int ic = face[i + 1].v - 1;
            Vec3 a = obj.positions[ia], b = obj.positions[ib], c = obj.positions[ic];
            Vec3 n = normalize(cross(b - a, c - a));
            acc[ia] += n; acc[ib] += n; acc[ic] += n;
        }
    }
    for (auto& n : acc) n = normalize(n);

    std::vector<Vertex> verts;

    Vec3 minB = obj.positions[0], maxB = obj.positions[0];
    for (const auto& p : obj.positions) {
        minB.x = std::min(minB.x, p.x); minB.y = std::min(minB.y, p.y); minB.z = std::min(minB.z, p.z);
        maxB.x = std::max(maxB.x, p.x); maxB.y = std::max(maxB.y, p.y); maxB.z = std::max(maxB.z, p.z);
    }
    Vec3 center = (minB + maxB) * 0.5f;
    outRadius = 0.0f;
    for (const auto& p : obj.positions) outRadius = std::max(outRadius, length(p - center));

    for (const auto& face : obj.faces) {
        for (size_t i = 1; i + 1 < face.size(); i++) {
            int ia = face[0].v - 1;
            int ib = face[i].v - 1;
            int ic = face[i + 1].v - 1;
            verts.push_back({ obj.positions[ia], acc[ia], {0,0} });
            verts.push_back({ obj.positions[ib], acc[ib], {0,0} });
            verts.push_back({ obj.positions[ic], acc[ic], {0,0} });
        }
    }

    outMesh.uploadNoIndex(verts);
    return true;
}

static void initScene() {
    gWhiteTex = Texture2D::makeWhite1x1();

    if (!gShader.loadFromFiles("./phong.vert", "./phong.frag")) {
        std::cerr << "Shader load failed.\n";
        std::exit(1);
    }

    if (!gTerrain.load(TERRAIN_OBJ, TERRAIN_PPM)) {
        std::cerr << "Terrain load failed: " << TERRAIN_OBJ << " / " << TERRAIN_PPM << "\n";
        std::exit(1);
    }

    // plane
    {
        float r = 1.0f;
        gPlaneOk = loadObjAsSceneMesh_NoTex("./enterprise.obj", gPlane.mesh, r);
        if (!gPlaneOk) std::cerr << "enterprise.obj load failed (plane)\n";
        gPlane.scale = 1.5f;
        gPlane.radius = r;
    }

    // ✅ sun + moon spheres
    {
        std::vector<Vertex> v;
        makeSphere(v, 28, 18);

        SceneObj sun;
        sun.mesh.uploadNoIndex(v);
        sun.pos = { -80, 140, -260 };
        sun.scale = 18.0f;  // 大
        sun.radius = 1.0f;
        gSunIdx = (int)gObjs.size();
        gObjs.push_back(std::move(sun));

        SceneObj moon;
        moon.mesh.uploadNoIndex(v);
        moon.pos = { 90, 120, -240 };
        moon.scale = 10.0f; // 小
        moon.radius = 1.0f;
        gMoonIdx = (int)gObjs.size();
        gObjs.push_back(std::move(moon));
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    gCam.reset();
    gLastT = (float)glutGet(GLUT_ELAPSED_TIME) * 0.001f;
}

static void reshape(int w, int h) {
    gWinW = w; gWinH = h;
    glViewport(0, 0, w, h);
}

static void drawOne(const Mesh& mesh, const Mat4& model, bool useTex, const Texture2D& tex) {
    gShader.setMat4("uModel", model);
    gShader.setMat3("uNormalMat", normalMatFromModel(model));

    gShader.setInt("uUseTexture", useTex ? 1 : 0);
    tex.bind(0);
    gShader.setInt("uTexture", 0);

    mesh.draw();
}

static void display() {
    glClearColor(0.07f, 0.08f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gShader.use();

    float aspect = (gWinH == 0) ? 1.0f : (float)gWinW / (float)gWinH;
    Mat4 P = perspective(60.0f * 3.1415926f / 180.0f, aspect, 0.1f, 500.0f);

    Mat4 V;
    if (!gThirdPerson) {
        V = gCam.view();
    }
    else {
        Vec3 planePos = gCam.pos;
        Vec3 eye = planePos - gCam.forward() * 8.0f + gCam.up() * 6.0f;
        Vec3 center = planePos + gCam.forward() * 30.0f;
        V = lookAtRH(eye, center, gCam.up());
    }

    gShader.setMat4("uProj", P);
    gShader.setMat4("uView", V);
    gShader.setVec3("uViewPos", gCam.pos);

    updateLights();
    updateFog();

    // ===== Terrain =====
    gShader.setInt("uUnlit", 0); // ✅ 确保地形走正常光照
    applyMaterial({ {0.05f,0.05f,0.05f},{1.0f,1.0f,1.0f},{0.10f,0.10f,0.10f}, 16.0f });

    Mat4 Mterrain = Mat4::identity();
    gShader.setMat4("uModel", Mterrain);
    gShader.setMat3("uNormalMat", normalMatFromModel(Mterrain));
    gShader.setInt("uUseTexture", 1);
    gTerrain.texture().bind(0);
    gShader.setInt("uTexture", 0);

    if (gUseSmooth) gTerrain.drawSmooth();
    else gTerrain.drawFlat();

    // ===== Objects =====
    const MaterialCPU& mm = gMats[(size_t)gMatIndex % gMats.size()];

    for (size_t i = 0; i < gObjs.size(); ++i) {
        const SceneObj& o = gObjs[i];
        Mat4 M = mul(translate(o.pos), scale({ o.scale,o.scale,o.scale }));

        // ✅ 太阳/月亮：强制“自发光颜色”，保证绝对是红橙/银白
        if ((int)i == gSunIdx) {
            gShader.setInt("uUnlit", 1);
            // 白天更亮，夜晚变暗
            if (gDayMode) gShader.setVec3("uUnlitColor", { 1.0f, 0.45f, 0.10f }); // 红橙太阳
            else          gShader.setVec3("uUnlitColor", { 0.18f, 0.10f, 0.05f });
            drawOne(o.mesh, M, false, gWhiteTex);
            continue;
        }
        if ((int)i == gMoonIdx) {
            gShader.setInt("uUnlit", 1);
            // 夜晚更亮，白天变暗
            if (gDayMode) gShader.setVec3("uUnlitColor", { 0.25f, 0.25f, 0.30f });
            else          gShader.setVec3("uUnlitColor", { 0.85f, 0.85f, 0.92f }); // 银白月亮
            drawOne(o.mesh, M, false, gWhiteTex);
            continue;
        }

        // 其他物体：正常材质光照
        gShader.setInt("uUnlit", 0);
        applyMaterial(mm);
        drawOne(o.mesh, M, false, gWhiteTex);
    }

    // ===== Plane (third person only) =====
    if (gPlaneOk && gThirdPerson) {
        gShader.setInt("uUnlit", 0);
        applyMaterial(mm);

        Vec3 planePos = gCam.pos;
        Mat4 Mplane = makeModelFromBasis(planePos, gCam.right(), gCam.up(), gCam.forward(), gPlane.scale);
        drawOne(gPlane.mesh, Mplane, false, gWhiteTex);
    }

    glutSwapBuffers();
}

static void onKeyDown(unsigned char key, int, int) {
    gKeys[key] = true;

    if (key == 27) std::exit(0);
    if (key == '1') gUseSmooth = false;
    if (key == '2') gUseSmooth = true;

    if (key == 't') gDayMode = !gDayMode;
    if (key == 'l') gSpotOn = !gSpotOn;
    if (key == 'f') gFogOn = !gFogOn;
    if (key == 'g') gFogMode = (gFogMode + 1) % 3;
    if (key == 'c') gCollisionOn = !gCollisionOn;

    if (key == 'm') gMatIndex = (gMatIndex + 1) % (int)gMats.size();
    if (key == 'r') gCam.reset();

    if (key == ' ') gAutoFly = !gAutoFly;
    if (key == 'v') gThirdPerson = !gThirdPerson;
}

static void onKeyUp(unsigned char key, int, int) { gKeys[key] = false; }

static void onSpecialDown(int key, int, int) {
    if (key >= 0 && key < 256) gSpec[key] = true;
}
static void onSpecialUp(int key, int, int) {
    if (key >= 0 && key < 256) gSpec[key] = false;
}

static void step() {
    float t = (float)glutGet(GLUT_ELAPSED_TIME) * 0.001f;
    float dt = std::min(0.033f, t - gLastT);
    gLastT = t;

    const float yawRate = 1.1f;
    const float pitchRate = 1.0f;
    const float rollRate = 1.2f;

    if (gSpec[GLUT_KEY_LEFT])  gCam.addYaw(+yawRate * dt);
    if (gSpec[GLUT_KEY_RIGHT]) gCam.addYaw(-yawRate * dt);
    if (gSpec[GLUT_KEY_UP])    gCam.addPitch(-pitchRate * dt);
    if (gSpec[GLUT_KEY_DOWN])  gCam.addPitch(+pitchRate * dt);

    if (gKeys[(unsigned char)'a']) gCam.addRoll(-rollRate * dt);
    if (gKeys[(unsigned char)'d']) gCam.addRoll(+rollRate * dt);

    if (gKeys[(unsigned char)'-']) gCam.speed = std::max(0.0f, gCam.speed - 12.0f * dt);
    if (gKeys[(unsigned char)'='] || gKeys[(unsigned char)'+'])
        gCam.speed = std::min(80.0f, gCam.speed + 12.0f * dt);

    Vec3 oldPos = gCam.pos;

    if (gAutoFly) {
        gCam.advance(dt);
    }
    else {
        float move = gCam.speed * dt;
        if (gKeys[(unsigned char)'w']) gCam.pos = gCam.pos + gCam.forward() * move;
        if (gKeys[(unsigned char)'s']) gCam.pos = gCam.pos - gCam.forward() * move;
        if (gKeys[(unsigned char)'q']) gCam.pos = gCam.pos + gCam.up() * move;
        if (gKeys[(unsigned char)'e']) gCam.pos = gCam.pos - gCam.up() * move;
        if (gKeys[(unsigned char)'z']) gCam.pos = gCam.pos - gCam.right() * move;
        if (gKeys[(unsigned char)'x']) gCam.pos = gCam.pos + gCam.right() * move;
    }

    if (gCollisionOn) {
        float y; Vec3 n;
        if (gTerrain.heightAt(gCam.pos.x, gCam.pos.z, y, n)) {
            float clearance = 2.0f;
            if (gCam.pos.y < y + clearance) gCam.pos.y = y + clearance;
        }
        else {
            gCam.pos.y = std::max(gCam.pos.y, gTerrain.minBound().y + 2.0f);
        }

        // ✅ 跳过太阳/月亮碰撞
        for (size_t i = 0; i < gObjs.size(); ++i) {
            if ((int)i == gSunIdx || (int)i == gMoonIdx) continue;
            if (sphereCollide(gCam.pos, gObjs[i])) {
                gCam.pos = oldPos;
                break;
            }
        }
    }

    glutPostRedisplay();
}

static void timer(int) {
    step();
    glutTimerFunc(16, timer, 0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

    glutInitContextVersion(3, 3);
    glutInitContextProfile(GLUT_CORE_PROFILE);

    glutInitWindowSize(gWinW, gWinH);
    glutCreateWindow("Flight Terrain - OpenGL+GLSL+FreeGLUT");

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "GLEW init failed.\n";
        return 1;
    }

    initScene();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(onKeyDown);
    glutKeyboardUpFunc(onKeyUp);
    glutSpecialFunc(onSpecialDown);
    glutSpecialUpFunc(onSpecialUp);
    glutTimerFunc(16, timer, 0);

    glutMainLoop();
    return 0;
}
