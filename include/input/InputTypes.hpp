#pragma once

enum class Action {
    TOGGLE_RENDERER,
    TOGGLE_CAMERA_MOVEMENT,
    TOGGLE_CAMERA_MODE
};

struct InputState {
    // keyboard
    bool w = false;
    bool s = false;
    bool a = false;
    bool d = false;
    bool space = false;
    bool shift = false;

    // mouse buttons
    bool left_mouse = false;
    bool right_mouse = false;
    bool middle_mouse = false;

    // mouse movement
    double mouse_delta_x = 0.0;
    double mouse_delta_y = 0.0;

    // scroll wheel
    double scroll_delta = 0.0;
};