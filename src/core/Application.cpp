#include "core/Application.hpp"
#include <GLFW/glfw3.h>
#include "physics/Kerr.hpp"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../external/stb/stb_image_write.h"

namespace cosmic {
namespace core {

Application::Application(int width, int height, const std::string& title)
    : deltaTime_(0.0f),
      fps_(0.0f),
      simulationTime_(0.0f),
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

    // Initialize camera presets
    initializePresets();

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

    const bool massChanged = ui_->blackHoleMassChanged();
    const bool metricTypeChanged = ui_->metricTypeChanged();
    const bool spinChanged = ui_->spinChanged();
    const bool presetChanged = ui_->presetChanged();

    renderer_->setMetricType(ui_->getMetricType());
    if (ui_->getMetricType() == 0) {
        renderer_->setSpin(0.0f);
    } else {
        renderer_->setSpin(ui_->getSpin());
    }

    if (massChanged || metricTypeChanged || spinChanged) {
        const float mass = ui_->getBlackHoleMass();
        const int metricType = ui_->getMetricType();

        if (metricType == 0) {
            blackHole_ = std::make_unique<physics::Schwarzschild>(mass);
            renderer_->setSpin(0.0f);
            std::cout << "Switched to Schwarzschild metric (M=" << mass << ")\n";
        } else {
            const float spin = ui_->getSpin();
            blackHole_ = std::make_unique<physics::Kerr>(mass, spin);
            std::cout << "Switched to Kerr metric (M=" << mass
                      << ", a/M=" << spin << ")\n";
        }
    }

    // Handle black hole preset changes
    if (presetChanged) {
        const auto& preset = ui_->getSelectedPreset();

        // Update camera to preset's suggested position
        camera_->setPosition(glm::vec3(0.0f, preset.cameraHeight, preset.cameraDistance));
        camera_->setYaw(-90.0f);
        camera_->setPitch(-15.0f);

        std::cout << "\n✓ Loaded Black Hole Preset: " << preset.name << "\n";
        std::cout << "  " << preset.description << "\n";
        std::cout << "  Mass: " << preset.mass << " M☉\n";
        std::cout << "  Spin: " << preset.spin << "\n";
        if (preset.hasJets) {
            std::cout << "  Features: Relativistic jets, accretion disk\n";
        }
        std::cout << "\n";
    }

    // Update particle trajectory system
    ui_->getParticleSystem().update(deltaTime);

    // Update neutron star (if active)
    auto neutronStar = ui_->getNeutronStar();
    if (neutronStar && neutronStar->isVisible()) {
        neutronStar->update(deltaTime);
    }

    // Update pulsar (if active)
    auto pulsar = ui_->getPulsar();
    if (pulsar && pulsar->isVisible()) {
        pulsar->update(deltaTime);
    }

    // Update gravitational wave system
    ui_->getGravitationalWaveSystem().update(deltaTime);

    ui_->resetChangeFlags();
}

void Application::render() {
    // Render scene
    renderer_->render(*camera_, blackHole_.get(), simulationTime_, deltaTime_);

    // Render UI on top
    ui_->newFrame();
    ui_->render(*camera_, blackHole_.get(), *renderer_, deltaTime_, fps_,
                activePresetNumber_, getActivePresetDescription());
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
    static bool tabWasPressed = false;
    if (window_->isKeyPressed(GLFW_KEY_TAB) && !ui_->wantsCaptureMouse()) {
        if (!tabWasPressed) {
            cursorCaptured_ = !cursorCaptured_;
            window_->setCursorVisible(!cursorCaptured_);
            firstMouse_ = true;
            tabWasPressed = true;
        }
    } else {
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

    // Clear active preset if user moves camera manually
    if (forward != 0.0f || right != 0.0f || up != 0.0f) {
        activePresetNumber_ = 0;
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

        // Clear active preset if user looks around
        if (xOffset != 0.0f || yOffset != 0.0f) {
            activePresetNumber_ = 0;
        }

        camera_->processMouseMovement(xOffset, yOffset);
    }

    // Camera presets (number keys 1-9)
    for (int i = 1; i <= 9; i++) {
        static bool numberKeyWasPressed[9] = {false};
        int keyCode = GLFW_KEY_1 + (i - 1);

        if (window_->isKeyPressed(keyCode)) {
            if (!numberKeyWasPressed[i-1]) {
                loadCameraPreset(i);
                numberKeyWasPressed[i-1] = true;
            }
        } else {
            numberKeyWasPressed[i-1] = false;
        }
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

    // Screenshot with F12 key
    static bool f12KeyWasPressed = false;
    if (window_->isKeyPressed(GLFW_KEY_F12)) {
        if (!f12KeyWasPressed) {
            saveScreenshot();
            f12KeyWasPressed = true;
        }
    } else {
        f12KeyWasPressed = false;
    }
}

void Application::updateTiming() {
    auto currentTime = Clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        currentTime - lastFrameTime_);
    deltaTime_ = duration.count() / 1000000.0f;  // Convert to seconds
    fps_ = 1.0f / deltaTime_;

    // Apply time scale and pause from UI
    float effectiveDeltaTime = deltaTime_;
    if (ui_) {
        if (ui_->isPaused()) {
            effectiveDeltaTime = 0.0f;
        } else {
            effectiveDeltaTime *= ui_->getTimeScale();
        }
    }

    simulationTime_ += effectiveDeltaTime;
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
    std::cout << "  H          - Toggle UI\n";
    std::cout << "  P          - Print statistics\n";
    std::cout << "  F12        - Save screenshot\n";
    std::cout << "  1-9        - Camera presets (instant teleport!)\n";
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

void Application::initializePresets() {
    cameraPresets_.clear();

    // Preset 1: Default - Safe distance, front view
    cameraPresets_.push_back({glm::vec3(0.0f, 0.0f, 20.0f), -90.0f, 0.0f, "Default View (Safe Distance)"});

    // Preset 2: Close-up - Near photon sphere
    cameraPresets_.push_back({glm::vec3(0.0f, 0.0f, 4.0f), -90.0f, 0.0f, "Close-up (Near Photon Sphere)"});

    // Preset 3: Side view - From the side
    cameraPresets_.push_back({glm::vec3(15.0f, 0.0f, 0.0f), 0.0f, 0.0f, "Side View"});

    // Preset 4: Top-down - Looking down at disk
    cameraPresets_.push_back({glm::vec3(0.0f, 15.0f, 0.0f), -90.0f, -45.0f, "Top-Down (Disk View)"});

    // Preset 5: Edge-on disk view
    cameraPresets_.push_back({glm::vec3(0.0f, 0.2f, 12.0f), -90.0f, -2.0f, "Edge-On Disk"});

    // Preset 6: Diagonal approach
    cameraPresets_.push_back({glm::vec3(10.0f, 10.0f, 10.0f), -135.0f, -30.0f, "Diagonal View"});

    // Preset 7: Far orbit
    cameraPresets_.push_back({glm::vec3(30.0f, 0.0f, 0.0f), 0.0f, 0.0f, "Far Orbit"});

    // Preset 8: Danger zone - Very close!
    cameraPresets_.push_back({glm::vec3(0.0f, 0.0f, 2.5f), -90.0f, 0.0f, "DANGER! (Event Horizon)"});

    // Preset 9: Above disk
    cameraPresets_.push_back({glm::vec3(0.0f, 8.0f, 0.0f), -90.0f, -90.0f, "Above Disk (Bird's Eye)"});

    std::cout << "Camera presets initialized (" << cameraPresets_.size() << " presets)\n";
}

bool Application::saveScreenshot() {
    // Get current window dimensions
    int width = window_->getWidth();
    int height = window_->getHeight();

    // Allocate buffer for pixel data (RGB, 3 bytes per pixel)
    std::vector<unsigned char> pixels(width * height * 3);

    // Read pixels from framebuffer
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Flip image vertically (OpenGL has origin at bottom-left)
    std::vector<unsigned char> flipped(width * height * 3);
    for (int y = 0; y < height; y++) {
        memcpy(&flipped[y * width * 3],
               &pixels[(height - 1 - y) * width * 3],
               width * 3);
    }

    // Generate timestamped filename
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream oss;
    oss << "screenshot_"
        << std::put_time(&tm, "%Y%m%d_%H%M%S")
        << ".png";
    std::string filename = oss.str();

    // Save as PNG
    int result = stbi_write_png(filename.c_str(), width, height, 3,
                                 flipped.data(), width * 3);

    if (result) {
        std::cout << "\n✓ Screenshot saved: " << filename << " (" << width << "x" << height << ")\n\n";
        return true;
    } else {
        std::cerr << "\n✗ Failed to save screenshot: " << filename << "\n\n";
        return false;
    }
}

void Application::loadCameraPreset(int presetNumber) {
    if (presetNumber < 1 || presetNumber > static_cast<int>(cameraPresets_.size())) {
        std::cout << "Invalid preset number: " << presetNumber << "\n";
        return;
    }

    const auto& preset = cameraPresets_[presetNumber - 1];
    camera_->setPosition(preset.position);
    camera_->setYaw(preset.yaw);
    camera_->setPitch(preset.pitch);
    activePresetNumber_ = presetNumber;

    std::cout << "\n✓ Loaded Preset " << presetNumber << ": " << preset.description << "\n";
    std::cout << "  Position: (" << preset.position.x << ", "
              << preset.position.y << ", " << preset.position.z << ")\n";
    std::cout << "  Orientation: Yaw=" << preset.yaw << "°, Pitch=" << preset.pitch << "°\n\n";
}

} // namespace core
} // namespace cosmic
