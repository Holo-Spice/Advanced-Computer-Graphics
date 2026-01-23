#include "ObjLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cctype>

static inline void stripUTF8BOM(std::string& s)
{
    // UTF-8 BOM: EF BB BF
    if (s.size() >= 3 &&
        (unsigned char)s[0] == 0xEF &&
        (unsigned char)s[1] == 0xBB &&
        (unsigned char)s[2] == 0xBF)
    {
        s.erase(0, 3);
    }
}

static inline void normalizeTag(std::string& tag)
{
    stripUTF8BOM(tag);
    // 去掉开头非字母（保险）
    while (!tag.empty() && !std::isalpha((unsigned char)tag[0])) {
        tag.erase(tag.begin());
    }
    // 转小写（保险）
    for (char& c : tag) c = (char)std::tolower((unsigned char)c);
}

static bool parseFaceToken(const std::string& tok, ObjIndex& out)
{
    // 支持: v | v/vt | v//vn | v/vt/vn
    out = {};

    int parts[3]{ 0,0,0 };
    int pi = 0;
    std::string cur;

    for (char ch : tok) {
        if (ch == '/') {
            if (pi > 2) break;
            parts[pi] = cur.empty() ? 0 : std::stoi(cur);
            cur.clear();
            ++pi;
        }
        else {
            cur.push_back(ch);
        }
    }
    if (pi <= 2) parts[pi] = cur.empty() ? 0 : std::stoi(cur);

    out.v = parts[0];
    out.vt = parts[1];
    out.vn = parts[2];

    return out.v != 0;
}

bool loadOBJ_V_VT_F(const std::string& path, ObjModel& out)
{
    std::ifstream f(path, std::ios::in);
    if (!f) {
        std::cerr << "OBJ open failed: " << path << "\n";
        return false;
    }

    out = {};
    std::string line;

    while (std::getline(f, line)) {
        if (line.empty()) continue;

        // 处理 BOM 在行首的情况（有些文件会把 BOM 附在第一行）
        stripUTF8BOM(line);

        // 跳过注释
        if (!line.empty() && line[0] == '#') continue;

        std::stringstream ss(line);
        std::string tag;
        ss >> tag;
        if (tag.empty()) continue;

        normalizeTag(tag);

        if (tag == "v") {
            float x = 0, y = 0, z = 0;
            ss >> x >> y >> z;
            out.positions.emplace_back(x, y, z);
        }
        else if (tag == "vt") {
            float s = 0, t = 0;
            ss >> s >> t;
            out.texcoords.push_back({ s,t });
        }
        else if (tag == "vn") {
            float x = 0, y = 0, z = 0;
            ss >> x >> y >> z;
            out.normals.emplace_back(x, y, z);
        }
        else if (tag == "f") {
            std::vector<ObjIndex> face;
            std::string tok;
            while (ss >> tok) {
                ObjIndex idx;
                if (parseFaceToken(tok, idx)) face.push_back(idx);
            }
            if (face.size() >= 3) out.faces.push_back(std::move(face));
        }
        // 其他 tag（o/g/s/usemtl/mtllib...）直接忽略
    }

    if (out.positions.empty() || out.faces.empty()) {
        std::cerr << "OBJ missing data. positions=" << out.positions.size()
            << " faces=" << out.faces.size()
            << " file=" << path << "\n";
        return false;
    }

    return true;
}
