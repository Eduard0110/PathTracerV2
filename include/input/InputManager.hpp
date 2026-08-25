#pragma once

#include <string>
#include <vector>

#include <GLFW/glfw3.h>

#include "input/InputTypes.hpp"

class InputManager {
public:
    InputManager()  = default;
    ~InputManager() = default;
    
    void init(GLFWwindow* window);
    void update();
        
    // continuous input
    const InputState& get_input_state() const { return m_input_state; }

    // actions
    bool was_pressed(Action action) const;
	std::vector<std::string> consumeDroppedFiles();

private:
    static void scroll_callback(GLFWwindow* window, double x_offset, double y_offset);
	static void drop_callback(GLFWwindow* window, int count, const char** paths);
	void update_keyboard();
	void update_mouse();

private:
    GLFWwindow* m_window;

    InputState m_input_state;

    bool m_previous_keys[GLFW_KEY_LAST + 1]{};
    bool m_current_keys[GLFW_KEY_LAST + 1]{};

    bool m_previous_mouse_buttons[GLFW_MOUSE_BUTTON_LAST + 1]{};
    bool m_current_mouse_buttons[GLFW_MOUSE_BUTTON_LAST + 1]{};

    double m_previous_mouse_x = 0.0;
    double m_previous_mouse_y = 0.0;

    double m_scroll_delta = 0.0;
	std::vector<std::string> m_dropped_files;
};
