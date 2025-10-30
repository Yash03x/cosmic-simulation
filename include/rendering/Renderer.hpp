#pragma once

#include "Shader.hpp"
#include "Camera.hpp"
#include "../physics/Metric.hpp"
#include <memory>

namespace cosmic {
namespace rendering {

/**
 * @brief Main renderer for black hole visualization
 *
 * Handles fullscreen quad rendering and ray tracing in shaders
 */
class Renderer {
public:
    /**
     * @brief Construct renderer
     * @param width Window width
     * @param height Window height
     */
    Renderer(int width, int height);

    /**
     * @brief Destructor - cleanup OpenGL resources
     */
    ~Renderer();

    // Prevent copying
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    /**
     * @brief Initialize renderer (load shaders, setup buffers)
     * @return True if successful
     */
    bool initialize();

    /**
     * @brief Render the scene
     * @param camera Camera for view parameters
     * @param metric Black hole metric for physics
     */
    void render(const Camera& camera, const physics::Metric* metric);

    /**
     * @brief Set render resolution
     * @param width New width
     * @param height New height
     */
    void setResolution(int width, int height);

    /**
     * @brief Set maximum ray tracing steps
     * @param steps Maximum number of integration steps
     */
    void setMaxRaySteps(int steps) { maxRaySteps_ = steps; }

    /**
     * @brief Get maximum ray tracing steps
     * @return Current maximum steps
     */
    int getMaxRaySteps() const { return maxRaySteps_; }

    /**
     * @brief Set integration step size
     * @param size Step size for geodesic integration
     */
    void setStepSize(float size) { stepSize_ = size; }

    /**
     * @brief Get integration step size
     * @return Current step size
     */
    float getStepSize() const { return stepSize_; }

    /**
     * @brief Enable/disable accretion disk rendering
     * @param enable True to enable
     */
    void setAccretionDiskEnabled(bool enable) { accretionDiskEnabled_ = enable; }

    /**
     * @brief Check if accretion disk is enabled
     * @return True if enabled
     */
    bool isAccretionDiskEnabled() const { return accretionDiskEnabled_; }

    /**
     * @brief Set background color
     * @param r Red component (0-1)
     * @param g Green component (0-1)
     * @param b Blue component (0-1)
     */
    void setBackgroundColor(float r, float g, float b);

    /**
     * @brief Get current window width
     * @return Width in pixels
     */
    int getWidth() const { return width_; }

    /**
     * @brief Get current window height
     * @return Height in pixels
     */
    int getHeight() const { return height_; }

private:
    int width_;
    int height_;

    // OpenGL objects
    unsigned int quadVAO_;  // Vertex Array Object for fullscreen quad
    unsigned int quadVBO_;  // Vertex Buffer Object

    // Shader program
    Shader rayTracerShader_;

    // Rendering parameters
    int maxRaySteps_;
    float stepSize_;
    bool accretionDiskEnabled_;
    glm::vec3 backgroundColor_;

    /**
     * @brief Setup fullscreen quad for ray tracing
     */
    void setupQuad();

    /**
     * @brief Cleanup OpenGL resources
     */
    void cleanup();
};

} // namespace rendering
} // namespace cosmic
