#pragma once

#include "Window.hpp"
#include "UI.hpp"
#include "../rendering/Camera.hpp"
#include "../rendering/Renderer.hpp"
#include "../physics/Metric.hpp"
#include "../physics/Schwarzschild.hpp"
#include <memory>
#include <chrono>
#include <vector>
#include <string>

namespace cosmic {
namespace core {

/**
 * @brief Main application class
 *
 * Manages the main loop, window, rendering, and user input
 */
class Application {
public:
    /**
     * @brief Construct application
     * @param width Window width
     * @param height Window height
     * @param title Window title
     */
    Application(int width = 1280,
                int height = 720,
                const std::string& title = "Cosmic Simulator - Black Hole");

    /**
     * @brief Destructor
     */
    ~Application();

    /**
     * @brief Initialize application
     * @return True if successful
     */
    bool initialize();

    /**
     * @brief Run main application loop
     */
    void run();

    /**
     * @brief Shutdown application
     */
    void shutdown();

private:
    // Core components
    std::unique_ptr<Window> window_;
    std::unique_ptr<UI> ui_;
    std::unique_ptr<rendering::Camera> camera_;
    std::unique_ptr<rendering::Renderer> renderer_;
    std::unique_ptr<physics::Metric> blackHole_;

    // Timing
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    TimePoint lastFrameTime_;
    float deltaTime_;
    float fps_;
    float simulationTime_;

    // Input state
    bool firstMouse_;
    double lastMouseX_;
    double lastMouseY_;
    bool cursorCaptured_;

    /**
     * @brief Update application state
     * @param deltaTime Time since last frame
     */
    void update(float deltaTime);

    /**
     * @brief Render frame
     */
    void render();

    /**
     * @brief Process user input
     * @param deltaTime Time since last frame
     */
    void processInput(float deltaTime);

    /**
     * @brief Update timing information
     */
    void updateTiming();

    /**
     * @brief Handle window resize
     * @param width New width
     * @param height New height
     */
    void onWindowResize(int width, int height);

    /**
     * @brief Print controls to console
     */
    void printControls();

    /**
     * @brief Print frame statistics
     */
    void printStats();

    /**
     * @brief Load camera preset by number (1-9)
     * @param presetNumber Preset number
     */
    void loadCameraPreset(int presetNumber);

    /**
     * @brief Initialize camera presets
     */
    void initializePresets();

    // Camera presets
    struct CameraPreset {
        glm::vec3 position;
        float yaw;
        float pitch;
        std::string description;
    };
    std::vector<CameraPreset> cameraPresets_;
    int activePresetNumber_ = 0;  // 0 = no preset, 1-9 = preset number

public:
    /**
     * @brief Get active camera preset number
     * @return Preset number (0 = no preset, 1-9 = preset number)
     */
    int getActivePresetNumber() const { return activePresetNumber_; }

    /**
     * @brief Get active camera preset description
     * @return Description string, or empty if no preset active
     */
    std::string getActivePresetDescription() const {
        if (activePresetNumber_ > 0 && activePresetNumber_ <= static_cast<int>(cameraPresets_.size())) {
            return cameraPresets_[activePresetNumber_ - 1].description;
        }
        return "";
    }
};

} // namespace core
} // namespace cosmic
