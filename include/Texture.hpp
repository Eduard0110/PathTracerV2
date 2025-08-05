#pragma once

#include <glad/glad.h>

class Texture {
public:
    Texture(int width, int height);
    ~Texture();

    void bind(GLenum texture_unit) const;
    void bindImageTexture() const;
    void create();
    void clearToColor(float r, float g, float b, float a) const;

    GLuint getID() const { return m_id; }
private:
    GLuint m_id;

    int m_width;
    int m_height;
};
