# Use static runtime (MSVC only)
if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>")
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
    Main.cpp
    App.cpp
    ComputeProgram.cpp
    ShaderProgram.cpp
    Texture.cpp
    RectVAO.cpp
    MyGui.cpp
    Framebuffer.cpp
    BaseShaderProgram.cpp
    Camera.cpp

    ${CMAKE_SOURCE_DIR}/include/stb/stb_image_write.cpp
    ${CMAKE_SOURCE_DIR}/include/glad/glad.c
    ${IMGUI_SOURCES}
)

# create executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES}  "Framebuffer.cpp")

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