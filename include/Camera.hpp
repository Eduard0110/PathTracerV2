#pragma once 

#include <array>
#include <cmath>

class Camera {
public:
	Camera();
	Camera(
		std::array<float, 3> position,
		std::array<float, 3> rotation,
		float sensetivity = 1.0f,
		float speed = 1.0f
	);

	void move(float x, float y, float z, float dt);
	void rotate(float mouse_x, float mouse_y, float window_width, float window_height);

	void setPosition(const std::array<float, 3>& pos) { m_position = pos; }
	void setRotation(const std::array<float, 3>& rot) { m_rotation = rot; }
	void setSensetivity(const float sensetivity) { m_sensetivity = sensetivity; }
	void setSpeed(const float speed) { m_speed = speed; }
	void switchUpdate() { m_update = !m_update; }

	const std::array<float, 3>& getPosition() const { return m_position; }
	const std::array<float, 3>& getRotation() const { return m_rotation; }
	const float getSensetivity() const { return m_sensetivity; }
	const float getSpeed() const { return m_speed; }

private:
	std::array<float, 3> m_position;
	std::array<float, 3> m_rotation;
	float m_sensetivity;
	float m_speed;

	bool m_update = true;
};