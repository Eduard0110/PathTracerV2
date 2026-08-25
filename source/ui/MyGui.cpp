#include "ui/MyGui.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <string>

#include "imgui/imgui_internal.h"
#include "core/Paths.hpp"

namespace {
    constexpr float radians_to_degrees = 57.2957795131f;

    void requestRasterPreview(GuiActions& actions) {
        actions.change_render_mode = true;
        actions.requested_render_mode = Renderer::RenderMode::RASTERIZED;
    }

    std::string normalisedPath(const std::string& path) {
        std::error_code error;
        std::filesystem::path normalised = std::filesystem::weakly_canonical(path, error);

        if (error)
            normalised = std::filesystem::path(path).lexically_normal();

        std::string result = normalised.generic_string();
        std::transform(result.begin(), result.end(), result.begin(),
            [](unsigned char character) {
                return static_cast<char>(std::tolower(character));
            });
        return result;
    }

    bool isSupportedSkyboxExtension(std::string extension) {
        std::transform(extension.begin(), extension.end(), extension.begin(),
            [](unsigned char character) {
                return static_cast<char>(std::tolower(character));
            });

        return extension == ".hdr" || extension == ".png" ||
            extension == ".jpg" || extension == ".jpeg";
    }
}

void MyGui::initImGui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // Keep the docking layout beside the executable instead of depending on
    // whichever working directory happened to launch the application.
    static const std::string imgui_settings_path = Paths::imguiSettings().string();
    io.IniFilename = imgui_settings_path.c_str();

    // use windows specific font instead
    // of ImGui default font which looks blurry when enlarged
    ImFont* uiFont = io.Fonts->AddFontFromFileTTF(
        "C:/Windows/Fonts/segoeui.ttf",
        17.0f
    );

    // Keep the application usable if the system font is unavailable.
    if (uiFont == nullptr) {
        ImFontConfig fallbackConfig;
        fallbackConfig.SizePixels = 16.0f;
        uiFont = io.Fonts->AddFontDefault(&fallbackConfig);
    }

    io.FontDefault = uiFont;

    ImGui::StyleColorsDark();

    // Detached editor panels should visually match panels in the main window.
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(10.0f, 9.0f);
    style.FramePadding = ImVec2(7.0f, 5.0f);
    style.ItemSpacing = ImVec2(8.0f, 7.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 5.0f);

    // Increase separation between text and controls without making the theme
    // overly bright or distracting from the rendered image.
    style.Colors[ImGuiCol_Text]             = ImVec4(0.94f, 0.96f, 0.99f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled]     = ImVec4(0.60f, 0.64f, 0.71f, 1.00f);
    style.Colors[ImGuiCol_WindowBg]         = ImVec4(0.065f, 0.075f, 0.095f, 1.00f);
    style.Colors[ImGuiCol_ChildBg]          = ImVec4(0.075f, 0.085f, 0.110f, 1.00f);
    style.Colors[ImGuiCol_FrameBg]          = ImVec4(0.14f, 0.16f, 0.21f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered]   = ImVec4(0.22f, 0.26f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_FrameBgActive]    = ImVec4(0.27f, 0.32f, 0.43f, 1.00f);
    style.Colors[ImGuiCol_Header]           = ImVec4(0.17f, 0.22f, 0.31f, 1.00f);
    style.Colors[ImGuiCol_HeaderHovered]    = ImVec4(0.25f, 0.32f, 0.45f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive]     = ImVec4(0.31f, 0.40f, 0.56f, 1.00f);
    style.Colors[ImGuiCol_Separator]        = ImVec4(0.32f, 0.36f, 0.44f, 1.00f);
    style.Colors[ImGuiCol_Border]           = ImVec4(0.25f, 0.29f, 0.36f, 1.00f);

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");
}

void MyGui::newFrameImGui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

GuiActions MyGui::draw(Scene& scene, Renderer& renderer, CameraController& camera_controller) {
    GuiActions actions;

    drawMainMenu(actions);
    drawDockspace();

    drawSceneHierarchy(scene, actions);
    drawViewport(scene, renderer);
    drawInspector(scene, actions);
    drawMaterials(scene, renderer);
    drawRendererSettings(renderer, actions);
    drawCameraSettings(scene, renderer, camera_controller);
    drawPostProcessSettings(renderer);
    drawEnvironmentSettings(renderer);
    drawBvhDebug(scene);
    drawAssets(scene, renderer, actions);

    return actions;
}

void MyGui::setStatusMessage(std::string message, bool is_error) {
    m_status_message = std::move(message);
    m_status_is_error = is_error;
}

void MyGui::drawMainMenu(GuiActions& actions) {
    if (!ImGui::BeginMainMenuBar())
        return;

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Export current image"))
            actions.export_image = true;

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View")) {
        ImGui::MenuItem("Scene Hierarchy", nullptr, &m_state.show_scene_hierarchy);
        ImGui::MenuItem("Inspector", nullptr, &m_state.show_inspector);
        ImGui::MenuItem("Materials", nullptr, &m_state.show_materials);
        ImGui::MenuItem("Renderer", nullptr, &m_state.show_renderer_settings);
        ImGui::MenuItem("Camera", nullptr, &m_state.show_camera_settings);
        ImGui::MenuItem("Post Process", nullptr, &m_state.show_post_process_settings);
        ImGui::MenuItem("Environment", nullptr, &m_state.show_environment_settings);
        ImGui::MenuItem("BVH Debug", nullptr, &m_state.show_bvh_debug);
		ImGui::MenuItem("Assets", nullptr, &m_state.show_assets);
        ImGui::Separator();

        if (ImGui::MenuItem("Reset default layout"))
            m_reset_layout = true;

        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}

void MyGui::drawDockspace() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    // Increment this ID when the shipped default layout changes. Otherwise an
    // older imgui.ini can leave newly introduced windows floating elsewhere.
    const ImGuiID dockspaceId = ImGui::GetID("PathTracerDockspaceV2");

    // build a useful layout only on first launch or after an explicit reset
    // afterwards ImGui restores the user's arrangement from imgui.ini
    if (m_reset_layout || ImGui::DockBuilderGetNode(dockspaceId) == nullptr) {
        m_reset_layout = false;

        ImGui::DockBuilderRemoveNode(dockspaceId);
        ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodePos(dockspaceId, viewport->WorkPos);
        ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->WorkSize);

        ImGuiID centre = dockspaceId;
        ImGuiID left = ImGui::DockBuilderSplitNode(centre, ImGuiDir_Left, 0.19f, nullptr, &centre);
        ImGuiID right = ImGui::DockBuilderSplitNode(centre, ImGuiDir_Right, 0.25f, nullptr, &centre);
        ImGuiID bottom = ImGui::DockBuilderSplitNode(centre, ImGuiDir_Down, 0.27f, nullptr, &centre);

        ImGui::DockBuilderDockWindow("Scene Hierarchy", left);
		ImGui::DockBuilderDockWindow("Assets", left);
        ImGui::DockBuilderDockWindow("Inspector", right);
        ImGui::DockBuilderDockWindow("Camera", right);
        ImGui::DockBuilderDockWindow("Viewport", centre);

        // These windows share the bottom region as selectable tabs.
        ImGui::DockBuilderDockWindow("Materials", bottom);
        ImGui::DockBuilderDockWindow("Renderer", bottom);
        ImGui::DockBuilderDockWindow("Post Process", bottom);
        ImGui::DockBuilderDockWindow("Environment", bottom);
        ImGui::DockBuilderDockWindow("BVH Debug", bottom);

        ImGui::DockBuilderFinish(dockspaceId);
    }

    ImGui::DockSpaceOverViewport(
        dockspaceId,
        viewport,
        ImGuiDockNodeFlags_PassthruCentralNode
    );
}

void MyGui::drawSceneHierarchy(Scene& scene, GuiActions& actions) {
    if (!m_state.show_scene_hierarchy)
        return;

    if (!ImGui::Begin("Scene Hierarchy", &m_state.show_scene_hierarchy)) {
        ImGui::End();
        return;
    }

    const bool canCreateObject = !scene.getMeshes().empty() && !scene.getMaterials().empty();

    if (!canCreateObject)
        ImGui::BeginDisabled();

    if (ImGui::Button("Create Object", ImVec2(-1.0f, 0.0f))) {
        if (scene.getMesh(m_state.new_object_mesh_id) == nullptr)
            m_state.new_object_mesh_id = scene.getMeshes().front().getId();

        if (scene.getMaterial(m_state.new_object_material_id) == nullptr)
            m_state.new_object_material_id = scene.getMaterials().front().id;

        ImGui::OpenPopup("Create Object");
    }

    if (!canCreateObject)
        ImGui::EndDisabled();

    drawCreateObjectPopup(scene, actions);

    ImGui::TextDisabled("%zu object(s)", scene.getObjects().size());
    ImGui::Separator();

    for (const SceneObject& object : scene.getObjects()) {
        const bool selected = object.object_id == m_state.selected_object_id;
        const std::string label = object.name + "##object_" +
            std::to_string(object.object_id);

        if (!object.visible)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));

        if (ImGui::Selectable(label.c_str(), selected)) {
            m_state.selected_object_id = object.object_id;
            m_state.selected_material_id = object.material_id;
        }

        if (!object.visible)
            ImGui::PopStyleColor();
    }

    ImGui::End();
}

void MyGui::drawViewport(const Scene& scene, Renderer& renderer) {
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

    if (!ImGui::Begin("Viewport", nullptr, flags)) {
        ImGui::End();
        return;
    }

    const char* mode_name = renderer.getRenderMode() == Renderer::RenderMode::RASTERIZED ? "Rasterized preview" : "Path traced";

    ImGui::TextDisabled(
        "%s  |  %d x %d  |  Loaded: %zu  |  Rendered: %zu triangles",
        mode_name,
        renderer.getRenderWidth(),
        renderer.getRenderHeight(),
        scene.getLoadedTriangleCount(),
        scene.getRenderedTriangleCount()
    );

    const ImVec2 available = ImGui::GetContentRegionAvail();

    if (available.x > 0.0f && available.y > 0.0f) {
        const float texture_aspect = static_cast<float>(renderer.getRenderWidth()) /
            static_cast<float>(renderer.getRenderHeight());
        const float available_aspect = available.x / available.y;

        ImVec2 image_size = available;
        if (available_aspect > texture_aspect)
            image_size.x = image_size.y * texture_aspect;
        else
            image_size.y = image_size.x / texture_aspect;

        const ImVec2 cursor = ImGui::GetCursorPos();
        ImGui::SetCursorPos(ImVec2(
            cursor.x + (available.x - image_size.x) * 0.5f,
            cursor.y + (available.y - image_size.y) * 0.5f
        ));

        ImGui::Image(
            reinterpret_cast<void*>(
                static_cast<intptr_t>(renderer.getDisplayTextureID())
            ),
            image_size,
            ImVec2(0.0f, 1.0f),
            ImVec2(1.0f, 0.0f)
        );
    }

    ImGui::End();
}

void MyGui::drawInspector(Scene& scene, GuiActions& actions) {
    if (!m_state.show_inspector)
        return;

    if (!ImGui::Begin("Inspector", &m_state.show_inspector)) {
        ImGui::End();
        return;
    }

    const SceneObject* selected = scene.getObject(m_state.selected_object_id);
    if (selected == nullptr) {
        ImGui::TextDisabled("Select an object from the hierarchy.");
        ImGui::End();
        return;
    }

    const uint32_t objectId = selected->object_id;
    ImGui::TextUnformatted(selected->name.c_str());
    ImGui::TextDisabled("Object %u  |  Mesh %u", selected->object_id, selected->mesh_id);
    ImGui::Separator();

    bool visible = selected->visible;
    if (ImGui::Checkbox("Visible", &visible)) {
        scene.setObjectVisibility(objectId, visible);
        requestRasterPreview(actions);
    }

    const Material* current_material = scene.getMaterial(selected->material_id);
    const char* material_preview = current_material != nullptr ? current_material->name.c_str() : "Missing material";

    if (ImGui::BeginCombo("Material", material_preview)) {
        for (const Material& material : scene.getMaterials()) {
            const bool material_selected = material.id == selected->material_id;

            if (ImGui::Selectable(material.name.c_str(), material_selected)) {
                scene.setObjectMaterial(objectId, material.id);
                m_state.selected_material_id = material.id;
                requestRasterPreview(actions);
            }

            if (material_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

    ImGui::SeparatorText("Transform");

    Transform transform = selected->transform;
    float position[3] = {
        transform.position.x,
        transform.position.y,
        transform.position.z
    };
    float rotation_degrees[3] = {
        transform.rotation.x * radians_to_degrees,
        transform.rotation.y * radians_to_degrees,
        transform.rotation.z * radians_to_degrees
    };
    float scale[3] = {
        transform.scale.x,
        transform.scale.y,
        transform.scale.z
    };

    bool transform_changed = false;

    if (ImGui::DragFloat3("Position", position, 0.02f)) {
        transform.position = { position[0], position[1], position[2] };
        transform_changed = true;
    }

    if (ImGui::DragFloat3("Rotation", rotation_degrees, 0.25f)) {
        constexpr float degrees_to_radians = 1.0f / radians_to_degrees;
        transform.rotation = {
            rotation_degrees[0] * degrees_to_radians,
            rotation_degrees[1] * degrees_to_radians,
            rotation_degrees[2] * degrees_to_radians
        };
        transform_changed = true;
    }

    if (ImGui::DragFloat3("Scale", scale, 0.01f)) {
        // Zero scale creates a singular normal matrix, so keep it positive.
        transform.scale = {
            std::max(scale[0], 0.001f),
            std::max(scale[1], 0.001f),
            std::max(scale[2], 0.001f)
        };
        transform_changed = true;
    }

    if (transform_changed) {
        scene.setObjectTransform(objectId, transform);
        requestRasterPreview(actions);
    }

    ImGui::Spacing();
    ImGui::TextWrapped(
        "Object edits switch to rasterized preview. Enter Path Traced mode "
        "when you are ready to rebuild and upload the BVH."
    );

    ImGui::End();
}

void MyGui::drawMaterials(Scene& scene, Renderer& renderer) {
    if (!m_state.show_materials)
        return;

    if (!ImGui::Begin("Materials", &m_state.show_materials)) {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Add Material")) {
        Material material;
        material.name = "Material " + std::to_string(scene.getMaterials().size());
        m_state.selected_material_id = scene.createMaterial(material);
        renderer.uploadMaterialsToGPU(scene);
    }

    ImGui::Separator();

    ImGui::BeginChild("MaterialList", ImVec2(170.0f, 0.0f), ImGuiChildFlags_Borders);
    for (const Material& material : scene.getMaterials()) {
        const bool selected = material.id == m_state.selected_material_id;
        const std::string label = material.name + "##material_" +
            std::to_string(material.id);

        if (ImGui::Selectable(label.c_str(), selected))
            m_state.selected_material_id = material.id;
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("MaterialProperties", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders);

    const Material* selected = scene.getMaterial(m_state.selected_material_id);
    if (selected == nullptr) {
        ImGui::TextDisabled("Select a material to edit it.");
        ImGui::EndChild();
        ImGui::End();
        return;
    }

    Material edited = *selected;
    bool changed = false;

    char name[128]{};
    std::snprintf(name, sizeof(name), "%s", edited.name.c_str());
    if (ImGui::InputText("Name", name, sizeof(name))) {
        edited.name = name;
        changed = true;
    }

    changed |= ImGui::Checkbox("Transparency", &edited.isTransparent);
    changed |= ImGui::ColorEdit3("Base Colour", &edited.baseColour.x);
    changed |= ImGui::ColorEdit3("Emission Colour", &edited.emission.x);
    changed |= ImGui::ColorEdit3("Absorption Colour", &edited.absorption.x);
    changed |= ImGui::DragFloat("Emission Strength", &edited.emissionStrength, 0.1f, 0.0f, 100000.0f);
    changed |= ImGui::DragFloat("Absorption Strength", &edited.absorptionStrength, 0.1f, 0.0f, 100000.0f);
    changed |= ImGui::SliderFloat("Roughness", &edited.roughness, 0.001f, 1.0f);
    changed |= ImGui::SliderFloat("Metallic", &edited.metallic, 0.0f, 1.0f);
    changed |= ImGui::DragFloat("IOR", &edited.ior, 0.01f, 1.0f, 5.0f);

    ImGui::TextDisabled("Material ID: %u", edited.id);

    if (changed && scene.setMaterial(edited.id, edited))
        renderer.uploadMaterialsToGPU(scene);

    ImGui::EndChild();
    ImGui::End();
}

void MyGui::drawRendererSettings(Renderer& renderer, GuiActions& actions) {
    if (!m_state.show_renderer_settings)
        return;

    if (!ImGui::Begin("Renderer", &m_state.show_renderer_settings)) {
        ImGui::End();
        return;
    }

    const char* modes[] = { "Rasterized", "Path Traced" };
    int mode = static_cast<int>(renderer.getRenderMode());

    if (ImGui::Combo("Render Mode", &mode, modes, IM_ARRAYSIZE(modes))) {
        actions.change_render_mode = true;
        actions.requested_render_mode = static_cast<Renderer::RenderMode>(mode);
    }

    ImGui::SeparatorText("Output Resolution");

    if (m_state.render_width_input <= 0 || m_state.render_height_input <= 0) {
        m_state.render_width_input = renderer.getRenderWidth();
        m_state.render_height_input = renderer.getRenderHeight();
    }

    ImGui::SetNextItemWidth(130.0f);
    ImGui::InputInt("Width", &m_state.render_width_input, 16, 128);
    ImGui::SetNextItemWidth(130.0f);
    ImGui::InputInt("Height", &m_state.render_height_input, 16, 128);

    m_state.render_width_input = std::max(m_state.render_width_input, 1);
    m_state.render_height_input = std::max(m_state.render_height_input, 1);

    if (ImGui::Button("1280 x 720")) {
        m_state.render_width_input = 1280;
        m_state.render_height_input = 720;
    }
    ImGui::SameLine();
    if (ImGui::Button("1920 x 1080")) {
        m_state.render_width_input = 1920;
        m_state.render_height_input = 1080;
    }
    ImGui::SameLine();
    if (ImGui::Button("3840 x 2160")) {
        m_state.render_width_input = 3840;
        m_state.render_height_input = 2160;
    }

    const bool resolution_changed =
        m_state.render_width_input != renderer.getRenderWidth() ||
        m_state.render_height_input != renderer.getRenderHeight();
    const bool resolution_supported =
        m_state.render_width_input <= renderer.getMaxRenderDimension() &&
        m_state.render_height_input <= renderer.getMaxRenderDimension();

    if (!resolution_changed || !resolution_supported)
        ImGui::BeginDisabled();

    if (ImGui::Button("Apply Resolution")) {
        actions.resize_render_output = true;
        actions.requested_render_width = m_state.render_width_input;
        actions.requested_render_height = m_state.render_height_input;
    }

    if (!resolution_changed || !resolution_supported)
        ImGui::EndDisabled();

    if (!resolution_supported) {
        ImGui::TextColored(
            ImVec4(1.0f, 0.42f, 0.42f, 1.0f),
            "Maximum supported texture dimension: %d",
            renderer.getMaxRenderDimension()
        );
    }

    int reflections = renderer.getMaxReflections();
    if (ImGui::SliderInt("Maximum Bounces", &reflections, 1, 50))
        renderer.setMaxReflections(reflections);

    int samples = renderer.getSamplesPerPixel();
    if (ImGui::SliderInt("Samples Per Pixel", &samples, 1, 64))
        renderer.setSamplesPerPixel(samples);

    bool russian_roulette = renderer.getRussianRoulette();
    if (ImGui::Checkbox("Russian Roulette", &russian_roulette))
        renderer.setRussianRoulette(russian_roulette);

    ImGui::Spacing();
    ImGui::TextWrapped(
        "Enabling Path Traced mode rebuilds the BVH\n"
		"Rasterised mode is intended for scene editing."
    );

    ImGui::End();
}

void MyGui::drawCameraSettings(Scene& scene, Renderer& renderer, CameraController& camera_controller) {
    if (!m_state.show_camera_settings)
        return;

    if (!ImGui::Begin("Camera", &m_state.show_camera_settings)) {
        ImGui::End();
        return;
    }

    const char* modes[] = { "Orbit", "Free Fly" };
    int mode = static_cast<int>(camera_controller.getCameraMode());
    if (ImGui::Combo("Control Mode", &mode, modes, IM_ARRAYSIZE(modes)))
        camera_controller.setCameraMode(static_cast<CameraController::CameraMode>(mode));

    const char* states[] = { "Idle", "Moving" };
    int state = static_cast<int>(camera_controller.getCameraState());
    if (ImGui::Combo("Input State", &state, states, IM_ARRAYSIZE(states)))
        camera_controller.setCameraState(static_cast<CameraController::CameraState>(state));

    Camera& camera = scene.getCamera();
    const auto position = camera.getPosition();
    const auto rotation = camera.getRotation();

    ImGui::SeparatorText("Current Camera");
    ImGui::Text("Position  %.3f, %.3f, %.3f", position[0], position[1], position[2]);
    ImGui::Text("Rotation  %.1f, %.1f, %.1f deg",
        rotation[0] * radians_to_degrees,
        rotation[1] * radians_to_degrees,
        rotation[2] * radians_to_degrees);

    ImGui::SeparatorText("Lens");

    float focal_length = camera.getFocalLength();
    if (ImGui::DragFloat("Focal Length (mm)", &focal_length, 0.5f, 1.0f, 300.0f, "%.1f")) {
        camera.setFocalLength(focal_length);
        renderer.invalidateAccumulation();
    }

    constexpr float sensor_width_mm = 36.0f;
    const float horizontal_fov = 2.0f * std::atan(
        sensor_width_mm / (2.0f * camera.getFocalLength())
    ) * radians_to_degrees;
    ImGui::TextDisabled("Horizontal field of view: %.1f deg", horizontal_fov);

    float aperture_radius = camera.getApertureRadius();
    if (ImGui::DragFloat("Aperture Radius", &aperture_radius, 0.001f, 0.0f, 10.0f, "%.3f")) {
        camera.setApertureRadius(aperture_radius);
        renderer.invalidateAccumulation();
    }

    float focus_distance = camera.getFocusDistance();
    if (ImGui::DragFloat("Focus Distance", &focus_distance, 0.05f, 0.01f, 10000.0f, "%.2f")) {
        camera.setFocusDistance(focus_distance);
        renderer.invalidateAccumulation();
    }

    ImGui::TextWrapped("WASD, Shift, Space - move camera\n"
					   "Middle Mouse - rotate camera\n"
					   "V - switch camera mode\n"
					   "C - enable/disable camera movement");

    ImGui::End();
}

void MyGui::drawPostProcessSettings(Renderer& renderer) {
    if (!m_state.show_post_process_settings)
        return;

    if (!ImGui::Begin("Post Process", &m_state.show_post_process_settings)) {
        ImGui::End();
        return;
    }

    ImGui::SeparatorText("Tone Mapping");

    bool use_aces = renderer.getAcesToneMapping();
    if (ImGui::Checkbox("ACES Tone Mapping", &use_aces))
        renderer.setAcesToneMapping(use_aces);

    float exposure = renderer.getPostProcessExposure();
    if (ImGui::SliderFloat("Exposure", &exposure, -10.0f, 10.0f, "%+.2f EV"))
        renderer.setPostProcessExposure(exposure);

    ImGui::TextDisabled("One exposure stop doubles or halves image brightness.");

    ImGui::SeparatorText("Colour Adjustment");

    float contrast = renderer.getPostProcessContrast();
    if (ImGui::SliderFloat("Contrast", &contrast, 0.0f, 2.0f, "%.2f"))
        renderer.setPostProcessContrast(contrast);

    float saturation = renderer.getPostProcessSaturation();
    if (ImGui::SliderFloat("Saturation", &saturation, 0.0f, 2.0f, "%.2f"))
        renderer.setPostProcessSaturation(saturation);

    ImGui::Spacing();
    if (ImGui::Button("Reset Post Process"))
        renderer.resetPostProcessSettings();

    ImGui::Spacing();
    ImGui::TextWrapped(
        "These controls affect only the displayed and exported image. They do "
        "not restart or modify the accumulated linear HDR render. Disabling "
        "ACES clamps values outside the displayable range."
    );

    ImGui::End();
}

void MyGui::drawEnvironmentSettings(Renderer& renderer) {
    if (!m_state.show_environment_settings)
        return;

    if (!ImGui::Begin("Environment", &m_state.show_environment_settings)) {
        ImGui::End();
        return;
    }

    ImGui::TextUnformatted("Current skybox:");
    ImGui::TextWrapped("%s", renderer.getSkyboxName().c_str());
    ImGui::Spacing();

    bool clamp_skybox = renderer.getSkyboxClamping();
    if (ImGui::Checkbox("Clamp Skybox Radiance", &clamp_skybox))
        renderer.setSkyboxClamping(clamp_skybox);

    if (!clamp_skybox)
        ImGui::BeginDisabled();

    float clamp_value = renderer.getSkyboxClampValue();
    if (ImGui::DragFloat("Maximum Value", &clamp_value, 0.1f, 0.0f, 100000.0f, "%.2f"))
        renderer.setSkyboxClampValue(clamp_value);

    if (!clamp_skybox)
        ImGui::EndDisabled();

    ImGui::End();
}

void MyGui::drawBvhDebug(const Scene& scene) {
    if (!m_state.show_bvh_debug)
        return;

    if (!ImGui::Begin("BVH Debug", &m_state.show_bvh_debug)) {
        ImGui::End();
        return;
    }

    ImGui::Text("Loaded triangles: %zu", scene.getLoadedTriangleCount());
    ImGui::Text("Rendered triangles: %zu", scene.getRenderedTriangleCount());
    ImGui::Text("Flattened BVH triangles: %zu", scene.getTriangles().size());
    ImGui::Text("BVH nodes: %zu", scene.getBvhNodes().size());
    ImGui::Text("Triangle indices: %zu", scene.getBvhTriangleIndicies().size());
    ImGui::Separator();
    ImGui::TextDisabled("BVH node visualization controls can be added here without mixing them into the renderer settings window.");

    ImGui::End();
}

void MyGui::scanMeshFolder() {
    m_mesh_folder_scanned = true;
    m_available_mesh_files.clear();

    const std::filesystem::path mesh_root = Paths::resource("3D_models");
    std::error_code error;

    if (!std::filesystem::exists(mesh_root, error)) {
        setStatusMessage("The resources/3D_models folder could not be found.", true);
        return;
    }

    std::filesystem::recursive_directory_iterator iterator(
        mesh_root,
        std::filesystem::directory_options::skip_permission_denied,
        error
    );
    const std::filesystem::recursive_directory_iterator end;

    while (iterator != end) {
        if (!error && iterator->is_regular_file(error)) {
            std::string extension = iterator->path().extension().string();
            std::transform(extension.begin(), extension.end(), extension.begin(),
                [](unsigned char character) {
                    return static_cast<char>(std::tolower(character));
                });

            if (extension == ".obj")
                m_available_mesh_files.push_back(iterator->path().string());
        }

        error.clear();
        iterator.increment(error);
    }

    std::sort(m_available_mesh_files.begin(), m_available_mesh_files.end());
}

void MyGui::scanSkyboxFolder() {
    m_skybox_folder_scanned = true;
    m_available_skybox_files.clear();

    const std::filesystem::path skybox_root = Paths::resource("skybox_images");
    std::error_code error;

    if (!std::filesystem::exists(skybox_root, error)) {
        setStatusMessage("The resources/skybox_images folder could not be found.", true);
        return;
    }

    std::filesystem::recursive_directory_iterator iterator(
        skybox_root,
        std::filesystem::directory_options::skip_permission_denied,
        error
    );
    const std::filesystem::recursive_directory_iterator end;

    while (iterator != end) {
        if (!error && iterator->is_regular_file(error) &&
            isSupportedSkyboxExtension(iterator->path().extension().string()))
        {
            m_available_skybox_files.push_back(iterator->path().string());
        }

        error.clear();
        iterator.increment(error);
    }

    std::sort(m_available_skybox_files.begin(), m_available_skybox_files.end());
}

void MyGui::drawAssets(
    const Scene& scene,
    const Renderer& renderer,
    GuiActions& actions)
{
    if (!m_state.show_assets)
        return;

    if (!ImGui::Begin("Assets", &m_state.show_assets)) {
        ImGui::End();
        return;
    }

    if (!m_mesh_folder_scanned)
        scanMeshFolder();
    if (!m_skybox_folder_scanned)
		scanSkyboxFolder();

    if (ImGui::Button("Refresh Folders")) {
        scanMeshFolder();
		scanSkyboxFolder();
	}

    ImGui::SameLine();
    ImGui::TextDisabled("Drop OBJ or skybox images anywhere on the application window");

    if (!m_status_message.empty()) {
        const ImVec4 colour = m_status_is_error
            ? ImVec4(1.0f, 0.42f, 0.42f, 1.0f)
            : ImVec4(0.45f, 0.90f, 0.58f, 1.0f);
        ImGui::TextColored(colour, "%s", m_status_message.c_str());
    }

    ImGui::SeparatorText("3D_models Folder");

    const std::filesystem::path mesh_root = Paths::resource("3D_models");

    if (ImGui::BeginTable("AvailableMeshFiles", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("OBJ File", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 75.0f);
        ImGui::TableHeadersRow();

        for (const std::string& file_path : m_available_mesh_files) {
            const std::string comparable_path = normalisedPath(file_path);
            const bool loaded = std::any_of(
                scene.getMeshes().begin(),
                scene.getMeshes().end(),
                [&](const Mesh& mesh) {
                    return normalisedPath(mesh.getFilePath()) == comparable_path;
                }
            );

            std::error_code relative_error;
            std::filesystem::path display_path = std::filesystem::relative(file_path, mesh_root, relative_error);
            if (relative_error)
                display_path = std::filesystem::path(file_path).filename();

            ImGui::PushID(file_path.c_str());
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(display_path.generic_string().c_str());
            ImGui::TableSetColumnIndex(1);

            if (loaded) {
                ImGui::TextDisabled("Loaded");
            }
            else if (ImGui::SmallButton("Load")) {
                actions.mesh_paths_to_load.push_back(file_path);
            }

            ImGui::PopID();
        }

        ImGui::EndTable();
    }

    ImGui::SeparatorText("Skybox Images Folder");

    const std::filesystem::path skybox_root = Paths::resource("skybox_images");

    if (ImGui::BeginTable(
        "AvailableSkyboxFiles",
        2,
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("HDR / Image File", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 75.0f);
        ImGui::TableHeadersRow();

        for (const std::string& file_path : m_available_skybox_files) {
            const bool active = normalisedPath(file_path) ==
                normalisedPath(renderer.getSkyboxName());

            std::error_code relative_error;
            std::filesystem::path display_path = std::filesystem::relative(
                file_path,
                skybox_root,
                relative_error
            );
            if (relative_error)
                display_path = std::filesystem::path(file_path).filename();

            ImGui::PushID(file_path.c_str());
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(display_path.generic_string().c_str());
            ImGui::TableSetColumnIndex(1);

            if (active)
                ImGui::TextDisabled("Active");
            else if (ImGui::SmallButton("Load"))
                actions.skybox_paths_to_load.push_back(file_path);

            ImGui::PopID();
        }

        ImGui::EndTable();
    }
    ImGui::End();
}

void MyGui::drawCreateObjectPopup(Scene& scene, GuiActions& actions) {
    bool keep_open = true;

    if (!ImGui::BeginPopupModal("Create Object", &keep_open, ImGuiWindowFlags_AlwaysAutoResize))
        return;

    ImGui::InputText("Name", m_state.new_object_name, sizeof(m_state.new_object_name));

    const Mesh* selected_mesh = scene.getMesh(m_state.new_object_mesh_id);
    const char* mesh_preview = selected_mesh != nullptr ? selected_mesh->getName().c_str() : "Select a mesh";

    if (ImGui::BeginCombo("Mesh", mesh_preview)) {
        for (const Mesh& mesh : scene.getMeshes()) {
            const bool selected = mesh.getId() == m_state.new_object_mesh_id;
            const std::string label = mesh.getName() + "##new_object_mesh_" + std::to_string(mesh.getId());

            if (ImGui::Selectable(label.c_str(), selected))
                m_state.new_object_mesh_id = mesh.getId();

            if (selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    const Material* selected_material = scene.getMaterial(m_state.new_object_material_id);
    const char* material_preview = selected_material != nullptr ? selected_material->name.c_str() : "Select a material";

    if (ImGui::BeginCombo("Material", material_preview)) {
        for (const Material& material : scene.getMaterials()) {
            const bool selected = material.id == m_state.new_object_material_id;
            const std::string label = material.name + "##new_object_material_" +
                std::to_string(material.id);

            if (ImGui::Selectable(label.c_str(), selected))
                m_state.new_object_material_id = material.id;

            if (selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::SeparatorText("Initial Transform");

    Transform& transform = m_state.new_object_transform;
    ImGui::DragFloat3("Position", &transform.position.x, 0.02f);

    float rotation_degrees[3] = {
        transform.rotation.x * radians_to_degrees,
        transform.rotation.y * radians_to_degrees,
        transform.rotation.z * radians_to_degrees
    };
    if (ImGui::DragFloat3("Rotation", rotation_degrees, 0.25f)) {
        constexpr float degrees_to_radians = 1.0f / radians_to_degrees;
        transform.rotation = {
            rotation_degrees[0] * degrees_to_radians,
            rotation_degrees[1] * degrees_to_radians,
            rotation_degrees[2] * degrees_to_radians
        };
    }

    float scale[3] = {
        transform.scale.x,
        transform.scale.y,
        transform.scale.z
    };
    if (ImGui::DragFloat3("Scale", scale, 0.01f)) {
        transform.scale = {
            std::max(scale[0], 0.001f),
            std::max(scale[1], 0.001f),
            std::max(scale[2], 0.001f)
        };
    }

    const bool validSelection =
        scene.getMesh(m_state.new_object_mesh_id) != nullptr &&
        scene.getMaterial(m_state.new_object_material_id) != nullptr;

    if (!validSelection)
        ImGui::BeginDisabled();

    if (ImGui::Button("Create", ImVec2(120.0f, 0.0f))) {
        const std::string name = m_state.new_object_name[0] != '\0'
            ? m_state.new_object_name
            : "New Object";

        const uint32_t objectId = scene.createObject(
            name,
            m_state.new_object_mesh_id,
            m_state.new_object_material_id,
            m_state.new_object_transform
        );

        m_state.selected_object_id = objectId;
        m_state.selected_material_id = m_state.new_object_material_id;
        m_state.new_object_transform = {};
        std::snprintf(m_state.new_object_name, sizeof(m_state.new_object_name), "New Object");

        requestRasterPreview(actions);
        ImGui::CloseCurrentPopup();
    }

    if (!validSelection)
        ImGui::EndDisabled();

    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120.0f, 0.0f)))
        ImGui::CloseCurrentPopup();

    ImGui::EndPopup();
}

void MyGui::render() const {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* previousContext = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(previousContext);
    }
}

