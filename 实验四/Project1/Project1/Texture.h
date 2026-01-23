#ifndef FLYSCAPE_TEXTURE_H
#define FLYSCAPE_TEXTURE_H

#include <string>
#include <vector>
#include <GL/glew.h>

struct ImageRGB {
    int w = 0, h = 0;
    std::vector<unsigned char> data; // w*h*3
};

class Texture2D {
public:
    Texture2D() = default;
    ~Texture2D();

    bool loadPPM_P6(const std::string& path, bool genMip = true);
    void bind(int unit = 0) const;

    static Texture2D makeWhite1x1();

    GLuint id() const { return tex_; }

private:
    GLuint tex_ = 0;

    static bool readPPM_P6(const std::string& path, ImageRGB& out);
};
#endif // FLYSCAPE_TEXTURE_H