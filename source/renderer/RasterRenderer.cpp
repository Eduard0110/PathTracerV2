#include "renderer/RasterRenderer.hpp"

#include <cmath>
#include <cstddef>
#include <iostream>
#include <stdexcept>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "tinyobjloader/tiny_obj_loader.h"
#include "core/Paths.hpp"

namespace {
    glm::mat4 createModelMatrix(const Transform& transform) {
        glm::mat4 model(1.0f);
        model = glm::translate(model, glm::vec3(
            transform.position.x,
            transform.position.y,
            transform.position.z
        ));

        // Preserve the existing Z * Y * X rotation order.
        model = glm::rotate(model, transform.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::rotate(model, transform.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, transform.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));

        return glm::scale(model, glm::vec3(transform.scale.x, transform.scale.y, transform.scale.z));
    }

    glm::mat4 createViewMatrix(const Camera& camera) {
        const auto position = camera.getPosition();
        const auto rotation = camera.getRotation();

        const float pitch = rotation[0];
        const float yaw = rotation[1];

        const glm::vec3 eye{ position[0], position[1], position[2] };
        const glm::vec3 forward{ std::sin(yaw) * std::cos(pitch), std::sin(pitch), std::cos(yaw) * std::cos(pitch) };

        // The path tracer uses +Z as forward and +X as screen-right. Standard
        // glm::lookAt is right-handed and would mirror the horizontal axis
        // when looking towards +Z, so construct the matching view basis here.
        const glm::vec3 right = glm::normalize(glm::cross(
            glm::vec3(0.0f, 1.0f, 0.0f),
            forward
        ));
        const glm::vec3 up = glm::cross(forward, right);

        glm::mat4 view(1.0f);

        view[0][0] = right.x;
        view[1][0] = right.y;
        view[2][0] = right.z;
        view[3][0] = -glm::dot(right, eye);

        view[0][1] = up.x;
        view[1][1] = up.y;
        view[2][1] = up.z;
        view[3][1] = -glm::dot(up, eye);

        view[0][2] = -forward.x;
        view[1][2] = -forward.y;
        view[2][2] = -forward.z;
        view[3][2] = glm::dot(forward, eye);

        return view;
    }

    void uploadMatrix4(GLuint program, const char* name, const glm::mat4& matrix) {
        const GLint location = glGetUniformLocation(program, name);

        if (location >= 0)
            glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void uploadMatrix3(GLuint program, const char* name, const glm::mat3& matrix) {
        const GLint location = glGetUniformLocation(program, name);

        if (location >= 0)
            glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
    }

    glm::vec3 getPosition(const tinyobj::attrib_t& attributes, const tinyobj::index_t& index) {
        return {
            attributes.vertices[3 * index.vertex_index + 0],
            attributes.vertices[3 * index.vertex_index + 1],
            attributes.vertices[3 * index.vertex_index + 2]
        };
    }
}

RasterRenderer::RasterRenderer(int width, int height)
    : m_width(width),
      m_height(height),
      m_colour_texture(width, height) {
}

RasterRenderer::~RasterRenderer() {
    destroyMeshes();
}

void RasterRenderer::init() {
    m_shader.createAndCompile(
        Paths::shader("RasterVertex.vs").string(),
        Paths::shader("RasterFragment.fs").string()
    );

    m_colour_texture.create();
    m_framebuffer.attachTexture(m_colour_texture.getID(), m_width, m_height, true);
}

void RasterRenderer::resize(int width, int height) {
    if (width <= 0 || height <= 0)
        return;

    if (width == m_width && height == m_height)
        return;

    m_width = width;
    m_height = height;

    m_colour_texture.create(m_width, m_height, nullptr);
    m_framebuffer.attachTexture(m_colour_texture.getID(), m_width, m_height, true);
}

bool RasterRenderer::hasMesh(uint32_t mesh_id) const {
    return m_meshes.find(mesh_id) != m_meshes.end();
}

void RasterRenderer::loadMesh(uint32_t mesh_id, const std::string& file_path) {
    tinyobj::attrib_t attributes;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string error; 

	const bool loaded = tinyobj::LoadObj( &attributes, &shapes, &materials, &error, file_path.c_str(), nullptr, true );

    if (!error.empty())
        std::cerr << "TinyObjLoader: " << error << '\n';

    if (!loaded)
        throw std::runtime_error("Failed to load raster mesh: " + file_path);

    std::size_t index_count = 0;

    for (const tinyobj::shape_t& shape : shapes)
        index_count += shape.mesh.indices.size();

    std::vector<RasterVertex> vertices;
    vertices.reserve(index_count);

    for (const tinyobj::shape_t& shape : shapes) {
        const std::vector<tinyobj::index_t>& indices = shape.mesh.indices;

        for (std::size_t i = 0; i + 2 < indices.size(); i += 3) {
            const tinyobj::index_t triangle_indices[3] = { indices[i], indices[i + 1], indices[i + 2] };

            const glm::vec3 positions[3] = {
                getPosition(attributes, triangle_indices[0]),
                getPosition(attributes, triangle_indices[1]),
                getPosition(attributes, triangle_indices[2])
            };

            const glm::vec3 face_normal = glm::normalize(glm::cross(
                positions[1] - positions[0],
                positions[2] - positions[0]
            ));

            for (int vertex = 0; vertex < 3; ++vertex) {
                const tinyobj::index_t& index = triangle_indices[vertex];
                glm::vec3 normal = face_normal;
                float u = 0.0f;
                float v = 0.0f;

                if (index.normal_index >= 0) {
                    normal = {
                        attributes.normals[3 * index.normal_index + 0],
                        attributes.normals[3 * index.normal_index + 1],
                        attributes.normals[3 * index.normal_index + 2]
                    };
                }

                if (index.texcoord_index >= 0) {
                    u = attributes.texcoords[2 * index.texcoord_index + 0];
                    v = attributes.texcoords[2 * index.texcoord_index + 1];
                }

                vertices.push_back({
                    { positions[vertex].x, positions[vertex].y, positions[vertex].z },
                    { normal.x, normal.y, normal.z },
                    { u, v }
                });
            }
        }
    }

    RasterMesh new_mesh;
    new_mesh.vertex_count = static_cast<GLsizei>(vertices.size());

    glCreateVertexArrays(1, &new_mesh.vao);
    glCreateBuffers(1, &new_mesh.vertex_buffer);

    glNamedBufferData(
        new_mesh.vertex_buffer,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(RasterVertex)),
        vertices.data(),
        GL_STATIC_DRAW
    );

    glVertexArrayVertexBuffer(
        new_mesh.vao,
        0,
        new_mesh.vertex_buffer,
        0,
        sizeof(RasterVertex)
    );

    glEnableVertexArrayAttrib(new_mesh.vao, 0);
    glVertexArrayAttribFormat(
        new_mesh.vao,
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        offsetof(RasterVertex, position)
    );
    glVertexArrayAttribBinding(new_mesh.vao, 0, 0);

    glEnableVertexArrayAttrib(new_mesh.vao, 1);
    glVertexArrayAttribFormat(
        new_mesh.vao,
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        offsetof(RasterVertex, normal)
    );
    glVertexArrayAttribBinding(new_mesh.vao, 1, 0);

    glEnableVertexArrayAttrib(new_mesh.vao, 2);
    glVertexArrayAttribFormat(
        new_mesh.vao,
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        offsetof(RasterVertex, uv)
    );
    glVertexArrayAttribBinding(new_mesh.vao, 2, 0);

    auto existing = m_meshes.find(mesh_id);

    if (existing != m_meshes.end()) {
        destroyMesh(existing->second);
        existing->second = new_mesh;
    }
    else {
        m_meshes.emplace(mesh_id, new_mesh);
    }
}

void RasterRenderer::render(const std::vector<SceneObject>& objects, const Camera& camera) {
    m_framebuffer.bind();

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.04f, 0.05f, 0.07f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shader.use();

    const glm::mat4 view = createViewMatrix(camera);
    const float aspect = static_cast<float>(m_width) / static_cast<float>(m_height);
    // The editor uses a fixed 36 mm full-frame sensor, leaving focal length as
    // the single user-facing field-of-view control.
    constexpr float sensor_width_mm = 36.0f;
    const float sensor_height = sensor_width_mm / aspect;
    const float vertical_fov = 2.0f * std::atan(
        sensor_height / (2.0f * camera.getFocalLength())
    );
    const glm::mat4 projection = glm::perspective(
        vertical_fov,
        aspect,
        0.01f,
        1000.0f
    );

    uploadMatrix4(m_shader.getID(), "view", view);
    uploadMatrix4(m_shader.getID(), "projection", projection);

    for (const SceneObject& object : objects) {
        if (!object.visible)
            continue;

        const auto mesh = m_meshes.find(object.mesh_id);

        if (mesh == m_meshes.end())
            continue;

        const glm::mat4 model = createModelMatrix(object.transform);
        const glm::mat3 normal_matrix = glm::transpose(
            glm::inverse(glm::mat3(model))
        );

        uploadMatrix4(m_shader.getID(), "model", model);
        uploadMatrix3(m_shader.getID(), "normalMatrix", normal_matrix);
        m_shader.setUniform("materialId", object.material_id);

        glBindVertexArray(mesh->second.vao);
        glDrawArrays(GL_TRIANGLES, 0, mesh->second.vertex_count);
    }

    glBindVertexArray(0);
    glDisable(GL_DEPTH_TEST);
    m_framebuffer.unbind();
}

void RasterRenderer::destroyMesh(RasterMesh& mesh) {
    if (mesh.vertex_buffer != 0)
        glDeleteBuffers(1, &mesh.vertex_buffer);

    if (mesh.vao != 0)
        glDeleteVertexArrays(1, &mesh.vao);

    mesh = {};
}

void RasterRenderer::destroyMeshes() {
    for (auto& entry : m_meshes)
        destroyMesh(entry.second);

    m_meshes.clear();
}
