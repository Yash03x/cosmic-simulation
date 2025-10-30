#include "core/Application.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <iomanip>

namespace cosmic {
namespace core {

Application::Application(int width, int height, const std::string& title)
    : deltaTime_(0.0f),
      fps_(0.0f),
      firstMouse_(true),
      lastMouseX_(width / 2.0),
      lastMouseY_(height / 2.0),
      cursorCaptured_(false) {

    window_ = std::make_unique<Window>(width, height, title);
    camera_ = std::make_unique<rendering::Camera>(
        glm::vec3(0.0f, 0.0f, 20.0f),  // Start 20 units away from black hole
        45.0f,                          // FOV
        static_cast<float>(width) / static_cast<float>(height)  // Aspect ratio
    );
    renderer_ = std::make_unique<rendering::Renderer>(width, height);

    // Create a Schwarzschild black hole with mass 1.0 (solar masses)
    blackHole_ = std::make_unique<physics::Schwarzschild>(1.0);

    // UI will be initialized after window creation
}

Application::~Application() {
    shutdown();
}

bool Application::initialize() {
    std::cout << "=== Cosmic Simulator - Black Hole Visualization ===\n\n";

    // Initialize window
    if (!window_->initialize()) {
        std::cerr << "ERROR::APPLICATION::WINDOW_INITIALIZATION_FAILED\n";
        return false;
    }

    // Initialize UI
    ui_ = std::make_unique<UI>(window_->getHandle());
    if (!ui_->initialize()) {
        std::cerr << "ERROR::APPLICATION::UI_INITIALIZATION_FAILED\n";
        return false;
    }

    // Initialize renderer
    if (!renderer_->initialize()) {
        std::cerr << "ERROR::APPLICATION::RENDERER_INITIALIZATION_FAILED\n";
        return false;
    }

    // Set window callbacks
    window_->setFramebufferSizeCallback(
        [this](int width, int height) {
            onWindowResize(width, height);
        }
    );

    // Enable VSync by default
    window_->setVSync(true);

    // Print controls
    printControls();

    // Initialize timing
    lastFrameTime_ = Clock::now();

    std::cout << "\nInitialization complete!\n\n";

    return true;
}

void Application::run() {
    std::cout << "Starting main loop...\n\n";

    while (!window_->shouldClose()) {
        updateTiming();
        processInput(deltaTime_);
        update(deltaTime_);
        render();
        window_->swapBuffers();
        window_->pollEvents();
    }

    std::cout << "\nExiting application...\n";
}

void Application::shutdown() {
    // Cleanup is handled by unique_ptr destructors
}

void Application::update(float deltaTime) {
    camera_->update(deltaTime);

    // Check if black hole mass was changed in UI
    if (ui_->blackHoleMassChanged()) {
        blackHole_ = std::make_unique<physics::Schwarzschild>(ui_->getBlackHoleMass());
        std::cout << "Black hole mass changed to " << ui_->getBlackHoleMass() << " M☉\n";
    }
    ui_->resetChangeFlags();
}

void Application::render() {
    // Render scene
    renderer_->render(*camera_, blackHole_.get());

    // Render UI on top
    ui_->newFrame();
    ui_->render(*camera_, blackHole_.get(), *renderer_, deltaTime_, fps_);
    ui_->endFrame();
}

void Application::processInput(float deltaTime) {
    // Exit on ESC
    if (window_->isKeyPressed(GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window_->getHandle(), true);
        return;
    }

    // Toggle UI with H key
    static bool hKeyWasPressed = false;
    if (window_->isKeyPressed(GLFW_KEY_H)) {
        if (!hKeyWasPressed) {
            ui_->toggleVisibility();
            hKeyWasPressed = true;
        }
    } else {
        hKeyWasPressed = false;
    }

    // Don't process camera input if UI is capturing keyboard
    if (ui_->wantsCaptureKeyboard()) {
        return;
    }

    // Toggle cursor capture with TAB (only if UI is not capturing mouse)
    if (window_->isKeyPressed(GLFW_KEY_TAB) && !ui_->wantsCaptureMouse()) {
        static bool tabWasPressed = false;
        if (!tabWasPressed) {
            cursorCaptured_ = !cursorCaptured_;
            window_->setCursorVisible(!cursorCaptured_);
            firstMouse_ = true;
            tabWasPressed = true;
        }
    } else {
        static bool tabWasPressed = false;
        tabWasPressed = false;
    }

    // Movement (WASD + QE)
    float forward = 0.0f;
    float right = 0.0f;
    float up = 0.0f;

    if (window_->isKeyPressed(GLFW_KEY_W)) forward += 1.0f;
    if (window_->isKeyPressed(GLFW_KEY_S)) forward -= 1.0f;
    if (window_->isKeyPressed(GLFW_KEY_D)) right += 1.0f;
    if (window_->isKeyPressed(GLFW_KEY_A)) right -= 1.0f;
    if (window_->isKeyPressed(GLFW_KEY_E)) up += 1.0f;
    if (window_->isKeyPressed(GLFW_KEY_Q)) up -= 1.0f;

    // Speed boost with SHIFT
    if (window_->isKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
        camera_->setMovementSpeed(20.0f);
    } else {
        camera_->setMovementSpeed(10.0f);
    }

    camera_->processMovement(forward, right, up, deltaTime);

    // Mouse look (if cursor is captured and UI is not capturing)
    if (cursorCaptured_ && !ui_->wantsCaptureMouse()) {
        double mouseX, mouseY;
        window_->getMousePosition(mouseX, mouseY);

        if (firstMouse_) {
            lastMouseX_ = mouseX;
            lastMouseY_ = mouseY;
            firstMouse_ = false;
        }

        float xOffset = static_cast<float>(mouseX - lastMouseX_);
        float yOffset = static_cast<float>(lastMouseY_ - mouseY);  // Reversed

        lastMouseX_ = mouseX;
        lastMouseY_ = mouseY;

        camera_->processMouseMovement(xOffset, yOffset);
    }

    // Print stats with P key
    static bool pKeyWasPressed = false;
    if (window_->isKeyPressed(GLFW_KEY_P)) {
        if (!pKeyWasPressed) {
            printStats();
            pKeyWasPressed = true;
        }
    } else {
        pKeyWasPressed = false;
    }
}

void Application::updateTiming() {
    auto currentTime = Clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        currentTime - lastFrameTime_);
    deltaTime_ = duration.count() / 1000000.0f;  // Convert to seconds
    fps_ = 1.0f / deltaTime_;
    lastFrameTime_ = currentTime;
}

void Application::onWindowResize(int width, int height) {
    renderer_->setResolution(width, height);
    camera_->setAspect(static_cast<float>(width) / static_cast<float>(height));
    std::cout << "Window resized to " << width << "x" << height << "\n";
}

void Application::printControls() {
    std::cout << "=== Controls ===\n";
    std::cout << "  WASD       - Move camera (forward/left/back/right)\n";
    std::cout << "  Q/E        - Move camera down/up\n";
    std::cout << "  Mouse      - Look around (press TAB to capture/release)\n";
    std::cout << "  SHIFT      - Speed boost\n";
    std::cout << "  TAB        - Toggle mouse capture\n";
    std::cout << "  P          - Print statistics\n";
    std::cout << "  ESC        - Exit\n";
    std::cout << "\n";
}

void Application::printStats() {
    std::cout << "\n=== Statistics ===\n";
    std::cout << "  FPS: " << std::fixed << std::setprecision(1) << fps_ << "\n";
    std::cout << "  Frame time: " << std::fixed << std::setprecision(2)
              << (deltaTime_ * 1000.0f) << " ms\n";

    glm::vec3 pos = camera_->getPosition();
    std::cout << "  Camera position: ("
              << std::fixed << std::setprecision(2)
              << pos.x << ", " << pos.y << ", " << pos.z << ")\n";

    // Distance from black hole
    float distance = glm::length(pos);
    std::cout << "  Distance from black hole: " << distance << " M\n";

    double eventHorizon = blackHole_->eventHorizonRadius();
    double photonSphere = blackHole_->photonSphereRadius();
    double isco = blackHole_->iscoRadius();

    std::cout << "  Event horizon: " << eventHorizon << " M\n";
    std::cout << "  Photon sphere: " << photonSphere << " M\n";
    std::cout << "  ISCO: " << isco << " M\n";

    if (distance < eventHorizon) {
        std::cout << "  ** WARNING: Inside event horizon! **\n";
    } else if (distance < photonSphere) {
        std::cout << "  ** Inside photon sphere **\n";
    } else if (distance < isco) {
        std::cout << "  ** Inside ISCO **\n";
    }

    std::cout << "\n";
}

} // namespace core
} // namespace cosmic
