#include "App.hpp"
#include <math.h>

static constexpr const char* COMPUTE_SHADER_PATH  = "C:/Users/golon/source/repos/PathTracer/shaders/ComputeShader.comp";
static constexpr const char* VERTEX_SHADER_PATH   = "C:/Users/golon/source/repos/PathTracer/shaders/VertexShader.vs";
static constexpr const char* FRAGMENT_SHADER_PATH = "C:/Users/golon/source/repos/PathTracer/shaders/FragmentShader.fs";

// construcotr / destructor

App::App()  : m_compute_texture(m_WINDOW_WIDTH, m_WINDOW_HEIGHT),
              m_ping1_texture(m_WINDOW_WIDTH, m_WINDOW_HEIGHT),
              m_ping2_texture(m_WINDOW_WIDTH, m_WINDOW_HEIGHT),
              m_post_proccess_texture(m_WINDOW_WIDTH, m_WINDOW_HEIGHT)
{
    initGLFW();
    createWindow();
    initGLAD();
    m_my_gui.initImGui(m_window);

    m_compute_program.createAndCompile(COMPUTE_SHADER_PATH);
    m_accumulation_shader_program.createAndCompile(VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH);

    m_rect_vao.createVao();

    m_compute_texture.create();
    m_ping1_texture.create();
    m_ping2_texture.create();
    m_post_proccess_texture.create();

    m_ping1_fbo.attachTexture(
        m_ping1_texture.getID(),
        m_WINDOW_WIDTH, m_WINDOW_HEIGHT
    );

    m_ping2_fbo.attachTexture(
        m_ping2_texture.getID(),
        m_WINDOW_WIDTH, m_WINDOW_HEIGHT
    );
    
    m_post_proccess_fbo.attachTexture(
        m_post_proccess_texture.getID(),
        m_WINDOW_WIDTH, m_WINDOW_HEIGHT
    );
}

App::~App() {
    // shutdown ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(m_window);
    glfwTerminate();
}

// initialisation methods
void App::initGLFW() {
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW.\n";
        std::exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

void App::createWindow() {
    m_window = glfwCreateWindow(m_WINDOW_WIDTH, m_WINDOW_HEIGHT, "PathTracer", nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window.\n";
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(0);
}

void App::initGLAD() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD.\n";
        glfwDestroyWindow(m_window);
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }
}

// error callback
void App::glfwErrorCallback(int error, const char* message) {
    std::cerr << "GLFW Error [" << error << "]: " << message << std::endl;
}

// debug info
void App::printDebugInfo() {
    std::cout << "GL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n\n";

    int work_grp_cnt[3], work_grp_size[3], invocations;
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);

    std::cout << "Max work group count: "
        << "x: " << work_grp_cnt[0] << ", "
        << "y: " << work_grp_cnt[1] << ", "
        << "z: " << work_grp_cnt[2] << '\n';

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);

    std::cout << "Max work group size: "
        << "x: " << work_grp_size[0] << ", "
        << "y: " << work_grp_size[1] << ", "
        << "z: " << work_grp_size[2] << '\n';

    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &invocations);
    std::cout << "Max work group invocations: " << invocations << "\n\n";
}

// export image in png
void App::exportImage(GLuint textureID, const int width, const int height, const char* fname) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    unsigned char* pixels = new unsigned char[width * height * 3];
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    // Flip the pixel data vertically
    unsigned char* flippedPixels = new unsigned char[width * height * 3];
    for (int y = 0; y < height; ++y) {
        memcpy(flippedPixels + y * width * 3, pixels + (height - y - 1) * width * 3, width * 3);
    }
    std::string name = "C:/Users/golon/source/repos/PathTracer/resources/" + static_cast<std::string>(fname);
    stbi_write_png(name.c_str(), width, height, 3, flippedPixels, width * 3);

    delete[] pixels;
    delete[] flippedPixels;

    glBindTexture(GL_TEXTURE_2D, 0);
}

// uniforms
void App::setStaticUniforms() {
    m_first_frame = false;
    m_compute_program.setUniform("resolution", m_WINDOW_WIDTH, m_WINDOW_HEIGHT);
}

void App::setPerFrameUniforms() {
    m_uniforms.time = static_cast<float>(glfwGetTime());
    m_compute_program.setUniform("time", m_uniforms.time);
}

// clock
void App::updateDeltaTime() {
    double currentTime = glfwGetTime();
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;
}

void App::updateWindowCaption() const {
    if (static_cast<int>(m_uniforms.frame_count) % 60 == 1) {
        double fps = 1.0 / deltaTime;;

        std::ostringstream title;
        title << "PathTracer - "
            << std::fixed << std::setprecision(1)
            << fps << " FPS | "
            << deltaTime << " ms/frame";

        glfwSetWindowTitle(m_window, title.str().c_str());
    }
}

// main loop functions

void App::processInput() {
    glfwPollEvents();
    m_my_gui.newFrameImGui();
    m_uniforms.frame_count++;
}

void App::updateFrame() {
    glViewport(0, 0, m_WINDOW_WIDTH, m_WINDOW_HEIGHT);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void App::runComputeShaderPass() {
    m_compute_program.use();
    // set compute shader uniforms
    setPerFrameUniforms();
    if (m_first_frame) setStaticUniforms();
    // bind texture as render target
    m_compute_texture.bindImageTexture();

    GLuint groups_x = static_cast<GLuint>(std::ceil(m_WINDOW_WIDTH / 2.0f));
    GLuint groups_y = static_cast<GLuint>(std::ceil(m_WINDOW_HEIGHT / 1.0f));
    glDispatchCompute(groups_x, groups_y, 1);

    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void App::runAccumulationPass() {
    Framebuffer& outputFBO = m_ping ? m_ping1_fbo : m_ping2_fbo;
    Texture& inputTex = m_ping ? m_ping2_texture : m_ping1_texture;

    outputFBO.bind();
    updateFrame();

    m_accumulation_shader_program.use();
    m_accumulation_shader_program.setUniform("framesStill", m_uniforms.frame_count);
    m_accumulation_shader_program.setUniform("computeTex", 0);
    m_accumulation_shader_program.setUniform("accumTex", 1);

    m_compute_texture.bind(0);
    inputTex.bind(1);

    m_rect_vao.bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    outputFBO.unbind();

    m_ping = !m_ping;
}

void App::renderGui() {
    Texture& outputTex = m_ping ? m_ping2_texture : m_ping1_texture;
    m_my_gui.createGui(outputTex.getID(), m_WINDOW_WIDTH, m_WINDOW_HEIGHT);

    if (ImGui::Button("Export")) {
        exportImage(
            m_compute_texture.getID(),
            m_WINDOW_WIDTH, m_WINDOW_HEIGHT,
            "RandomTest.png"
        );
    }

    m_my_gui.render();
}

void App::run() {
    printDebugInfo();
    while (!glfwWindowShouldClose(m_window)) {
        processInput();
        updateFrame();
        runComputeShaderPass();
        runAccumulationPass();
        renderGui();
        updateDeltaTime();
        updateWindowCaption();
        glfwSwapBuffers(m_window);
    }
}