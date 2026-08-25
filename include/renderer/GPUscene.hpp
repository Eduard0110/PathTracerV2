#pragma once

#include <glad/glad.h>
#include <unordered_map>
#include <vector>

#include "scene/Geometry.hpp"
#include "renderer/GPUData.hpp"

class GPUscene {
public:
	enum class BufferType {
		TRIANGLE_SSBO,
		BVH_SSBO,
		BVH_TRIANGLE_INDEX_SSBO,
		MATERIAL_SSBO
	};

public:
	GPUscene() = default;
	~GPUscene();

	template <typename T>
	void uploadBufferToGPU(BufferType type, const std::vector<T>& data);
	
	GLuint getSSBOBuffer(BufferType type) { return m_buffer_map[type]; }

private:
	std::unordered_map<BufferType, GLuint> m_buffer_map;
};