#include "renderer/BaseShaderProgram.hpp"

#include <algorithm>
#include <stdexcept>
#include <vector>

BaseShaderProgram::~BaseShaderProgram() {
    if (m_id) glDeleteProgram(m_id);
}

std::string BaseShaderProgram::loadShaderSource(const std::string& path) {
    std::ifstream file(path);
    std::stringstream buffer;

    if (!file.is_open())
        throw std::runtime_error("Shader file could not be opened: " + path);

    buffer << file.rdbuf();
    return buffer.str();
}

void BaseShaderProgram::checkCompileErrors(GLuint shader, const char* type) const {
    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint log_length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<char> log(static_cast<std::size_t>(std::max(log_length, 1)));
        glGetShaderInfoLog(shader, log_length, nullptr, log.data());
        throw std::runtime_error(
            std::string(type) + " compilation failed:\n" + log.data()
        );
    }
}

GLuint BaseShaderProgram::compile(GLenum type, const std::string& source) const {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    const char* type_str = "UNKNOWN_SHADER";
    switch (type) {
    case GL_VERTEX_SHADER:   type_str = "VERTEX_SHADER"; break;
    case GL_FRAGMENT_SHADER: type_str = "FRAGMENT_SHADER"; break;
    case GL_COMPUTE_SHADER:  type_str = "COMPUTE_SHADER"; break;
    }

    try {
        checkCompileErrors(shader, type_str);
    }
    catch (...) {
        glDeleteShader(shader);
        throw;
    }

    return shader;
}

// uniforms

GLint BaseShaderProgram::getUniformLocation(const std::string& name) const {
    // cache loop up
    auto it = m_uniform_cache.find(name);
    if (it != m_uniform_cache.end())
        return it->second;
    // if not found, get the uniform location and load it into the cache
    GLint location = glGetUniformLocation(m_id, name.c_str());
    if (location == -1) {
        std::cerr << "Warning: Uniform '" << name << "' doesn't exist or isn't used.\n";
    }

    m_uniform_cache[name] = location;
    return location;
}

void BaseShaderProgram::setUniform(const std::string& name, int value) const {
    glUniform1i(getUniformLocation(name), value);
}

void BaseShaderProgram::setUniform(const std::string& name, float value) const {
    glUniform1f(getUniformLocation(name), value);
}

void BaseShaderProgram::setUniform(const std::string& name, GLuint value) const {
    glUniform1ui(getUniformLocation(name), value);
}

void BaseShaderProgram::setUniform(const std::string& name, int value1, int value2) const {
    glUniform2i(getUniformLocation(name), value1, value2);
}

void BaseShaderProgram::setUniform(const std::string& name, float value1, float value2) const {
    glUniform2f(getUniformLocation(name), value1, value2);
}

void BaseShaderProgram::setUniform(const std::string& name, const std::array<float, 2>& v) const {
    glUniform2fv(getUniformLocation(name), 1, v.data());
}

void BaseShaderProgram::setUniform(const std::string& name, const std::array<float, 3>& v) const {
    glUniform3fv(getUniformLocation(name), 1, v.data());
}

void BaseShaderProgram::setUniform(const std::string& name, const std::array<float, 4>& v) const {
    glUniform4fv(getUniformLocation(name), 1, v.data());
}
