#pragma once

#include <glad/glad.h>

class Texture {
public:
    Texture();
    Texture(int width, int height, GLenum internal_format = GL_RGBA16F);
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    void bind(GLenum texture_unit) const;
    void bindImageTexture() const;
    void create();
    void create(int width, int height, const float* pixels);
    void clearToColor(float r, float g, float b, float a) const;
    void setWrap(GLenum wrap_s, GLenum wrap_t) const;

    GLuint getID() const { return m_id; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    GLenum getInternalFormat() const { return m_internal_format; }
private:
    GLuint m_id;

    int m_width;
    int m_height;
    GLenum m_internal_format;
};
