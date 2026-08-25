#pragma once

#include <glad/glad.h>

class Framebuffer {
public:
    Framebuffer();
    ~Framebuffer();

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    void attachTexture(GLuint textureID, int width, int height, bool useDepth = false);

    void bind() const;
    void unbind() const;

private:
    void destroy();

private:
    int m_width = 0;
    int m_height = 0;
    bool m_useDepth = false;

    GLuint m_fbo = 0;
    GLuint m_depthRBO = 0;
};
