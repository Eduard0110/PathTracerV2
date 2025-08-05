#pragma once

#include <glad/glad.h>

#include <unordered_map>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class BaseShaderProgram {
public:
    BaseShaderProgram() = default;
    ~BaseShaderProgram();

    BaseShaderProgram(const BaseShaderProgram&) = delete;
    BaseShaderProgram& operator=(const BaseShaderProgram&) = delete;
    BaseShaderProgram(BaseShaderProgram&&) noexcept;
    BaseShaderProgram& operator=(BaseShaderProgram&&) noexcept;

    void setUniform(const std::string& name, bool value) const;                                // 1 bool
    void setUniform(const std::string& name, int value) const;                                // 1 int
    void setUniform(const std::string& name, float value) const;                              // 1 float
    void setUniform(const std::string& name, int value1, int value2) const;                   // 2 ints
    void setUniform(const std::string& name, float value1, float value2) const;               // 2 float
    void setUniform(const std::string& name, int value1, int value2, int value3) const;       // 3 ints
    void setUniform(const std::string& name, float value1, float value2, float value3) const; // 3 floats
    void setUniform(const std::string& name, const std::array<int, 3>& values) const;         // 3 ints
    void setUniform(const std::string& name, const std::array<float, 3>& values) const;       // 3 floats


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
