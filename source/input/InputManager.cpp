#include "input/InputManager.hpp"

#include <cstring>
#include <utility>

void InputManager::init(GLFWwindow* window) {
    m_window = window;
	glfwSetWindowUserPointer(window, this);
    glfwSetScrollCallback(window, scroll_callback);
	glfwSetDropCallback(window, drop_callback);

    glfwGetCursorPos(window, &m_previous_mouse_x, &m_previous_mouse_y);

}

void InputManager::update() {
    update_keyboard();
    update_mouse();

    // scroll is an event, so reset it after each frame
    m_scroll_delta = 0.0;
}

bool InputManager::was_pressed(Action action) const {
    int key = GLFW_KEY_UNKNOWN;

    switch (action) {
    case Action::TOGGLE_RENDERER:
        key = GLFW_KEY_R;
        break;

    case Action::TOGGLE_CAMERA_MOVEMENT:
        key = GLFW_KEY_C;
        break;

    case Action::TOGGLE_CAMERA_MODE:
        key = GLFW_KEY_V;
        break;
    }

    if (key == GLFW_KEY_UNKNOWN)
        return false;

    return m_current_keys[key] && !m_previous_keys[key];
}

std::vector<std::string> InputManager::consumeDroppedFiles() {
	std::vector<std::string> files = std::move(m_dropped_files);
	m_dropped_files.clear();
	return files;
}

void InputManager::update_keyboard() {
    std::memcpy(
        m_previous_keys,
        m_current_keys,
        sizeof(m_current_keys)
    );

    for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; ++key)
        m_current_keys[key] = glfwGetKey(m_window, key) == GLFW_PRESS;
    
	// set the input state based on the current keys
    m_input_state.w = m_current_keys[GLFW_KEY_W];
    m_input_state.s = m_current_keys[GLFW_KEY_S];
    m_input_state.a = m_current_keys[GLFW_KEY_A];
    m_input_state.d = m_current_keys[GLFW_KEY_D];
    m_input_state.space = m_current_keys[GLFW_KEY_SPACE];
    m_input_state.shift = m_current_keys[GLFW_KEY_LEFT_SHIFT]; // left shift only for now || m_current_keys[GLFW_KEY_RIGHT_SHIFT];
}

void InputManager::update_mouse() {
    std::memcpy(
        m_previous_mouse_buttons,
        m_current_mouse_buttons,
        sizeof(m_current_mouse_buttons)
    );

    for (int button = 0; button <= GLFW_MOUSE_BUTTON_LAST; ++button)
        m_current_mouse_buttons[button] = glfwGetMouseButton(m_window, button) == GLFW_PRESS;
    

    m_input_state.left_mouse = m_current_mouse_buttons[GLFW_MOUSE_BUTTON_LEFT];
    m_input_state.right_mouse = m_current_mouse_buttons[GLFW_MOUSE_BUTTON_RIGHT];
    m_input_state.middle_mouse = m_current_mouse_buttons[GLFW_MOUSE_BUTTON_MIDDLE];

    double mouse_x;
    double mouse_y;

    glfwGetCursorPos(m_window, &mouse_x, &mouse_y);

    m_input_state.mouse_delta_x = mouse_x - m_previous_mouse_x;
    m_input_state.mouse_delta_y = mouse_y - m_previous_mouse_y;

    m_previous_mouse_x = mouse_x;
    m_previous_mouse_y = mouse_y;

    m_input_state.scroll_delta = m_scroll_delta;
}

void InputManager::scroll_callback( GLFWwindow* window, double x_offset, double y_offset) {
    InputManager* input = static_cast<InputManager*>(glfwGetWindowUserPointer(window));

    input->m_scroll_delta = y_offset;
}

void InputManager::drop_callback(GLFWwindow* window, int count, const char** paths) {
	InputManager* input = static_cast<InputManager*>(glfwGetWindowUserPointer(window));

	if (input == nullptr || paths == nullptr)
		return;

	for (int i = 0; i < count; ++i) {
		if (paths[i] != nullptr)
			input->m_dropped_files.emplace_back(paths[i]);
	}
}

