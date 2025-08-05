#include "Texture.hpp"

Texture::Texture(int width, int height)
    : m_width(width), m_height(height), m_id(0) {
}

Texture::~Texture() {
    glDeleteTextures(1, &m_id);
}

void Texture::clearToColor(float r, float g, float b, float a) const {
    float clearColor[4] = { r, g, b, a };
    glClearTexImage(m_id, 0, GL_RGBA, GL_FLOAT, clearColor);
}

void Texture::create() {
    glCreateTextures(GL_TEXTURE_2D, 1, &m_id);

    glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTextureStorage2D(m_id, 1, GL_RGBA8, m_width, m_height);
}

void Texture::bind(GLenum texture_unit) const {
    glBindTextureUnit(texture_unit, m_id);
}

void Texture::bindImageTexture() const {
    glBindImageTexture(0, m_id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
}