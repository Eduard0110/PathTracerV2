#pragma once

#include <glad/glad.h>

#include <unordered_map>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <array>

class BaseShaderProgram {
public:
    BaseShaderProgram() = default;
    ~BaseShaderProgram();

    BaseShaderProgram(const BaseShaderProgram&) = delete;
    BaseShaderProgram& operator=(const BaseShaderProgram&) = delete;

    void setUniform(const std::string& name, int value) const;
    void setUniform(const std::string& name, float value) const;
    void setUniform(const std::string& name, GLuint value) const;

    void setUniform(const std::string& name, int value1, int value2) const;
    void setUniform(const std::string& name, float value1, float value2) const;

    void setUniform(const std::string& name, const std::array<float, 2>& v) const;
    void setUniform(const std::string& name, const std::array<float, 3>& v) const;
    void setUniform(const std::string& name, const std::array<float, 4>& v) const;


    void use() const { glUseProgram(m_id); }

    GLuint getID() const { return m_id; }

    GLuint compile(GLenum type, const std::string& source) const;

protected:
    GLuint m_id = 0;

    // cache of uniform locations
    mutable std::unordered_map<std::string, GLint> m_uniform_cache;

    // get location from cache
    GLint getUniformLocation(const std::string& name) const;

    // helper function to load the source file
    std::string loadShaderSource(const std::string& path);

    // checks for compile errors
    void checkCompileErrors(GLuint shader, const char* type) const;
};
