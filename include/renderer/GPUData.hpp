#pragma once

#include "core/Vectors.hpp"

struct alignas(16) GPUMaterial {
    Vec4 baseColour;
    Vec4 emission;  // 4th component is emission strength
    Vec4 absorption;  // 4th component is absorption strength

    float roughness;
    float metallic;
    float ior;
    bool isTransparent;

};

static_assert(alignof(GPUMaterial) == 16);
static_assert(sizeof(GPUMaterial) == 64);