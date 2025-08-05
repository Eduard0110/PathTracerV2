#include "RectVAO.hpp"

RectVAO::~RectVAO() {
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
}

void RectVAO::createVao() {

    GLfloat vertices[] =
    {
        -1.0f, -1.0f , 0.0f, 0.0f, 0.0f,
        -1.0f,  1.0f , 0.0f, 0.0f, 1.0f,
         1.0f,  1.0f , 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f , 0.0f, 1.0f, 0.0f,
    };

    GLuint indices[] =
    {
        0, 2, 1,
        0, 3, 2
    };

    // create buffers
    glCreateVertexArrays(1, &m_VAO);
    glCreateBuffers(1, &m_VBO);
    glCreateBuffers(1, &m_EBO);

    glNamedBufferData(m_VBO, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glNamedBufferData(m_EBO, sizeof(indices), indices, GL_STATIC_DRAW);

    // vertex attribute: position (location = 0)
    glEnableVertexArrayAttrib(m_VAO, 0);
    glVertexArrayAttribFormat(m_VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(m_VAO, 0, 0);

    // vertex attribute: texcoord (location = 1)
    glEnableVertexArrayAttrib(m_VAO, 1);
    glVertexArrayAttribFormat(m_VAO, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat));
    glVertexArrayAttribBinding(m_VAO, 1, 0);

    // connect VBO and EBO
    glVertexArrayVertexBuffer(m_VAO, 0, m_VBO, 0, 5 * sizeof(GLfloat));
    glVertexArrayElementBuffer(m_VAO, m_EBO);
}