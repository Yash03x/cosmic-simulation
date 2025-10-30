#pragma once

#include "../physics/Metric.hpp"
#include "../rendering/Camera.hpp"
#include "../rendering/Renderer.hpp"
#include <memory>

// Forward declarations
struct GLFWwindow;

namespace cosmic {
namespace core {

/**
 * @brief UI manager using ImGui
 *
 * Handles all UI rendering and user interaction with simulation parameters
 */
class UI {
public:
    /**
     * @brief Construct UI manager
     * @param window GLFW window handle
     */
    explicit UI(GLFWwindow* window);

    /**
     * @brief Destructor - cleanup ImGui
     */
    ~UI();

    // Prevent copying
    UI(const UI&) = delete;
    UI& operator=(const UI&) = delete;

    /**
     * @brief Initialize ImGui
     * @return True if successful
     */
    bool initialize();

    /**
     * @brief Begin new frame
     */
    void newFrame();

    /**
     * @brief Render all UI elements
     * @param camera Camera reference for displaying stats
     * @param metric Black hole metric
     * @param renderer Renderer for parameter control
     * @param deltaTime Frame time
     * @param fps Frames per second
     * @param activePresetNumber Active camera preset (0 = none, 1-9 = preset number)
     * @param activePresetDescription Description of active preset
     */
    void render(rendering::Camera& camera,
                physics::Metric* metric,
                rendering::Renderer& renderer,
                float deltaTime,
                float fps,
                int activePresetNumber = 0,
                const std::string& activePresetDescription = "");

    /**
     * @brief End frame and render ImGui
     */
    void endFrame();

    /**
     * @brief Check if UI wants to capture mouse
     * @return True if mouse is over UI
     */
    bool wantsCaptureMouse() const;

    /**
     * @brief Check if UI wants to capture keyboard
     * @return True if keyboard focus is on UI
     */
    bool wantsCaptureKeyboard() const;

    /**
     * @brief Toggle UI visibility
     */
    void toggleVisibility() { visible_ = !visible_; }

    /**
     * @brief Set UI visibility
     * @param visible True to show UI
     */
    void setVisible(bool visible) { visible_ = visible; }

    /**
     * @brief Check if UI is visible
     * @return True if visible
     */
    bool isVisible() const { return visible_; }

    /**
     * @brief Get black hole mass (can be modified by UI)
     * @return Current mass setting
     */
    float getBlackHoleMass() const { return blackHoleMass_; }

    /**
     * @brief Check if black hole mass was changed
     * @return True if changed this frame
     */
    bool blackHoleMassChanged() const { return blackHoleMassChanged_; }

    /**
     * @brief Get selected metric type (0 = Schwarzschild, 1 = Kerr)
     */
    int getMetricType() const { return metricType_; }

    /**
     * @brief Check if metric type changed this frame
     */
    bool metricTypeChanged() const { return metricTypeChanged_; }

    /**
     * @brief Get spin parameter a/M
     */
    float getSpin() const { return spin_; }

    /**
     * @brief Check if spin changed this frame
     */
    bool spinChanged() const { return spinChanged_; }

    /**
     * @brief Reset changed flags
     */
    void resetChangeFlags() {
        blackHoleMassChanged_ = false;
        metricTypeChanged_ = false;
        spinChanged_ = false;
    }

private:
    GLFWwindow* window_;
    bool visible_;

    // UI state
    float blackHoleMass_;
    bool blackHoleMassChanged_;
    int metricType_;
    bool metricTypeChanged_;
    float spin_;
    bool spinChanged_;

    int maxRaySteps_;
    float stepSize_;
    bool accretionDiskEnabled_;
    float cameraSpeed_;

    // Accretion disk physics parameters
    float accretionRate_;
    float alphaViscosity_;
    float diskInclination_;
    float scaleHeightRatio_;

    // Demo window
    bool showDemoWindow_;

    /**
     * @brief Render main control panel
     */
    void renderControlPanel(rendering::Camera& camera,
                           physics::Metric* metric,
                           rendering::Renderer& renderer);

    /**
     * @brief Render statistics panel
     */
    void renderStatsPanel(rendering::Camera& camera,
                         physics::Metric* metric,
                         float deltaTime,
                         float fps,
                         int activePresetNumber,
                         const std::string& activePresetDescription);

    /**
     * @brief Render about panel
     */
    void renderAboutPanel();

    /**
     * @brief Cleanup ImGui resources
     */
    void cleanup();
};

} // namespace core
} // namespace cosmic
