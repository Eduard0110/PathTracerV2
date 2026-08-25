#include "renderer/Renderer.hpp"

#include "core/Paths.hpp"

Renderer::Renderer(int render_width, int render_height)
	:   m_render_width(render_width),
        m_render_height(render_height),
        m_raster_renderer(render_width, render_height),
        m_compute_texture(render_width, render_height),
		m_ping1_texture(render_width, render_height, GL_RGBA32F),
		m_ping2_texture(render_width, render_height, GL_RGBA32F),
		m_post_process_texture(render_width, render_height)
{
}

void Renderer::initiateGLobjects() {
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_max_texture_size);

    m_compute_program.createAndCompile(Paths::shader("ComputeShader.comp").string());
    m_accumulation_shader_program.createAndCompile(
        Paths::shader("VertexShader.vs").string(),
        Paths::shader("AccumulationShader.fs").string()
    );
    m_post_process_shader_program.createAndCompile(
        Paths::shader("VertexShader.vs").string(),
        Paths::shader("PostProcessShader.fs").string()
    );

    m_raster_renderer.init();

    m_rect_vao.createVao();

    m_compute_texture.create();
    m_ping1_texture.create();
    m_ping2_texture.create();
    m_post_process_texture.create();
    m_skybox.init();

    m_ping1_fbo.attachTexture(m_ping1_texture.getID(), m_render_width, m_render_height);
    m_ping2_fbo.attachTexture(m_ping2_texture.getID(), m_render_width, m_render_height);
    m_post_process_fbo.attachTexture(m_post_process_texture.getID(), m_render_width, m_render_height);

}

GPUMaterial Renderer::makeGPUMaterial(const Material& material) {
    return GPUMaterial{
        Vec4{
            material.baseColour.x,
            material.baseColour.y,
            material.baseColour.z,
            1.0f
        },
        Vec4{
            material.emission.x,
            material.emission.y,
            material.emission.z,
            material.emissionStrength
        },
        Vec4{
            material.absorption.x,
            material.absorption.y,
            material.absorption.z,
            material.absorptionStrength
        },
        material.roughness,
        material.metallic,
        material.ior,
        material.isTransparent
    };
}

void Renderer::loadRasterMesh(uint32_t meshId, const std::string& filePath) {
    m_raster_renderer.loadMesh(meshId, filePath);
}

bool Renderer::resizeRenderOutput(int width, int height) {
    if (width <= 0 || height <= 0)
        return false;

    if (m_max_texture_size > 0 && (width > m_max_texture_size || height > m_max_texture_size)) {
        return false;
    }

    if (width == m_render_width && height == m_render_height)
        return true;

    m_render_width = width;
    m_render_height = height;

    // update all render targets so they all have the same resolution as the output resolution
    m_compute_texture.create(width, height, nullptr);
    m_ping1_texture.create(width, height, nullptr);
    m_ping2_texture.create(width, height, nullptr);
    m_post_process_texture.create(width, height, nullptr);

    m_ping1_fbo.attachTexture(m_ping1_texture.getID(), width, height);
    m_ping2_fbo.attachTexture(m_ping2_texture.getID(), width, height);
    m_post_process_fbo.attachTexture(m_post_process_texture.getID(), width, height);

    m_raster_renderer.resize(width, height);
    resetAccumulation();
    return true;
}

void Renderer::loadSkybox(const std::string& file_path) {
    m_skybox.loadSkybox(file_path);
    resetAccumulation();
}

void Renderer::uploadMaterialsToGPU(const Scene& scene) {
    if (m_render_mode == RenderMode::PATH_TRACED)
        resetAccumulation();

    std::vector<GPUMaterial> gpuMaterials;
    gpuMaterials.reserve(scene.getMaterials().size());

    for (const auto& material : scene.getMaterials())
        gpuMaterials.push_back(makeGPUMaterial(material));

    m_gpu_scene.uploadBufferToGPU(GPUscene::BufferType::MATERIAL_SSBO, gpuMaterials);
}

void Renderer::uploadGeometryToGPU(const Scene& scene) {
    if (m_render_mode == RenderMode::PATH_TRACED)
        resetAccumulation();

    m_gpu_scene.uploadBufferToGPU(GPUscene::BufferType::TRIANGLE_SSBO, scene.getTriangles());
    m_gpu_scene.uploadBufferToGPU(GPUscene::BufferType::BVH_SSBO, scene.getBvhNodes());
    m_gpu_scene.uploadBufferToGPU(GPUscene::BufferType::BVH_TRIANGLE_INDEX_SSBO, scene.getBvhTriangleIndicies());

}

void Renderer::update( const Scene& scene, bool cameraChanged) {
    const Camera& camera = scene.getCamera();

    if (m_render_mode == RenderMode::RASTERIZED) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_gpu_scene.getSSBOBuffer(GPUscene::BufferType::MATERIAL_SSBO));
        m_raster_renderer.render( scene.getObjects(), camera );
        runPostProcessPass(m_raster_renderer.getTextureID());
        return;
    }

    if (cameraChanged)
        resetAccumulation();

    ++m_uniforms.total_frame_count;

    runComputeShaderPass(camera);
    runAccumulationPass();
    const GLuint accumulation_texture = m_ping
        ? m_ping2_texture.getID()
        : m_ping1_texture.getID();
    runPostProcessPass(accumulation_texture);
}

void Renderer::setMaxReflections(int value) {
    value = std::max(value, 1);

    if (value == m_uniforms.max_reflections)
        return;

    m_uniforms.max_reflections = value;
    resetAccumulation();
}

void Renderer::setSamplesPerPixel(int value) {
    value = std::clamp(value, 1, 64);

    if (value == m_uniforms.samples_per_pixel)
        return;

    m_uniforms.samples_per_pixel = value;
    resetAccumulation();
}

void Renderer::setRussianRoulette(bool enabled) {
    if (enabled == m_uniforms.russian_roulette)
        return;

    m_uniforms.russian_roulette = enabled;
    resetAccumulation();
}

void Renderer::setSkyboxClamping(bool enabled) {
    if (enabled == m_uniforms.clamp_skybox)
        return;

    m_uniforms.clamp_skybox = enabled;
    resetAccumulation();
}

void Renderer::setSkyboxClampValue(float value) {
    value = std::max(value, 0.0f);

    if (value == m_uniforms.skybox_clamp_value)
        return;

    m_uniforms.skybox_clamp_value = value;
    resetAccumulation();
}

void Renderer::setPostProcessExposure(float value) {
    m_uniforms.post_exposure = std::clamp(value, -10.0f, 10.0f);
}

void Renderer::setPostProcessContrast(float value) {
    m_uniforms.post_contrast = std::clamp(value, 0.0f, 4.0f);
}

void Renderer::setPostProcessSaturation(float value) {
    m_uniforms.post_saturation = std::clamp(value, 0.0f, 4.0f);
}

void Renderer::setAcesToneMapping(bool enabled) {
    m_uniforms.use_aces_tone_mapping = enabled;
}

void Renderer::resetPostProcessSettings() {
    m_uniforms.post_exposure = 0.0f;
    m_uniforms.post_contrast = 1.0f;
    m_uniforms.post_saturation = 1.0f;
    m_uniforms.use_aces_tone_mapping = true;
}

void Renderer::invalidateAccumulation() {
    resetAccumulation();
}

void Renderer::resetAccumulation() {
    m_uniforms.accumulated_frame_count = 0.0f;
    m_ping = true;
}

void Renderer::setRenderMode(RenderMode mode) {
    if (mode == m_render_mode)
        return;

    m_render_mode = mode;
    resetAccumulation();
}

void Renderer::toggleRenderMode() {
    m_render_mode =
        m_render_mode == RenderMode::PATH_TRACED ?
                         RenderMode::RASTERIZED :
                         RenderMode::PATH_TRACED;
    resetAccumulation();
}

GLuint Renderer::getTextureID(int type) {
    switch (type) {
        case MyCs::COMPUTE_TEXTURE:
            return m_compute_texture.getID();
        case MyCs::POST_PROCESS_TEXTURE:
            return m_post_process_texture.getID();
        case MyCs::ACCUMULATE_TEXTURE:
            return m_ping ? m_ping2_texture.getID() : m_ping1_texture.getID();
        default:
            std::cerr << "Invalid texture type requested.\n";
            return 0;
    }
}

GLuint Renderer::getDisplayTextureID() const {
    return m_post_process_texture.getID();
}

void Renderer::setComputeShaderUniforms(const Camera& camera) {
	// Resolution can change while the application is running.
    m_compute_program.setUniform("resolution", m_render_width, m_render_height);

    // choose appropriate values depending on rendering type
    int effective_reflections = m_render_mode == RenderMode::RASTERIZED ? 2 : m_uniforms.max_reflections;
    int effective_samples     = m_render_mode == RenderMode::RASTERIZED ? 1 : m_uniforms.samples_per_pixel;


	// Set per-frame uniforms
    m_compute_program.setUniform("totalFrameCount", m_uniforms.total_frame_count);
    m_compute_program.setUniform("russianRoulette", m_uniforms.russian_roulette);
    m_compute_program.setUniform("maxReflections", effective_reflections);
    m_compute_program.setUniform("samplesPerPixel", effective_samples);
	// Set camera uniforms
	m_compute_program.setUniform("cameraPos", camera.getPosition());
	m_compute_program.setUniform("cameraRot", camera.getRotation());
	m_compute_program.setUniform("cameraFocalLength", camera.getFocalLength());
	m_compute_program.setUniform("cameraApertureRadius", camera.getApertureRadius());
	m_compute_program.setUniform("cameraFocusDistance", camera.getFocusDistance());
	// Environment controls are uniforms rather than texture changes, but they
	// still restart accumulation through their setters.
	m_compute_program.setUniform("clampSkybox", m_uniforms.clamp_skybox);
	m_compute_program.setUniform("skyboxClampValue", m_uniforms.skybox_clamp_value);
}

void Renderer::runComputeShaderPass(const Camera& camera) {
    m_compute_program.use();
    // set compute shader uniforms
    setComputeShaderUniforms(camera);
    // bind texture as render target
    m_compute_texture.bindImageTexture();

    glBindBufferBase(
        GL_SHADER_STORAGE_BUFFER,
        1,
        m_gpu_scene.getSSBOBuffer(GPUscene::BufferType::TRIANGLE_SSBO)
    );

    glBindBufferBase(
        GL_SHADER_STORAGE_BUFFER,
        2,
        m_gpu_scene.getSSBOBuffer(GPUscene::BufferType::BVH_SSBO)
    );

    glBindBufferBase(
        GL_SHADER_STORAGE_BUFFER,
        3,
        m_gpu_scene.getSSBOBuffer(GPUscene::BufferType::BVH_TRIANGLE_INDEX_SSBO)
    );

    glBindBufferBase(
        GL_SHADER_STORAGE_BUFFER,
        4,
        m_gpu_scene.getSSBOBuffer(GPUscene::BufferType::MATERIAL_SSBO)
    );

    m_skybox.bind(5);

    GLuint groups_x = static_cast<GLuint>(std::ceil(m_render_width / 16.0f));
    GLuint groups_y = static_cast<GLuint>(std::ceil(m_render_height / 16.0f));
    glDispatchCompute(groups_x, groups_y, 1);

    glMemoryBarrier(GL_ALL_BARRIER_BITS);

	m_uniforms.accumulated_frame_count++;
}

void Renderer::runAccumulationPass() {
    Framebuffer& outputFBO = m_ping ? m_ping1_fbo : m_ping2_fbo;
    Texture& inputTex = m_ping ? m_ping2_texture : m_ping1_texture;

    outputFBO.bind();
    //updateFrame();

    m_accumulation_shader_program.use();
    m_accumulation_shader_program.setUniform("accumulatedFrameCount", m_uniforms.accumulated_frame_count);
    m_accumulation_shader_program.setUniform("computeTex", 0);
    m_accumulation_shader_program.setUniform("accumTex", 1);

    m_compute_texture.bind(0);
    inputTex.bind(1);

    m_rect_vao.bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    outputFBO.unbind();

    m_ping = !m_ping;
}

void Renderer::runPostProcessPass(GLuint input_texture) {
    m_post_process_fbo.bind();

    m_post_process_shader_program.use();

    m_post_process_shader_program.setUniform("inputTex", 0);
    m_post_process_shader_program.setUniform("exposure", m_uniforms.post_exposure);
    m_post_process_shader_program.setUniform("contrast", m_uniforms.post_contrast);
    m_post_process_shader_program.setUniform("saturation", m_uniforms.post_saturation);
    m_post_process_shader_program.setUniform(
        "useAcesToneMapping",
        static_cast<int>(m_uniforms.use_aces_tone_mapping)
    );

    glBindTextureUnit(0, input_texture);

    m_rect_vao.bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    m_post_process_fbo.unbind();
}
