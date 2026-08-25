#pragma once

#include "renderer/BaseShaderProgram.hpp"


class ShaderProgram : public BaseShaderProgram {
public:
    void createAndCompile(const std::string& vertex_path, const std::string& fragment_path);
};
