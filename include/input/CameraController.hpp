#pragma once

#include <array>
#include <GLFW/glfw3.h>

#include "input/InputTypes.hpp"
#include "camera/Camera.hpp"


class CameraController {
public:
	enum class CameraMode {
		ORBIT,
		FREE_FLY
	};

	enum class CameraState {
		IDLE,
		MOVING
	};

public:
	CameraController();
	~CameraController() = default;

	void update(Camera& camera, const InputState& input_state, float delta_time);
	void freeFly(Camera& camera, const InputState& input_state, float delta_time);
	void orbit(Camera& camera, const InputState& input_state, float delta_time);
	void updateState(const Action& action);
	bool hasCameraMoved() const { return m_camera_moved; }

	// getters / setters
	CameraMode getCameraMode() { return m_camera_mode; }
	CameraState getCameraState() { return m_camera_state; }

	void setCameraMode(CameraMode mode);
	void setCameraState(CameraState state) { m_camera_state = state; }

private:
	void orbitInitCorrection(Camera& camera);

private:
	CameraMode m_camera_mode;
	CameraState m_camera_state;

	// track if camera has moved
	std::array<float, 3> m_previous_position{};
	std::array<float, 3> m_previous_rotation{};

	bool m_camera_moved = false;
	bool m_orbit_initialized = false;

	// orbit mode
	std::array<float, 3> m_orbit_target{ 0.0f, 0.0f, 0.0f };
	float m_orbit_distance = 5.0f;
	float m_move_speed = 5.0f;
	float m_orbit_speed = 0.005f;
	float m_zoom_speed = 0.5f;
};
