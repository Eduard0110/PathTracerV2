#include "renderer/ShaderProgram.hpp"

#include <algorithm>
#include <stdexcept>
#include <vector>

void ShaderProgram::createAndCompile(const std::string& vertex_path, const std::string& fragment_path) {
    GLuint vertex_shader = 0;
    GLuint fragment_shader = 0;
    GLuint new_program = 0;

    try {
        vertex_shader = compile(GL_VERTEX_SHADER, loadShaderSource(vertex_path));
        fragment_shader = compile(GL_FRAGMENT_SHADER, loadShaderSource(fragment_path));

        new_program = glCreateProgram();
        glAttachShader(new_program, vertex_shader);
        glAttachShader(new_program, fragment_shader);
        glLinkProgram(new_program);

        GLint success = GL_FALSE;
        glGetProgramiv(new_program, GL_LINK_STATUS, &success);
        if (!success) {
            GLint log_length = 0;
            glGetProgramiv(new_program, GL_INFO_LOG_LENGTH, &log_length);
            std::vector<char> log(static_cast<std::size_t>(std::max(log_length, 1)));
            glGetProgramInfoLog(new_program, log_length, nullptr, log.data());
            throw std::runtime_error(
                "Shader program linking failed:\n" + std::string(log.data())
            );
        }
    }
    catch (...) {
        if (new_program != 0)
            glDeleteProgram(new_program);
        if (vertex_shader != 0)
            glDeleteShader(vertex_shader);
        if (fragment_shader != 0)
            glDeleteShader(fragment_shader);
        throw;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    if (m_id != 0)
        glDeleteProgram(m_id);

    m_id = new_program;
    m_uniform_cache.clear();
}
