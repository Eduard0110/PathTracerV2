#include "ShaderProgram.hpp"

void ShaderProgram::createAndCompile(const std::string& vertex_path, const std::string& fragment_path) {
    std::string vertex_src = loadShaderSource(vertex_path);
    std::string fragment_src = loadShaderSource(fragment_path);

    GLuint vertex_shader = compile(GL_VERTEX_SHADER, vertex_src);
    GLuint fragment_shader = compile(GL_FRAGMENT_SHADER, fragment_src);

    m_id = glCreateProgram();
    glAttachShader(m_id, vertex_shader);
    glAttachShader(m_id, fragment_shader);
    glLinkProgram(m_id);

    GLint success;
    glGetProgramiv(m_id, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(m_id, 512, nullptr, info_log);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << info_log << std::endl;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}