#pragma once

#include "renderer/BaseShaderProgram.hpp"

class ComputeProgram : public BaseShaderProgram {
public:
    void createAndCompile(const std::string& compute_shader_path);
};
