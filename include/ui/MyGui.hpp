#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "renderer/Renderer.hpp"
#include "scene/Scene.hpp"
#include "input/CameraController.hpp"

struct GuiActions {
    bool export_image = false;
    bool change_render_mode = false;
    bool resize_render_output = false;
    Renderer::RenderMode requested_render_mode = Renderer::RenderMode::RASTERIZED;
    int requested_render_width = 0;
    int requested_render_height = 0;
    std::vector<std::string> mesh_paths_to_load;
	std::vector<std::string> skybox_paths_to_load;
};

struct EditorState {
    uint32_t selected_object_id = UINT32_MAX;
    uint32_t selected_material_id = UINT32_MAX;

    bool show_scene_hierarchy = true;
    bool show_inspector = true;
    bool show_materials = true;
    bool show_renderer_settings = true;
    bool show_post_process_settings = true;
    bool show_environment_settings = true;
    bool show_bvh_debug = false;
    bool show_camera_settings = true;
    bool show_assets = true;

    uint32_t new_object_mesh_id = UINT32_MAX;
    uint32_t new_object_material_id = UINT32_MAX;
    char new_object_name[128] = "New Object";
    Transform new_object_transform{};

    int render_width_input = 0;
    int render_height_input = 0;
};

class MyGui {
public:
    MyGui() = default;
    ~MyGui() = default;

    void initImGui(GLFWwindow* window);
    void newFrameImGui();
    void render() const;
	void setStatusMessage(std::string message, bool is_error = false);
	void refreshMeshFolder() { m_mesh_folder_scanned = false; }
	void refreshSkyboxFolder() { m_skybox_folder_scanned = false; }

    GuiActions draw(
        Scene& scene,
        Renderer& renderer,
        CameraController& camera_controller
    );

private:
    void drawDockspace();
    void drawMainMenu(GuiActions& actions);

    void drawSceneHierarchy(Scene& scene, GuiActions& actions);
    void drawViewport(const Scene& scene, Renderer& renderer);
    void drawInspector(Scene& scene, GuiActions& actions);
    void drawMaterials(Scene& scene, Renderer& renderer);
    void drawRendererSettings(Renderer& renderer, GuiActions& actions);
    void drawCameraSettings(Scene& scene, Renderer& renderer, CameraController& camera_controller);
    void drawPostProcessSettings(Renderer& renderer);
    void drawEnvironmentSettings(Renderer& renderer);
    void drawBvhDebug(const Scene& scene);
	void drawAssets(const Scene& scene, const Renderer& renderer, GuiActions& actions);
	void drawCreateObjectPopup(Scene& scene, GuiActions& actions);
	void scanMeshFolder();
	void scanSkyboxFolder();

private:
    EditorState m_state;
    bool m_reset_layout = false;
	bool m_mesh_folder_scanned = false;
	bool m_skybox_folder_scanned = false;
	bool m_status_is_error = false;
	std::string m_status_message;
	std::vector<std::string> m_available_mesh_files;
	std::vector<std::string> m_available_skybox_files;
};
