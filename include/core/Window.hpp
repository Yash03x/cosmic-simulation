#pragma once

#include <string>
#include <functional>

// Forward declare GLFW types
struct GLFWwindow;

namespace cosmic {
namespace core {

/**
 * @brief Window management wrapper for GLFW
 *
 * Handles window creation, OpenGL context, and input callbacks
 */
class Window {
public:
    /**
     * @brief Construct window
     * @param width Window width
     * @param height Window height
     * @param title Window title
     */
    Window(int width, int height, const std::string& title);

    /**
     * @brief Destructor - cleanup GLFW resources
     */
    ~Window();

    // Prevent copying
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    /**
     * @brief Initialize window and OpenGL context
     * @return True if successful
     */
    bool initialize();

    /**
     * @brief Check if window should close
     * @return True if close requested
     */
    bool shouldClose() const;

    /**
     * @brief Swap buffers and poll events
     */
    void swapBuffers();

    /**
     * @brief Poll input events
     */
    void pollEvents();

    /**
     * @brief Get window width
     * @return Current width
     */
    int getWidth() const { return width_; }

    /**
     * @brief Get window height
     * @return Current height
     */
    int getHeight() const { return height_; }

    /**
     * @brief Get aspect ratio
     * @return Width / height
     */
    float getAspectRatio() const {
        return static_cast<float>(width_) / static_cast<float>(height_);
    }

    /**
     * @brief Get GLFW window handle
     * @return Raw GLFW window pointer
     */
    GLFWwindow* getHandle() const { return window_; }

    /**
     * @brief Check if key is pressed
     * @param key GLFW key code
     * @return True if pressed
     */
    bool isKeyPressed(int key) const;

    /**
     * @brief Check if mouse button is pressed
     * @param button GLFW mouse button code
     * @return True if pressed
     */
    bool isMouseButtonPressed(int button) const;

    /**
     * @brief Get mouse position
     * @param x Output x coordinate
     * @param y Output y coordinate
     */
    void getMousePosition(double& x, double& y) const;

    /**
     * @brief Set mouse cursor visibility
     * @param visible True to show cursor
     */
    void setCursorVisible(bool visible);

    /**
     * @brief Set VSync enabled
     * @param enabled True to enable VSync
     */
    void setVSync(bool enabled);

    /**
     * @brief Set framebuffer size callback
     * @param callback Function to call when framebuffer is resized
     */
    using FramebufferSizeCallback = std::function<void(int, int)>;
    void setFramebufferSizeCallback(FramebufferSizeCallback callback);

private:
    int width_;
    int height_;
    std::string title_;
    GLFWwindow* window_;

    FramebufferSizeCallback framebufferSizeCallback_;

    /**
     * @brief Initialize GLFW library (called once)
     * @return True if successful
     */
    static bool initializeGLFW();

    /**
     * @brief Static framebuffer size callback for GLFW
     */
    static void framebufferSizeCallbackStatic(GLFWwindow* window, int width, int height);

    /**
     * @brief Cleanup resources
     */
    void cleanup();
};

} // namespace core
} // namespace cosmic
