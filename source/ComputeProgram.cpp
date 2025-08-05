#include "ComputeProgram.hpp"

void ComputeProgram::createAndCompile(const std::string& compute_shader_path) {
    // read the source file
    std::string source = loadShaderSource(compute_shader_path);

    // create and compile shader
    GLuint compute_shader = compile(GL_COMPUTE_SHADER, source);

    // create program and link
    m_id = glCreateProgram();
    glAttachShader(m_id, compute_shader);
    glLinkProgram(m_id);

    // check for linking errors
    GLint success;
    glGetProgramiv(m_id, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_id, 512, nullptr, infoLog);
        std::cerr << "ERROR::COMPUTE_PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // clean up
    glDeleteShader(compute_shader);
}