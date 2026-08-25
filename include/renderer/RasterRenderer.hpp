#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <glad/glad.h>

#include "camera/Camera.hpp"
#include "renderer/Framebuffer.hpp"
#include "renderer/ShaderProgram.hpp"
#include "renderer/Texture.hpp"
#include "scene/Geometry.hpp"

class RasterRenderer {
public:
    RasterRenderer(int width, int height);
    ~RasterRenderer();

    RasterRenderer(const RasterRenderer&) = delete;
    RasterRenderer& operator=(const RasterRenderer&) = delete;

    void init();
    void resize(int width, int height);

    // Loads one OBJ into a local-space VBO and associates it with mesh_id.
    // Loading an existing ID replaces its previous raster mesh.
    void loadMesh(uint32_t mesh_id, const std::string& file_path);
    bool hasMesh(uint32_t mesh_id) const;

    // The material SSBO is expected to be bound separately at the binding
    // used by the raster fragment shader. Each object supplies material_id.
    void render(
        const std::vector<SceneObject>& objects,
        const Camera& camera
    );

    GLuint getTextureID() const { return m_colour_texture.getID(); }

private:
    struct RasterVertex {
        float position[3];
        float normal[3];
        float uv[2];
    };

    struct RasterMesh {
        GLuint vao = 0;
        GLuint vertex_buffer = 0;
        GLsizei vertex_count = 0;
    };

    void destroyMesh(RasterMesh& mesh);
    void destroyMeshes();

private:
    int m_width;
    int m_height;

    ShaderProgram m_shader;
    Texture m_colour_texture;
    Framebuffer m_framebuffer;

    std::unordered_map<uint32_t, RasterMesh> m_meshes;
};
