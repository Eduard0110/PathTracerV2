#include "app/App.hpp"

#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

#include "scene/StandardMaterials.hpp"
#include "core/Paths.hpp"

namespace {
    std::string lowerExtension(const std::string& path) {
        std::string extension = std::filesystem::path(path).extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(),
            [](unsigned char character) {
                return static_cast<char>(std::tolower(character));
            });
        return extension;
    }

    bool isSupportedSkyboxExtension(const std::string& path) {
        const std::string extension = lowerExtension(path);
        return extension == ".hdr" || extension == ".png" ||
            extension == ".jpg" || extension == ".jpeg";
    }

    std::string normalisedAssetPath(const std::string& path) {
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

}


App::App(bool load_standard_materials) :
    m_renderer(std::make_unique<Renderer>(m_WINDOW_WIDTH, m_WINDOW_HEIGHT)),
    m_delta_time(0.0),
    m_last_time(0.0)
{
    initGLFW();
    createWindow();
    initGLAD();
    m_input_manager.init(m_window);
    m_my_gui.initImGui(m_window);
	m_renderer->initiateGLobjects();

	if (load_standard_materials) {
		for (Material& material : createStandardMaterials())
			m_scene.createMaterial(std::move(material));
	}


    m_scene.rebuildScene();

    // initialise GPU scene after loading meshes
	m_renderer->uploadMaterialsToGPU(m_scene);
	m_renderer->uploadGeometryToGPU(m_scene);
}

App::~App() {
    m_renderer.reset();
    
    // shutdown ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(m_window);
    glfwTerminate();
}

// initialisation methods
void App::initGLFW() {
    glfwSetErrorCallback(Diagnostics::glfwErrorCallback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW.\n";
        std::exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

void App::createWindow() {
    m_window = glfwCreateWindow(m_WINDOW_WIDTH, m_WINDOW_HEIGHT, "PathTracer", nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window.\n";
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(0);
}

void App::initGLAD() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD.\n";
        glfwDestroyWindow(m_window);
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    // the post-process shader performs the sRGB conversion
    glDisable(GL_FRAMEBUFFER_SRGB);
}

// export image in png
void App::exportImage(GLuint textureID, const int width, const int height, const char* fname) {
    if (textureID == 0 || width <= 0 || height <= 0 || fname == nullptr)
        throw std::invalid_argument("Cannot export an invalid render image");

    constexpr std::size_t channel_count = 3;
    const std::size_t width_size = static_cast<std::size_t>(width);
    const std::size_t height_size = static_cast<std::size_t>(height);

    if (width_size > std::numeric_limits<std::size_t>::max() / channel_count)
        throw std::overflow_error("Export image row is too large");

    const std::size_t row_bytes = width_size * channel_count;
    if (height_size > std::numeric_limits<std::size_t>::max() / row_bytes)
        throw std::overflow_error("Export image is too large");

    const std::size_t byte_count = row_bytes * height_size;
    if (byte_count > static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()))
        throw std::overflow_error("Export image exceeds OpenGL's readback limit");

    std::vector<unsigned char> pixels(byte_count);

    // RGB rows are not necessarily four-byte aligned. Preserve the caller's
    // state and request tightly packed output to match the vector allocation
    GLint previous_pack_alignment = 4;
    glGetIntegerv(GL_PACK_ALIGNMENT, &previous_pack_alignment);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glGetTextureImage(
        textureID,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        static_cast<GLsizei>(byte_count),
        pixels.data()
    );
    glPixelStorei(GL_PACK_ALIGNMENT, previous_pack_alignment);

    // OpenGL's origin is at the bottom-left; PNG rows start at the top-left.
    for (std::size_t y = 0; y < height_size / 2; ++y) {
        auto top = pixels.begin() + static_cast<std::ptrdiff_t>(y * row_bytes);
        auto bottom = pixels.begin() + static_cast<std::ptrdiff_t>((height_size - y - 1) * row_bytes);
        std::swap_ranges(top, top + static_cast<std::ptrdiff_t>(row_bytes), bottom);
    }

    const std::filesystem::path output_path = Paths::renderOutput(fname);
    if (stbi_write_png(
        output_path.string().c_str(),
        width,
        height,
        static_cast<int>(channel_count),
        pixels.data(),
        static_cast<int>(row_bytes)) == 0)
    {
        throw std::runtime_error("Failed to write render image: " + output_path.string());
    }
}

// clock
void App::updateDeltaTime() {
    double currentTime = glfwGetTime();
    m_delta_time = currentTime - m_last_time;
    m_last_time = currentTime;
}

// shows FPS
void App::updateWindowCaption() const {
    float fps = static_cast<float>(1.0 / m_delta_time);

    std::ostringstream title;
    title << "PathTracer - "
        << std::fixed << std::setprecision(1)
        << fps << " FPS | "
        << m_delta_time << " ms/frame";

    glfwSetWindowTitle(m_window, title.str().c_str());
}

// main loop functions

std::optional<uint32_t> App::loadMeshAsset(const std::string& file_path) {
    const std::filesystem::path path(file_path);
    std::error_code error;

    if (!std::filesystem::exists(path, error) ||
        !std::filesystem::is_regular_file(path, error))
    {
        m_my_gui.setStatusMessage("Mesh file does not exist: " + file_path, true);
        return std::nullopt;
    }

    if (lowerExtension(file_path) != ".obj") {
        m_my_gui.setStatusMessage("Only .obj meshes are currently supported: " + file_path, true);
        return std::nullopt;
    }

    const std::string comparablePath = normalisedAssetPath(file_path);

    // Avoid duplicate mesh IDs and duplicate GPU vertex buffers when the same
    // file is loaded from both the browser and drag-and-drop.
    for (const Mesh& mesh : m_scene.getMeshes()) {
        if (normalisedAssetPath(mesh.getFilePath()) == comparablePath) {
            m_my_gui.setStatusMessage("Mesh is already loaded: " + mesh.getName());
			m_my_gui.refreshMeshFolder();
            return mesh.getId();
        }
    }

    try {
        const std::string resolvedPath = std::filesystem::absolute(path).lexically_normal().string();
        const uint32_t meshId = m_scene.loadMesh(resolvedPath);

        // Both renderers use the same scene mesh ID, while retaining their own
        // geometry representation internally.
        m_renderer->loadRasterMesh(meshId, resolvedPath);

        m_my_gui.setStatusMessage("Loaded mesh: " + path.stem().string());
		m_my_gui.refreshMeshFolder();
        return meshId;
    }
    catch (const std::exception& exception) {
        m_my_gui.setStatusMessage(
            "Failed to load mesh '" + file_path + "': " + exception.what(),
            true
        );
        return std::nullopt;
    }
}

bool App::loadSkyboxAsset(const std::string& file_path) {
    const std::filesystem::path path(file_path);
    std::error_code error;

    if (!std::filesystem::exists(path, error) ||
        !std::filesystem::is_regular_file(path, error))
    {
        m_my_gui.setStatusMessage("Skybox file does not exist: " + file_path, true);
        return false;
    }

    if (!isSupportedSkyboxExtension(file_path)) {
        m_my_gui.setStatusMessage(
            "Supported skybox formats are .hdr, .png, .jpg and .jpeg: " + file_path,
            true
        );
        return false;
    }

    if (normalisedAssetPath(m_renderer->getSkyboxName()) ==
        normalisedAssetPath(file_path))
    {
        m_my_gui.setStatusMessage("Skybox is already active: " + path.stem().string());
        m_my_gui.refreshSkyboxFolder();
        return true;
    }

    try {
        const std::string resolved_path =
            std::filesystem::absolute(path).lexically_normal().string();

        m_renderer->loadSkybox(resolved_path);
        m_my_gui.setStatusMessage("Loaded skybox: " + path.stem().string());
        m_my_gui.refreshSkyboxFolder();
        return true;
    }
    catch (const std::exception& exception) {
        m_my_gui.setStatusMessage(
            "Failed to load skybox '" + file_path + "': " + exception.what(),
            true
        );
        return false;
    }
}

void App::setRenderMode(Renderer::RenderMode mode) {
    if (mode == m_renderer->getRenderMode())
        return;

    if (mode == Renderer::RenderMode::PATH_TRACED) {
        m_scene.rebuildScene();
        m_renderer->uploadMaterialsToGPU(m_scene);
        m_renderer->uploadGeometryToGPU(m_scene);
    }

    m_renderer->setRenderMode(mode);
}

void App::processInput() {
    glfwPollEvents();
    m_my_gui.newFrameImGui();
    m_input_manager.update();

    for (const std::string& droppedFile : m_input_manager.consumeDroppedFiles()) {
        const std::string extension = lowerExtension(droppedFile);

        if (extension == ".obj")
            loadMeshAsset(droppedFile);
        else if (isSupportedSkyboxExtension(droppedFile))
            loadSkyboxAsset(droppedFile);
        else
            m_my_gui.setStatusMessage("Unsupported dropped file: " + droppedFile, true);
    }

    if (m_input_manager.was_pressed(Action::TOGGLE_CAMERA_MODE)) 
        m_camera_controller.updateState(Action::TOGGLE_CAMERA_MODE);

    if (m_input_manager.was_pressed(Action::TOGGLE_CAMERA_MOVEMENT)) 
        m_camera_controller.updateState(Action::TOGGLE_CAMERA_MOVEMENT);

    if (m_input_manager.was_pressed(Action::TOGGLE_RENDERER)) {
        const Renderer::RenderMode newMode =
            m_renderer->getRenderMode() ==
            Renderer::RenderMode::RASTERIZED
            ? Renderer::RenderMode::PATH_TRACED
            : Renderer::RenderMode::RASTERIZED;

        setRenderMode(newMode);
    }
    

    m_camera_controller.update(
        m_scene.getCamera(),
        m_input_manager.get_input_state(),
        static_cast<float>(m_delta_time)
    );
}

void App::updateFrame() {
    glViewport(0, 0, m_WINDOW_WIDTH, m_WINDOW_HEIGHT);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void App::renderGui() {
    const GuiActions actions = m_my_gui.draw(
        m_scene,
        *m_renderer,
        m_camera_controller
    );

    // app owns operations that affect the whole rendering workflow
    if (actions.change_render_mode)
        setRenderMode(actions.requested_render_mode);

    for (const std::string& meshPath : actions.mesh_paths_to_load)
        loadMeshAsset(meshPath);

    for (const std::string& skyboxPath : actions.skybox_paths_to_load)
        loadSkyboxAsset(skyboxPath);

    if (actions.export_image) {
        const std::string filename = "RandomTest" + std::to_string(static_cast<int>(glfwGetTime())) + ".png";
        exportImage(
            m_renderer->getDisplayTextureID(),
            m_renderer->getRenderWidth(),
            m_renderer->getRenderHeight(),
            filename.c_str()
        );
    }

    m_my_gui.render();

    // Resize after ImGui has drawn this frame. The viewport's draw command
    // still references the old texture ID until rendering is complete.
    if (actions.resize_render_output) {
        if (m_renderer->resizeRenderOutput(actions.requested_render_width, actions.requested_render_height)) {
            m_my_gui.setStatusMessage(
                "Render resolution changed to " +
                std::to_string(actions.requested_render_width) + " x " +
                std::to_string(actions.requested_render_height)
            );
        }
        else {
            m_my_gui.setStatusMessage(
                "The requested render resolution is not supported by this GPU.",
                true
            );
        }
    }
}

void App::run() {
    Diagnostics::printOpenGLInfo();
    while (!glfwWindowShouldClose(m_window)) {
        processInput();
        updateFrame();
        m_renderer->update(m_scene, m_camera_controller.hasCameraMoved());
        renderGui();
        updateDeltaTime();
        updateWindowCaption();
        glfwSwapBuffers(m_window);
    }
}
