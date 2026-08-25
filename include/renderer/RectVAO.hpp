#pragma once

#include <glad/glad.h>

class RectVAO {
public:
    RectVAO() = default;
    ~RectVAO();

    RectVAO(const RectVAO&) = delete;
    RectVAO& operator=(const RectVAO&) = delete;

    void createVao();
    void bind() const { glBindVertexArray(m_VAO); };
    void unbind() const { glBindVertexArray(0); }

    GLuint getVAOID() const { return m_VAO; }

private:
    GLuint m_VBO = 0;
    GLuint m_VAO = 0;
    GLuint m_EBO = 0;
};
