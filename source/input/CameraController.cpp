#include "input/CameraController.hpp"

CameraController::CameraController()
	: m_camera_mode(CameraMode::ORBIT),
	m_camera_state(CameraState::MOVING)
{
}

void CameraController::setCameraMode(CameraMode mode) {
	if (mode != m_camera_mode) {
		m_camera_mode = mode;
		m_orbit_initialized = false;
	}
}

void CameraController::update(Camera& camera, const InputState& input_state, float delta_time) {
	m_camera_moved = false;

	// exit if idle
	if (m_camera_state == CameraState::IDLE)
		return;

	// store camera state before movement
	const auto previous_position = camera.getPosition();
	const auto previous_rotation = camera.getRotation();

	switch (m_camera_mode)
	{
	case CameraMode::FREE_FLY:
		freeFly(camera, input_state, delta_time);
		break;

	case CameraMode::ORBIT:
		orbit(camera, input_state, delta_time);
		break;
	}

	// check whether camera actually changed
	m_camera_moved =
		previous_position != camera.getPosition() ||
		previous_rotation != camera.getRotation();

}

void CameraController::freeFly(Camera& camera, const InputState& input_state, float delta_time) {
	float right = 0.0f;
	float forward = 0.0f;
	float up = 0.0f;

	if (input_state.w)     forward += 1.0f;
	if (input_state.s)     forward -= 1.0f;
	if (input_state.d)     right += 1.0f;
	if (input_state.a)     right -= 1.0f;
	if (input_state.space) up += 1.0f;
	if (input_state.shift) up -= 1.0f;

	// Prevent diagonal movement from being faster.
	const float length = std::sqrt(right * right + forward * forward + up * up);

	if (length > 0.0f) {
		const float distance = m_move_speed * delta_time;

		right = right / length * distance;
		forward = forward / length * distance;
		up = up / length * distance;

		camera.moveBy(right, forward, up);
	}

	if (input_state.middle_mouse) {
		camera.rotateBy(
			static_cast<float>(input_state.mouse_delta_x) * m_orbit_speed,
			static_cast<float>(input_state.mouse_delta_y) * m_orbit_speed
		);
	}
}

void CameraController::orbitInitCorrection(Camera& camera) {
	if (!m_orbit_initialized) {
		const auto position = camera.getPosition();

		const float dx = position[0] - m_orbit_target[0];
		const float dy = position[1] - m_orbit_target[1];
		const float dz = position[2] - m_orbit_target[2];

		m_orbit_distance = std::max(0.1f, std::sqrt(dx * dx + dy * dy + dz * dz));

		camera.lookAt(m_orbit_target[0], m_orbit_target[1], m_orbit_target[2]);

		m_orbit_initialized = true;
	}
}

void CameraController::orbit(Camera& camera, const InputState& input_state, float delta_time) {
	orbitInitCorrection(camera);

	if (input_state.middle_mouse) {
		camera.rotateBy(
			static_cast<float>(input_state.mouse_delta_x) * m_orbit_speed,
			static_cast<float>(input_state.mouse_delta_y) * m_orbit_speed
		);
	}

	if (input_state.scroll_delta != 0.0) {
		m_orbit_distance -= static_cast<float>(input_state.scroll_delta) * m_zoom_speed;
		m_orbit_distance = std::max(0.1f, m_orbit_distance);
	}

	const auto rotation = camera.getRotation();

	const float pitch = rotation[0];
	const float yaw = rotation[1];

	// Direction from the camera towards the orbit target.
	const float forward_x = std::sin(yaw) * std::cos(pitch);
	const float forward_y = std::sin(pitch);
	const float forward_z = std::cos(yaw) * std::cos(pitch);

	// Place the camera behind the target along its forward direction.
	const float camera_x = m_orbit_target[0] - forward_x * m_orbit_distance;
	const float camera_y = m_orbit_target[1] - forward_y * m_orbit_distance;
	const float camera_z = m_orbit_target[2] - forward_z * m_orbit_distance;

	camera.moveTo(camera_x, camera_y, camera_z);
}

void CameraController::updateState(const Action& action) {
	if (action == Action::TOGGLE_CAMERA_MODE)
	{
		if (m_camera_mode == CameraMode::FREE_FLY)
			m_camera_mode = CameraMode::ORBIT;
		else
			m_camera_mode = CameraMode::FREE_FLY;
	}
	else if (action == Action::TOGGLE_CAMERA_MOVEMENT)
	{
		if (m_camera_state == CameraState::IDLE)
			m_camera_state = CameraState::MOVING;
		else
			m_camera_state = CameraState::IDLE;
	}
}