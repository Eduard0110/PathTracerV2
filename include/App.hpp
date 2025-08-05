#pragma once

// GLad and GLFW3
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <sstream>
#include <iomanip>

// wrapper classes
#include "RectVAO.hpp"
#include "ShaderProgram.hpp"
#include "ComputeProgram.hpp"
#include "Framebuffer.hpp"
#include "Texture.hpp"

// ImGui
#include "MyGui.hpp"

// stb image
#include "stb/stb_image.h"
#include <stb/stb_image_write.h>

#include "Camera.hpp"

class App {
public:
	App();
	~App();

	void run();

private:
	// initialisation
	void initGLFW();
	void createWindow();
	void initGLAD();
	static void glfwErrorCallback(int error, const char* error_message);

	// other
	void printDebugInfo();
	void setStaticUniforms();
	void setPerFrameUniforms();
	void exportImage(GLuint textureID, const int width, const int height, const char* fname);
	void calculateFps() const;
	void updateDeltaTime();
	void updateWindowCaption() const;

	// main loop functions
	void checkCameraChange();
	void processInput();
	void updateFrame();
	void runComputeShaderPass();
	void runAccumulationPass();
	void renderGui();

private:
	static constexpr int m_WINDOW_WIDTH = 2560;
	static constexpr int m_WINDOW_HEIGHT = 1440;

	struct FrameUniforms {
		float time = 0.0f;
		float frame_count = 1.0f;
	};

	GLFWwindow* m_window = nullptr;

	// ImGui
	MyGui m_my_gui;

	// uniforms
	FrameUniforms m_uniforms;

	// first frame
	bool m_first_frame = true;
	bool m_ping = true;

	// clock
	double m_last_time = 0.0;
	double m_delta_time = 0.0;

	// GL objects
	ComputeProgram m_compute_program;
	ShaderProgram m_accumulation_shader_program;

	RectVAO m_rect_vao;

	Texture m_compute_texture;

	Texture m_ping1_texture;
	Texture m_ping2_texture;
	Texture m_post_proccess_texture;

	Framebuffer m_ping1_fbo;
	Framebuffer m_ping2_fbo;
	Framebuffer m_post_proccess_fbo;
	
	// Camera
	Camera m_camera;
	std::array<float, 3> m_last_camera_pos = { 0.0f, 0.0f, 0.0f };
	std::array<float, 3> m_last_camera_rot = { 0.0f, 0.0f, 0.0f };

	// key states to track toggle events
	std::unordered_map<int, bool> m_keyStates;
	bool m_accumulate = false;

};
