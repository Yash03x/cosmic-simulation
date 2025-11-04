cmake_minimum_required(VERSION 3.20)
project(CosmicSimulator VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# macOS-specific settings
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS version")
    set(OpenGL_GL_PREFERENCE LEGACY)
endif()

# Find packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(glm REQUIRED)

# GLAD library
add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

# stb_image (header-only)
add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE external/stb)

# ImGui library
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw glad)

# Core physics library (Schwarzschild and Kerr metrics, geodesic integration)
add_library(cosmic_physics STATIC
    src/physics/Schwarzschild.cpp
    src/physics/Kerr.cpp
    src/physics/Geodesic.cpp
)
target_include_directories(cosmic_physics PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(cosmic_physics PUBLIC Eigen3::Eigen)

# Source files
set(SOURCES
    src/main.cpp
    src/core/Application.cpp
    src/core/Window.cpp
    src/core/UI.cpp
    src/core/AdaptiveStepController.cpp
    src/rendering/Camera.cpp
    src/rendering/Shader.cpp
    src/rendering/Renderer.cpp
)

# Header files
set(HEADERS
    include/core/Application.hpp
    include/core/Window.hpp
    include/core/UI.hpp
    include/rendering/Camera.hpp
    include/rendering/Shader.hpp
    include/rendering/Renderer.hpp
    include/physics/Metric.hpp
    include/physics/Schwarzschild.hpp
    include/physics/Kerr.hpp
    include/physics/Geodesic.hpp
    include/utils/Constants.hpp
)

# Main executable
add_executable(CosmicSimulator ${SOURCES} ${HEADERS})

target_include_directories(CosmicSimulator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OPENGL_INCLUDE_DIR}
)

target_link_libraries(CosmicSimulator PRIVATE
    OpenGL::GL
    glfw
    glad
    Eigen3::Eigen
    glm::glm
    stb_image
    imgui
    cosmic_physics
)

# macOS-specific frameworks
if(APPLE)
    target_link_libraries(CosmicSimulator PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
endif()

# Copy shaders and assets to build directory
add_custom_command(TARGET CosmicSimulator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:CosmicSimulator>/shaders
)

add_custom_command(TARGET CosmicSimulator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:CosmicSimulator>/assets
)

# Compiler warnings
if(MSVC)
    target_compile_options(CosmicSimulator PRIVATE /W4)
else()
    target_compile_options(CosmicSimulator PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Example programs
add_executable(kerr_example examples/kerr_example.cpp)
target_include_directories(kerr_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(kerr_example PRIVATE cosmic_physics Eigen3::Eigen)
