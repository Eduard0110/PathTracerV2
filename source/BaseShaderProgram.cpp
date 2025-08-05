#include "BaseShaderProgram.hpp"

BaseShaderProgram::~BaseShaderProgram() {
    if (m_id) glDeleteProgram(m_id);
}

std::string BaseShaderProgram::loadShaderSource(const std::string& path) {
    std::ifstream file(path);
    std::stringstream buffer;

    if (!file.is_open()) {
        std::cerr << "ERROR::FILE_NOT_FOUND: " << path << std::endl;
        return "";
    }

    buffer << file.rdbuf();
    return buffer.str();
}

void BaseShaderProgram::checkCompileErrors(GLuint shader, const char* type) const {
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::" << type << "::COMPILATION_FAILED\n" << infoLog << std::endl;
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

    checkCompileErrors(shader, type_str);

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

void BaseShaderProgram::setUniform(const std::string& name, int value1, int value2) const {
    glUniform2i(getUniformLocation(name), value1, value2);
}

void BaseShaderProgram::setUniform(const std::string& name, float value1, float value2) const {
    glUniform2f(getUniformLocation(name), value1, value2);
}