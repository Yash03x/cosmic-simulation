#pragma once

#include "../rendering/Camera.hpp"
#include <string>
#include <memory>

// Forward declarations
struct GLFWwindow;

namespace cosmic {
namespace core {

/**
 * @brief Base interface for simulation modules
 *
 * Each cosmic phenomenon (black holes, galaxies, solar systems, etc.)
 * implements this interface to integrate with the application framework.
 */
class Module {
public:
    virtual ~Module() = default;

    /**
     * @brief Get module name
     */
    virtual std::string getName() const = 0;

    /**
     * @brief Get module description
     */
    virtual std::string getDescription() const = 0;

    /**
     * @brief Initialize module
     * @param window GLFW window handle
     * @return True if successful
     */
    virtual bool initialize(GLFWwindow* window) = 0;

    /**
     * @brief Update module state
     * @param deltaTime Time since last update (seconds)
     */
    virtual void update(float deltaTime) = 0;

    /**
     * @brief Render module
     * @param camera Camera for rendering
     * @param windowWidth Window width
     * @param windowHeight Window height
     */
    virtual void render(rendering::Camera& camera, int windowWidth, int windowHeight) = 0;

    /**
     * @brief Render module UI
     * Must be called between ImGui::NewFrame() and ImGui::Render()
     */
    virtual void renderUI() = 0;

    /**
     * @brief Handle keyboard input
     * @param key GLFW key code
     * @param action GLFW action (press, release, repeat)
     */
    virtual void handleKeyboard(int key, int action) {}

    /**
     * @brief Handle mouse input
     * @param button Mouse button
     * @param action GLFW action
     */
    virtual void handleMouse(int button, int action) {}

    /**
     * @brief Cleanup module resources
     */
    virtual void cleanup() = 0;

    /**
     * @brief Check if module wants to capture mouse input
     */
    virtual bool wantsCaptureMouse() const { return false; }

    /**
     * @brief Check if module wants to capture keyboard input
     */
    virtual bool wantsCaptureKeyboard() const { return false; }

    /**
     * @brief Reset module to initial state
     */
    virtual void reset() {}

    /**
     * @brief Get module category for organization
     * Examples: "Compact Objects", "Galaxies", "Stellar", "Cosmology"
     */
    virtual std::string getCategory() const = 0;
};

} // namespace core
} // namespace cosmic
