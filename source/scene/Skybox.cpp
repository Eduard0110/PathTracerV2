#include "scene/Skybox.hpp"

#include <stdexcept>

#include "stb/stb_image.h"

void Skybox::init() {
	// used until an environment image is selected
	const float fallback_pixel[4] = { 0.5f, 0.6f, 1.0f, 1.0f };
	m_texture.create(1, 1, fallback_pixel);
	m_texture.setWrap(GL_REPEAT, GL_CLAMP_TO_EDGE);
	m_name = "Default sky";
}

void Skybox::loadSkybox(const std::string& file_path) {
	int width = 0;
	int height = 0;
	int source_channels = 0;

	// stbi_loadf converts .png .jpg .hdr to floating point representation
	float* pixels = stbi_loadf(
		file_path.c_str(),
		&width,
		&height,
		&source_channels,
		STBI_rgb_alpha
	);

	if (pixels == nullptr) {
		throw std::runtime_error(
			"Failed to load skybox '" + file_path + "': " +
			stbi_failure_reason()
		);
	}

	m_texture.create(width, height, pixels);
	m_texture.setWrap(GL_REPEAT, GL_CLAMP_TO_EDGE);
	m_name = file_path;

	stbi_image_free(pixels);

}
