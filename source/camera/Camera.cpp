#include "camera/Camera.hpp"


Camera::Camera() : 
	m_position{0.0f, 0.0f, 4.0f},
	m_rotation{0.0f, 0.0f, 0.0f}
{
}

void Camera::moveTo(float x, float y, float z) {
    m_position = { x, y, z };
}

void Camera::moveBy(float right, float forward, float up) {
	// right
	m_position[0] += right * std::cos(m_rotation[1]);
	m_position[2] -= right * std::sin(m_rotation[1]);

	// forward
	m_position[0] += forward * std::sin(m_rotation[1]);
	m_position[2] += forward * std::cos(m_rotation[1]);

	// up
	m_position[1] += up;
}

void Camera::rotateBy(float mouse_delta_x, float mouse_delta_y)
{
    m_rotation[1] += mouse_delta_x;  // yaw
    m_rotation[0] -= mouse_delta_y;  // pitch

    // prevent looking beyond straight up/down
    constexpr float max_pitch = 1.5707963f - 0.01f;

    m_rotation[0] = std::clamp(m_rotation[0], -max_pitch, max_pitch);
}

void Camera::lookAt(float x, float y, float z)
{
    const float dx = x - m_position[0];
    const float dy = y - m_position[1];
    const float dz = z - m_position[2];

    const float horizontal_distance =
        std::sqrt(dx * dx + dz * dz);

    m_rotation[0] = std::atan2(dy, horizontal_distance);  // pitch
    m_rotation[1] = std::atan2(dx, dz);  // yaw
}

void Camera::setFocalLength(float focal_length_mm) {
    m_focal_length_mm = std::max(focal_length_mm, 1.0f);
}

void Camera::setApertureRadius(float aperture_radius) {
    m_aperture_radius = std::max(aperture_radius, 0.0f);
}

void Camera::setFocusDistance(float focus_distance) {
    m_focus_distance = std::max(focus_distance, 0.01f);
}
