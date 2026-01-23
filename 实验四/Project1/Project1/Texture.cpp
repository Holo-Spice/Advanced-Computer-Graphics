#include "Texture.h"
#include <fstream>
#include <iostream>
#include <sstream>

Texture2D::~Texture2D() {
    if (tex_) glDeleteTextures(1, &tex_);
}

static void skipComments(std::istream& is) {
    while (true) {
        int c = is.peek();
        if (c == '#') {
            std::string line;
            std::getline(is, line);
        }
        else break;
    }
}

bool Texture2D::readPPM_P6(const std::string& path, ImageRGB& out) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;

    std::string magic;
    f >> magic;
    if (magic != "P6") return false;

    skipComments(f);
    int w, h, maxv;
    f >> w; skipComments(f);
    f >> h; skipComments(f);
    f >> maxv;
    f.get(); // consume one whitespace after header

    if (w <= 0 || h <= 0 || maxv <= 0) return false;
    if (maxv > 255) return false; // 简化处理

    out.w = w; out.h = h;
    out.data.resize((size_t)w * h * 3);
    f.read(reinterpret_cast<char*>(out.data.data()), (std::streamsize)out.data.size());
    return f.good();
}

bool Texture2D::loadPPM_P6(const std::string& path, bool genMip) {
    ImageRGB img;
    if (!readPPM_P6(path, img)) {
        std::cerr << "PPM load failed: " << path << "\n";
        return false;
    }

    if (!tex_) glGenTextures(1, &tex_);
    glBindTexture(GL_TEXTURE_2D, tex_);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, img.w, img.h, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, genMip ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);

    if (genMip) glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

void Texture2D::bind(int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, tex_);
}

Texture2D Texture2D::makeWhite1x1() {
    Texture2D t;
    glGenTextures(1, &t.tex_);
    glBindTexture(GL_TEXTURE_2D, t.tex_);
    unsigned char px[3] = { 255,255,255 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, px);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    return t;
}
