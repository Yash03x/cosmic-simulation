#pragma once

#include "Window.hpp"
#include "UI.hpp"
#include "../rendering/Camera.hpp"
#include "../rendering/Renderer.hpp"
#include "../physics/Metric.hpp"
#include "../physics/Schwarzschild.hpp"
#include <memory>
#include <chrono>

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
};

} // namespace core
} // namespace cosmic
