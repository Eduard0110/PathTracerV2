#include "renderer/Framebuffer.hpp"
#include <iostream>

Framebuffer::Framebuffer() = default;

Framebuffer::~Framebuffer() {
    destroy();
}

void Framebuffer::attachTexture(GLuint textureID, int width, int height, bool useDepth) {
    destroy(); // clean up existing FBO if recreating

    m_width = width;
    m_height = height;
    m_useDepth = useDepth;

    glCreateFramebuffers(1, &m_fbo);
    glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, textureID, 0);

    if (m_useDepth) {
        glCreateRenderbuffers(1, &m_depthRBO);
        glNamedRenderbufferStorage(m_depthRBO, GL_DEPTH_COMPONENT24, m_width, m_height);
        glNamedFramebufferRenderbuffer(m_fbo, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRBO);
    }

    if (glCheckNamedFramebufferStatus(m_fbo, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer incomplete with external texture!" << std::endl;
    }
}

void Framebuffer::bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);
}

void Framebuffer::unbind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::destroy() {
    if (m_depthRBO) glDeleteRenderbuffers(1, &m_depthRBO);
    if (m_fbo) glDeleteFramebuffers(1, &m_fbo);
    m_depthRBO = 0;
    m_fbo = 0;
}
