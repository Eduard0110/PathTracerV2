# Use static runtime (MSVC only)
if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# ImGui sources
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/include/imgui)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/imgui_impl_opengl3.cpp
)


# source files
set(SOURCE_FILES
    app/Main.cpp
    app/App.cpp

    renderer/ComputeProgram.cpp
    renderer/ShaderProgram.cpp
    renderer/Texture.cpp
    renderer/RectVAO.cpp
    renderer/Framebuffer.cpp
    renderer/BaseShaderProgram.cpp
    renderer/Renderer.cpp
    renderer/GPUscene.cpp
    renderer/RasterRenderer.cpp

    ui/MyGui.cpp

    camera/Camera.cpp

    core/Diagnostics.cpp
    core/Paths.cpp

    input/InputManager.cpp
    input/CameraController.cpp

    scene/Mesh.cpp
    scene/BVH.cpp
    scene/Scene.cpp
    scene/Skybox.cpp
    scene/StandardMaterials.cpp
    

    ${CMAKE_SOURCE_DIR}/include/stb/stb_image_write.cpp
    ${CMAKE_SOURCE_DIR}/include/glad/glad.c
    ${IMGUI_SOURCES}
)

# create executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# link glfw
target_link_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/glfw
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw3_mt
    opengl32
    user32
    gdi32
    shell32
)

# copy the shaders directory after build
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/shaders"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders"
)

# Copy only assets used by the application. Rendered PNG exports and other
# files placed at the root of resources/ are not runtime dependencies.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/resources/3D_models"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources/3D_models"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/resources/skybox_images"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources/skybox_images"
)

# Creates a clean redistributable folder with:
# PathTracer.exe, shaders/, and resources/.
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/shaders" DESTINATION .)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/resources/3D_models" DESTINATION resources)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/resources/skybox_images" DESTINATION resources)
