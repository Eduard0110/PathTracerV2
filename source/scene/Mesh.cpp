#include "scene/Mesh.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader/tiny_obj_loader.h"

#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace
{
    Vec4 getPosition(
        const tinyobj::attrib_t& attrib,
        const tinyobj::index_t& index)
    {
        return {
            attrib.vertices[3 * index.vertex_index + 0],
            attrib.vertices[3 * index.vertex_index + 1],
            attrib.vertices[3 * index.vertex_index + 2],
            1.0f
        };
    }

    Vec4 getNormal(
        const tinyobj::attrib_t& attrib,
        const tinyobj::index_t& index)
    {
        if (index.normal_index < 0)
            return { 0.0f, 0.0f, 0.0f, 0.0f };

        return {
            attrib.normals[3 * index.normal_index + 0],
            attrib.normals[3 * index.normal_index + 1],
            attrib.normals[3 * index.normal_index + 2],
            0.0f
        };
    }

    Vec4 getUV(
        const tinyobj::attrib_t& attrib,
        const tinyobj::index_t& index)
    {
        if (index.texcoord_index < 0)
            return { 0.0f, 0.0f, 0.0f, 0.0f };

        return {
            attrib.texcoords[2 * index.texcoord_index + 0],
            attrib.texcoords[2 * index.texcoord_index + 1],
            0.0f,
            0.0f
        };
    }

    Triangle createTriangle(
        const tinyobj::attrib_t& attrib,
        const tinyobj::index_t& i0,
        const tinyobj::index_t& i1,
        const tinyobj::index_t& i2,
        uint32_t id)
    {
        Triangle triangle{};

        triangle.v0 = getPosition(attrib, i0);
        triangle.v1 = getPosition(attrib, i1);
        triangle.v2 = getPosition(attrib, i2);

        triangle.n0 = getNormal(attrib, i0);
        triangle.n1 = getNormal(attrib, i1);
        triangle.n2 = getNormal(attrib, i2);

        triangle.uv0 = getUV(attrib, i0);
        triangle.uv1 = getUV(attrib, i1);
        triangle.uv2 = getUV(attrib, i2);

        triangle.mesh_id = id;

        return triangle;
    }
}

Mesh::Mesh(const std::string& file_path, uint32_t id)
    : m_id(id),
      m_name(std::filesystem::path(file_path).stem().string()),
      m_file_path(file_path)
{
    loadOBJ(file_path, id);
}

void Mesh::loadOBJ(const std::string& file_path, uint32_t id) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string error;

    const bool success = tinyobj::LoadObj(
        &attrib,
        &shapes,
        &materials,
        &error,
        file_path.c_str(),
        nullptr,
        true
    );

    if (!error.empty())
        std::cerr << "TinyObjLoader: " << error << '\n';

    if (!success)
        throw std::runtime_error(
            "Failed to load OBJ: " + file_path
        );

    for (const auto& shape : shapes)
    {
        const auto& indices = shape.mesh.indices;

        for (size_t i = 0; i < indices.size(); i += 3)
        {
            m_triangles.push_back(
                createTriangle(
                    attrib,
                    indices[i],
                    indices[i + 1],
                    indices[i + 2],
                    id
                )
            );
        }
    }

    std::cout
        << "Loaded "
        << m_triangles.size()
        << " triangles from "
        << file_path
        << '\n';
}
