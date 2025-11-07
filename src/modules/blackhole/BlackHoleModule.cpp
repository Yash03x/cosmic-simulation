#include "modules/blackhole/BlackHoleModule.hpp"
#include "physics/Schwarzschild.hpp"
#include "physics/Kerr.hpp"
#include <GLFW/glfw3.h>
#include <iostream>

namespace cosmic {
namespace modules {
namespace blackhole {

BlackHoleModule::BlackHoleModule()
    : window_(nullptr),
      initialized_(false),
      simulationTime_(0.0f) {
}

BlackHoleModule::~BlackHoleModule() {
    cleanup();
}

bool BlackHoleModule::initialize(GLFWwindow* window) {
    window_ = window;

    std::cout << "BlackHoleModule: Initializing...\n";

    // Create UI
    ui_ = std::make_unique<core::UI>(window);
    if (!ui_->initialize()) {
        std::cerr << "ERROR::BlackHoleModule::Failed to initialize UI\n";
        return false;
    }

    // Create renderer
    renderer_ = std::make_unique<rendering::Renderer>();
    if (!renderer_->initialize()) {
        std::cerr << "ERROR::BlackHoleModule::Failed to initialize renderer\n";
        return false;
    }

    // Create initial black hole (Schwarzschild with mass 1.0)
    metric_ = std::make_unique<physics::Schwarzschild>(1.0);

    initialized_ = true;
    std::cout << "BlackHoleModule: Initialization complete\n";

    return true;
}

void BlackHoleModule::update(float deltaTime) {
    if (!initialized_) return;

    simulationTime_ += deltaTime;

    // Handle UI changes
    handlePresetChanges();
    handleMetricChanges();

    // Update particle system
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

    // Update binary black hole system (if active)
    auto binaryBH = ui_->getBinaryBlackHole();
    if (binaryBH && binaryBH->isVisible()) {
        binaryBH->update(deltaTime);
    }

    ui_->resetChangeFlags();
}

void BlackHoleModule::render(rendering::Camera& camera, int windowWidth, int windowHeight) {
    if (!initialized_) return;

    // Update comparison mode viewports if enabled
    auto& comparisonMode = ui_->getComparisonMode();
    if (comparisonMode.isEnabled()) {
        comparisonMode.updateViewports(windowWidth, windowHeight);

        // Sync cameras if enabled
        if (comparisonMode.areCamerasSynced()) {
            comparisonMode.getLeftCamera().setPosition(camera.getPosition());
            comparisonMode.getLeftCamera().setFront(camera.getFront());
            comparisonMode.getLeftCamera().setUp(camera.getUp());
            comparisonMode.syncCameras();
        }

        // Render left viewport
        auto leftVP = comparisonMode.getLeftViewport();
        glViewport(leftVP.x, leftVP.y, leftVP.width, leftVP.height);
        renderer_->render(comparisonMode.getLeftCamera(),
                         comparisonMode.getLeftMetric(),
                         simulationTime_,
                         0.016f);  // Approximate deltaTime

        // Render right viewport
        auto rightVP = comparisonMode.getRightViewport();
        glViewport(rightVP.x, rightVP.y, rightVP.width, rightVP.height);
        renderer_->render(comparisonMode.getRightCamera(),
                         comparisonMode.getRightMetric(),
                         simulationTime_,
                         0.016f);

        // Restore full viewport
        glViewport(0, 0, windowWidth, windowHeight);
    } else {
        // Normal single viewport rendering
        renderer_->render(camera, metric_.get(), simulationTime_, 0.016f);
    }
}

void BlackHoleModule::renderUI() {
    if (!initialized_) return;

    // Render all UI panels
    ui_->render(
        renderer_->getCamera(),
        metric_.get(),
        *renderer_,
        0.016f,  // deltaTime
        60.0f,   // fps
        0,       // activePresetNumber
        ""       // activePresetDescription
    );
}

void BlackHoleModule::handleKeyboard(int key, int action) {
    if (!initialized_) return;

    // Handle screenshot (F12 key is handled in Application)
    // Module-specific keyboard handling can go here
}

void BlackHoleModule::cleanup() {
    if (ui_) {
        ui_.reset();
    }
    if (renderer_) {
        renderer_.reset();
    }
    if (metric_) {
        metric_.reset();
    }

    initialized_ = false;
    std::cout << "BlackHoleModule: Cleanup complete\n";
}

void BlackHoleModule::reset() {
    simulationTime_ = 0.0f;

    // Reset particle system
    ui_->getParticleSystem().clear();

    // Reset other systems as needed
    std::cout << "BlackHoleModule: Reset to initial state\n";
}

bool BlackHoleModule::wantsCaptureMouse() const {
    if (!initialized_ || !ui_) return false;
    return ui_->wantsCaptureMouse();
}

bool BlackHoleModule::wantsCaptureKeyboard() const {
    if (!initialized_ || !ui_) return false;
    return ui_->wantsCaptureKeyboard();
}

void BlackHoleModule::handlePresetChanges() {
    if (ui_->hasPresetChanged()) {
        auto preset = ui_->getSelectedPreset();

        // Update mass and spin
        if (ui_->hasBlackHoleMassChanged()) {
            float mass = ui_->getBlackHoleMass();
            float spin = ui_->getSpin();

            if (spin == 0.0f) {
                metric_ = std::make_unique<physics::Schwarzschild>(mass);
            } else {
                metric_ = std::make_unique<physics::Kerr>(mass, spin);
            }
        }
    }
}

void BlackHoleModule::handleMetricChanges() {
    if (ui_->hasMetricTypeChanged() || ui_->hasSpinChanged() || ui_->hasBlackHoleMassChanged()) {
        int metricType = ui_->getMetricType();
        float mass = ui_->getBlackHoleMass();
        float spin = ui_->getSpin();

        if (metricType == 0) {
            // Schwarzschild
            metric_ = std::make_unique<physics::Schwarzschild>(mass);
        } else {
            // Kerr
            metric_ = std::make_unique<physics::Kerr>(mass, spin);
        }
    }
}

} // namespace blackhole
} // namespace modules
} // namespace cosmic
