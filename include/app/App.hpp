#pragma once

// GLad and GLFW3
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <sstream>
#include <iomanip>
#include <memory>
#include <optional>
#include <string>

// renderer with all GL objects and rendering logic
#include "renderer/Renderer.hpp"
#include "camera/Camera.hpp"
#include "core/Diagnostics.hpp"
#include "input/InputManager.hpp"
#include "input/CameraController.hpp"
#include "scene/Scene.hpp"

// ImGui
#include "ui/MyGui.hpp"

// stb image
#include "stb/stb_image.h"
#include <stb/stb_image_write.h>


class App {
public:
	explicit App(bool load_standard_materials = true);
	~App();

	void run();

private:
	// initialisation
	void initGLFW();
	void createWindow();
	void initGLAD();

	// other
	void exportImage(GLuint textureID, const int width, const int height, const char* fname);
	void calculateFps() const;
	void updateDeltaTime();
	void updateWindowCaption() const;

	// main loop functions
	void processInput();
	void updateFrame();
	void renderGui();

	void setRenderMode(Renderer::RenderMode mode);
	std::optional<uint32_t> loadMeshAsset(const std::string& file_path);
	bool loadSkyboxAsset(const std::string& file_path);

private:
	const int m_WINDOW_WIDTH = 1600;
	const int m_WINDOW_HEIGHT = 900;

	std::unique_ptr<Renderer> m_renderer;

	GLFWwindow* m_window = nullptr;

	// ImGui
	MyGui m_my_gui;

	// clock
	double m_last_time;
	double m_delta_time;

	// scene
	Scene m_scene;

	// input manager and camera controller
	InputManager m_input_manager;
	CameraController m_camera_controller;
};
