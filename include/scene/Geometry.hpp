#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>

#include "core/Vectors.hpp"

// using Vec4 for OpenGL compatibility
// the 'w' (4th) component is not used for geometry
struct alignas(16) Triangle {
    Vec4 v0, v1, v2;
    Vec4 n0, n1, n2;
    Vec4 uv0, uv1, uv2;

    uint32_t material_id;
    uint32_t mesh_id;
    uint32_t object_id;

    // preserve predictable GPU alignment
    uint32_t padding;
};

struct alignas(16) AABB {
    Vec4 min, max;

    AABB()
        : min(
            std::numeric_limits<float>::infinity(),
            std::numeric_limits<float>::infinity(),
            std::numeric_limits<float>::infinity(),
            0.0f),
        max(
            -std::numeric_limits<float>::infinity(),
            -std::numeric_limits<float>::infinity(),
            -std::numeric_limits<float>::infinity(),
            0.0f)
    {}
};

struct alignas(16) BVHNode {
    AABB bounding_box;

    int left = -1;
    int right = -1;

    int start = 0;
    int count = 0;
};

struct Transform {
    Vec3 position{};
    Vec3 rotation{};
    Vec3 scale{ 1, 1, 1 };
};

struct Material {
    std::string name;

    uint32_t id;

    Vec3 baseColour{ 0.8f, 0.8f, 0.8f };
    Vec3 emission{};
    Vec3 absorption{};

    bool isTransparent = false;
    float absorptionStrength = 0.0f;
    float roughness = 0.5f;
    float metallic = 0.0f;
    float emissionStrength = 0.0f;
    float ior = 1.5f;
};

struct SceneObject {
    std::string name;

    uint32_t object_id;
    uint32_t mesh_id;
    uint32_t material_id;

    Transform transform{};

    bool visible = true;
};

// these layouts must match the corresponding std430 GLSL structures
static_assert(sizeof(Vec4) == 16);

static_assert(alignof(Triangle) == 16);
static_assert(sizeof(Triangle) == 160);
static_assert(offsetof(Triangle, material_id) == 144);

static_assert(alignof(AABB) == 16);
static_assert(sizeof(AABB) == 32);

static_assert(alignof(BVHNode) == 16);
static_assert(sizeof(BVHNode) == 48);
static_assert(offsetof(BVHNode, left) == 32);
