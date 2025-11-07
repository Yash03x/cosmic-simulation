#include "core/ComparisonMode.hpp"
#include "physics/Schwarzschild.hpp"
#include "physics/Kerr.hpp"

namespace cosmic {
namespace core {

ComparisonMode::ComparisonMode()
    : enabled_(false),
      syncCameras_(true),
      splitMode_(0),
      leftCamera_(glm::vec3(0, 50, 100)),
      rightCamera_(glm::vec3(0, 50, 100)) {

    // Initialize with default presets (will be set later)
    leftPreset_ = BlackHolePresets::getM87Star();
    rightPreset_ = BlackHolePresets::getSgrAStar();

    // Create metrics
    leftMetric_ = createMetricFromPreset(leftPreset_);
    rightMetric_ = createMetricFromPreset(rightPreset_);
}

void ComparisonMode::setLeftBlackHole(const BlackHolePreset& preset) {
    leftPreset_ = preset;
    leftMetric_ = createMetricFromPreset(preset);
}

void ComparisonMode::setRightBlackHole(const BlackHolePreset& preset) {
    rightPreset_ = preset;
    rightMetric_ = createMetricFromPreset(preset);
}

void ComparisonMode::updateViewports(int windowWidth, int windowHeight) {
    if (splitMode_ == 0) {
        // Vertical split (left/right)
        int halfWidth = windowWidth / 2;

        leftViewport_.x = 0;
        leftViewport_.y = 0;
        leftViewport_.width = halfWidth;
        leftViewport_.height = windowHeight;

        rightViewport_.x = halfWidth;
        rightViewport_.y = 0;
        rightViewport_.width = halfWidth;
        rightViewport_.height = windowHeight;
    } else {
        // Horizontal split (top/bottom)
        int halfHeight = windowHeight / 2;

        leftViewport_.x = 0;
        leftViewport_.y = halfHeight;  // Top viewport
        leftViewport_.width = windowWidth;
        leftViewport_.height = halfHeight;

        rightViewport_.x = 0;
        rightViewport_.y = 0;  // Bottom viewport
        rightViewport_.width = windowWidth;
        rightViewport_.height = halfHeight;
    }

    // Update camera aspect ratios
    float leftAspect = static_cast<float>(leftViewport_.width) / static_cast<float>(leftViewport_.height);
    float rightAspect = static_cast<float>(rightViewport_.width) / static_cast<float>(rightViewport_.height);

    leftCamera_.setAspectRatio(leftAspect);
    rightCamera_.setAspectRatio(rightAspect);
}

void ComparisonMode::syncCameras() {
    if (!syncCameras_) return;

    // Copy left camera transform to right camera
    rightCamera_.setPosition(leftCamera_.getPosition());
    rightCamera_.setFront(leftCamera_.getFront());
    rightCamera_.setUp(leftCamera_.getUp());
}

void ComparisonMode::setSplitMode(int mode) {
    if (mode >= 0 && mode <= 1) {
        splitMode_ = mode;
    }
}

std::unique_ptr<physics::Metric> ComparisonMode::createMetricFromPreset(const BlackHolePreset& preset) {
    if (preset.spin == 0.0) {
        // Schwarzschild metric
        return std::make_unique<physics::Schwarzschild>(preset.mass);
    } else {
        // Kerr metric
        return std::make_unique<physics::Kerr>(preset.mass, preset.spin);
    }
}

} // namespace core
} // namespace cosmic
