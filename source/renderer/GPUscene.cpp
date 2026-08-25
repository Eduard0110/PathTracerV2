#include "renderer/GPUscene.hpp"


template <typename T>
void GPUscene::uploadBufferToGPU(BufferType type, const std::vector<T>& data) 
{
	if (m_buffer_map[type] == 0)
		glGenBuffers(1, &m_buffer_map[type]);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer_map[type]);

	glBufferData(
		GL_SHADER_STORAGE_BUFFER,
		data.size() * sizeof(T),
		data.data(),
		GL_STATIC_DRAW
	);
}

GPUscene::~GPUscene() {
	for (const auto& [type, buffer] : m_buffer_map)
		if (buffer != 0)
			glDeleteBuffers(1, &buffer);
}

// excplicit template instantiations for all types required
template void GPUscene::uploadBufferToGPU<Triangle>(
	BufferType,
	const std::vector<Triangle>&
);

template void GPUscene::uploadBufferToGPU<BVHNode>(
	BufferType,
	const std::vector<BVHNode>&
);

template void GPUscene::uploadBufferToGPU<int>(
	BufferType,
	const std::vector<int>&
);

template void GPUscene::uploadBufferToGPU<GPUMaterial>(
	BufferType,
	const std::vector<GPUMaterial>&
);