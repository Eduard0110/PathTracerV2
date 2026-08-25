#pragma once

#include <array>
#include <glad/glad.h>
// wrapper classes
#include "renderer/RectVAO.hpp"
#include "renderer/ShaderProgram.hpp"
#include "renderer/ComputeProgram.hpp"
#include "renderer/Framebuffer.hpp"
#include "renderer/Texture.hpp"
#include "camera/Camera.hpp"
#include "scene/Scene.hpp"
#include "scene/Skybox.hpp"
#include "renderer/GPUscene.hpp"
#include "renderer/RasterRenderer.hpp"

#include "core/Constants.hpp"


class Renderer {
public:
	enum class RenderMode {
		RASTERIZED,
		PATH_TRACED
	};

public:
	Renderer(int render_width, int render_height);
	~Renderer() = default;

	void initiateGLobjects();
	bool resizeRenderOutput(int width, int height);
	//void uploadSceneToGPU(const Scene& scene);
	void uploadMaterialsToGPU(const Scene& scene);
	void uploadGeometryToGPU(const Scene& scene);
	void loadRasterMesh(uint32_t meshId, const std::string& filePath);
	void loadSkybox(const std::string& file_path);

	GPUMaterial makeGPUMaterial(const Material& material);

	GLuint getTextureID(int type);

	void update(const Scene& scene, bool cameraChanged);

	// uniform getters/setters
	void setRussianRoulette(bool enabled);
	void setMaxReflections(int max_reflections);
	void setSamplesPerPixel(int samples_per_pixel);
	void setSkyboxClamping(bool enabled);
	void setSkyboxClampValue(float value);
	void setPostProcessExposure(float exposure);
	void setPostProcessContrast(float contrast);
	void setPostProcessSaturation(float saturation);
	void setAcesToneMapping(bool enabled);
	void resetPostProcessSettings();
	void invalidateAccumulation();

	bool getRussianRoulette() const { return m_uniforms.russian_roulette; }
	int  getMaxReflections()  const { return m_uniforms.max_reflections; }
	int  getSamplesPerPixel() const { return m_uniforms.samples_per_pixel; }
	bool getSkyboxClamping() const { return m_uniforms.clamp_skybox; }
	float getSkyboxClampValue() const { return m_uniforms.skybox_clamp_value; }
	float getPostProcessExposure() const { return m_uniforms.post_exposure; }
	float getPostProcessContrast() const { return m_uniforms.post_contrast; }
	float getPostProcessSaturation() const { return m_uniforms.post_saturation; }
	bool getAcesToneMapping() const { return m_uniforms.use_aces_tone_mapping; }

	void setRenderMode(RenderMode mode);
	RenderMode getRenderMode() const { return m_render_mode; }
	void toggleRenderMode();

	GLuint getDisplayTextureID() const;
	int getRenderWidth() const { return m_render_width; }
	int getRenderHeight() const { return m_render_height; }
	int getMaxRenderDimension() const { return m_max_texture_size; }
	const std::string& getSkyboxName() const { return m_skybox.getName(); }

private:
	void resetAccumulation();
	void setComputeShaderUniforms(const Camera& camera);
	void runComputeShaderPass(const Camera& camera);
	void runAccumulationPass();
	void runPostProcessPass(GLuint input_texture);

private:
	int m_render_width;
	int m_render_height;
	int m_max_texture_size = 0;

	struct FrameUniforms {
		int total_frame_count = 0;
		float accumulated_frame_count = 0;  // float for shader compatibility
		bool russian_roulette = true;
		int max_reflections = 7;
		int samples_per_pixel = 1;
		bool clamp_skybox = false;
		float skybox_clamp_value = 10.0f;
		float post_exposure = 0.0f;
		float post_contrast = 1.0f;
		float post_saturation = 1.0f;
		bool use_aces_tone_mapping = true;
	};

	FrameUniforms m_uniforms;
	RenderMode m_render_mode = RenderMode::PATH_TRACED;
	
	bool m_ping = true;
	bool m_has_last_camera = false;

	std::array<float, 3> m_last_camera_position{};
	std::array<float, 3> m_last_camera_rotation{};

	// GL objects
	ComputeProgram m_compute_program;
	ShaderProgram m_accumulation_shader_program;
	ShaderProgram m_post_process_shader_program;

	RectVAO m_rect_vao;

	Texture m_compute_texture;

	Texture m_ping1_texture;
	Texture m_ping2_texture;
	Texture m_post_process_texture;

	Framebuffer m_ping1_fbo;
	Framebuffer m_ping2_fbo;
	Framebuffer m_post_process_fbo;

	// GPU scene
	// cotains all SSBO buffers
	GPUscene m_gpu_scene;

	Skybox m_skybox;

	RasterRenderer m_raster_renderer;
};
