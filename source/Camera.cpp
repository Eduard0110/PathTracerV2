#include "Camera.hpp"

Camera::Camera() 
	: m_position{ 0.0f, 1.0f, -5.0f },
	  m_rotation{ 0.0f, 0.0f, 0.0f },
	  m_sensetivity(1.0f), 
	  m_speed(10.0f) {

}

Camera::Camera(std::array<float, 3> position,
			   std::array<float, 3> rotation,
			   float sensitivity,
			   float speed)
	: m_position(position),
	  m_rotation(rotation),
	  m_sensetivity(sensitivity),
	  m_speed(speed) {
}

void Camera::rotate(float mouse_x, float mouse_y, float window_width, float window_height) {
	// if camera update is set to false
	if (!m_update) return;
	// normalize mouse position relative to screen center
	float normalizedX = (mouse_x - window_width / 2.0f) / window_width;
	float normalizedY = (mouse_y - window_height / 2.0f) / window_height;

	float yaw = normalizedX * 360.0f * m_sensetivity;    // Y-axis
	float pitch = normalizedY * 180.0f * m_sensetivity;  // X-axis 

	m_rotation[0] = yaw;
	m_rotation[1] = pitch;
}

void Camera::move(float x, float y, float z, float dt) {
	// if camera update is set to false
	if (!m_update) return;

	float cos_a = cos(m_rotation[0] * (3.14f / 180.0f));
	float sin_a = sin(m_rotation[0] * (3.14f / 180.0f));

	if (z != 0.0f) { // forward and back
		m_position[2] += m_speed * cos_a * z * dt;
		m_position[0] += m_speed * sin_a * z * dt;
	}
	if (x != 0.0f) { // right and left
		m_position[2] -= m_speed * sin_a * x * dt;
		m_position[0] += m_speed * cos_a * x * dt;
	}
	if (y != 0.0f) { // up and down
		m_position[1] += m_speed * y * dt;
	}
}