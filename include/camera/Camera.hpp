#pragma once

#include <array>
#include <cmath>
#include <iostream>
#include <algorithm>

class Camera {
public:

public:
	Camera();

	//void processMouseMovement(float mouseX, float mouseY, int WINDOW_WIDTH, int WINDOW_HEIGHT);
	void rotateBy(float mouse_delta_x, float mouse_delta_y);
	void moveBy(float right, float forward, float up);
	void lookAt(float x, float y, float z);
	void moveTo(float x, float y, float z);
	void setFocalLength(float focal_length_mm);
	void setApertureRadius(float aperture_radius);
	void setFocusDistance(float focus_distance);

	// Getters
	std::array<float, 3> getPosition() const { return m_position; }
	std::array<float, 3> getRotation() const { return m_rotation; }
	float getFocalLength() const { return m_focal_length_mm; }
	float getApertureRadius() const { return m_aperture_radius; }
	float getFocusDistance() const { return m_focus_distance; }

private:
	std::array<float, 3> m_position;
	std::array<float, 3> m_rotation;

	// Focal length is expressed in millimetres. Aperture radius and focus
	// distance use scene units because they describe distances in the scene.
	float m_focal_length_mm = 50.0f;
	float m_aperture_radius = 0.0f;
	float m_focus_distance = 4.0f;
};
