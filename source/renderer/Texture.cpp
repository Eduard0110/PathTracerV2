#include "renderer/Texture.hpp"

Texture::Texture()
    : m_id(0), m_width(0), m_height(0), m_internal_format(GL_RGBA16F) {
}

Texture::Texture(int width, int height, GLenum internal_format)
    : m_id(0), m_width(width), m_height(height), m_internal_format(internal_format) {
}

Texture::~Texture() {
    glDeleteTextures(1, &m_id);
}

void Texture::clearToColor(float r, float g, float b, float a) const {
    float clearColor[4] = { r, g, b, a };
    glClearTexImage(m_id, 0, GL_RGBA, GL_FLOAT, clearColor);
}

void Texture::create() {
    if (m_width <= 0 || m_height <= 0)
        return;

    if (m_id != 0)
        glDeleteTextures(1, &m_id);

    glCreateTextures(GL_TEXTURE_2D, 1, &m_id);

    glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTextureStorage2D(m_id, 1, m_internal_format, m_width, m_height);
}

void Texture::create(int width, int height, const float* pixels) {
    m_width = width;
    m_height = height;

    create();

    if (m_id == 0 || pixels == nullptr)
        return;

    glTextureSubImage2D(
        m_id,
        0,
        0,
        0,
        m_width,
        m_height,
        GL_RGBA,
        GL_FLOAT,
        pixels
    );
}

void Texture::setWrap(GLenum wrap_s, GLenum wrap_t) const {
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, wrap_s);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, wrap_t);
}

void Texture::bind(GLenum texture_unit) const {
    glBindTextureUnit(texture_unit, m_id);
}

void Texture::bindImageTexture() const {
    glBindImageTexture(0, m_id, 0, GL_FALSE, 0, GL_WRITE_ONLY, m_internal_format);
}
