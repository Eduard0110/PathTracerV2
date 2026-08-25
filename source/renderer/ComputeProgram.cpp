#include "renderer/ComputeProgram.hpp"

#include <algorithm>
#include <stdexcept>
#include <vector>

void ComputeProgram::createAndCompile(const std::string& compute_shader_path) {
    GLuint compute_shader = 0;
    GLuint new_program = 0;

    try {
        compute_shader = compile(
            GL_COMPUTE_SHADER,
            loadShaderSource(compute_shader_path)
        );

        new_program = glCreateProgram();
        glAttachShader(new_program, compute_shader);
        glLinkProgram(new_program);

        GLint success = GL_FALSE;
        glGetProgramiv(new_program, GL_LINK_STATUS, &success);
        if (!success) {
            GLint log_length = 0;
            glGetProgramiv(new_program, GL_INFO_LOG_LENGTH, &log_length);
            std::vector<char> log(static_cast<std::size_t>(std::max(log_length, 1)));
            glGetProgramInfoLog(new_program, log_length, nullptr, log.data());
            throw std::runtime_error(
                "Compute program linking failed:\n" + std::string(log.data())
            );
        }
    }
    catch (...) {
        if (new_program != 0)
            glDeleteProgram(new_program);
        if (compute_shader != 0)
            glDeleteShader(compute_shader);
        throw;
    }

    glDeleteShader(compute_shader);

    if (m_id != 0)
        glDeleteProgram(m_id);

    m_id = new_program;
    m_uniform_cache.clear();
}
