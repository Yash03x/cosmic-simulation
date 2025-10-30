#include "core/Window.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

namespace cosmic {
namespace core {

Window::Window(int width, int height, const std::string& title)
    : width_(width),
      height_(height),
      title_(title),
      window_(nullptr) {
}

Window::~Window() {
    cleanup();
}

bool Window::initialize() {
    // Initialize GLFW
    if (!initializeGLFW()) {
        return false;
    }

    // Configure GLFW for OpenGL 4.1 Core (macOS maximum)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create window
    window_ = glfwCreateWindow(width_, height_, title_.c_str(), nullptr, nullptr);
    if (!window_) {
        std::cerr << "ERROR::WINDOW::GLFW_WINDOW_CREATION_FAILED\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window_);

    // Load OpenGL function pointers with GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "ERROR::WINDOW::GLAD_INITIALIZATION_FAILED\n";
        return false;
    }

    // Set window user pointer (for callbacks)
    glfwSetWindowUserPointer(window_, this);

    // Set framebuffer size callback
    glfwSetFramebufferSizeCallback(window_, framebufferSizeCallbackStatic);

    // Print OpenGL information
    std::cout << "OpenGL initialized successfully\n";
    std::cout << "  Vendor: " << glGetString(GL_VENDOR) << "\n";
    std::cout << "  Renderer: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "  Version: " << glGetString(GL_VERSION) << "\n";
    std::cout << "  GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";

    return true;
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(window_);
}

void Window::swapBuffers() {
    glfwSwapBuffers(window_);
}

void Window::pollEvents() {
    glfwPollEvents();
}

bool Window::isKeyPressed(int key) const {
    return glfwGetKey(window_, key) == GLFW_PRESS;
}

bool Window::isMouseButtonPressed(int button) const {
    return glfwGetMouseButton(window_, button) == GLFW_PRESS;
}

void Window::getMousePosition(double& x, double& y) const {
    glfwGetCursorPos(window_, &x, &y);
}

void Window::setCursorVisible(bool visible) {
    glfwSetInputMode(window_, GLFW_CURSOR,
                     visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

void Window::setVSync(bool enabled) {
    glfwSwapInterval(enabled ? 1 : 0);
}

void Window::setFramebufferSizeCallback(FramebufferSizeCallback callback) {
    framebufferSizeCallback_ = callback;
}

bool Window::initializeGLFW() {
    static bool glfwInitialized = false;
    if (!glfwInitialized) {
        if (!glfwInit()) {
            std::cerr << "ERROR::WINDOW::GLFW_INITIALIZATION_FAILED\n";
            return false;
        }
        glfwInitialized = true;

        // Set error callback
        glfwSetErrorCallback([](int error, const char* description) {
            std::cerr << "GLFW Error (" << error << "): " << description << "\n";
        });
    }
    return true;
}

void Window::framebufferSizeCallbackStatic(GLFWwindow* window, int width, int height) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win) {
        win->width_ = width;
        win->height_ = height;
        glViewport(0, 0, width, height);

        if (win->framebufferSizeCallback_) {
            win->framebufferSizeCallback_(width, height);
        }
    }
}

void Window::cleanup() {
    if (window_) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
    // Note: We don't call glfwTerminate() here as it's a global resource
    // that other windows might be using
}

} // namespace core
} // namespace cosmic
