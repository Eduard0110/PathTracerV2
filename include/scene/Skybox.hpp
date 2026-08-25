#pragma once

#include <string>

#include "renderer/Texture.hpp"


class Skybox {
public:
	Skybox()  = default;
	~Skybox() = default;

	void init();

	// will overwrite previously loaded skybox texture
	void loadSkybox(const std::string& file_path);
	void bind(GLenum texture_unit) const { m_texture.bind(texture_unit); }

	const std::string& getName() const { return m_name; }
	GLuint getTextureID() const { return m_texture.getID(); }

private:
	std::string m_name;
	Texture m_texture;
};
