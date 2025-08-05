#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>

class MyGui {
public:
	MyGui();
	~MyGui();

	void dock();
	void render() const;
	void createGui(GLuint textureID, int texWidth, int texHeight);

	void initImGui(GLFWwindow* window);
	void newFrameImGui();

};