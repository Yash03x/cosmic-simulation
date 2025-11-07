#pragma once

#include "../physics/Metric.hpp"
#include "../cosmic/BlackHolePresets.hpp"
#include "../rendering/Camera.hpp"
#include "../rendering/Renderer.hpp"
#include <memory>
#include <glm/glm.hpp>

namespace cosmic {
namespace core {

/**
 * @brief Viewport for side-by-side comparison
 */
struct Viewport {
    int x, y;           // Position in window
    int width, height;  // Dimensions

    glm::vec4 getViewport() const {
        return glm::vec4(x, y, width, height);
    }
};

/**
 * @brief Comparison mode for viewing two black holes side-by-side
 *
 * Allows comparison of different black holes with independent or synchronized cameras
 */
class ComparisonMode {
public:
    /**
     * @brief Construct comparison mode
     */
    ComparisonMode();

    /**
     * @brief Enable/disable comparison mode
     */
    void setEnabled(bool enabled) { enabled_ = enabled; }

    /**
     * @brief Check if comparison mode is enabled
     */
    bool isEnabled() const { return enabled_; }

    /**
     * @brief Set left black hole
     */
    void setLeftBlackHole(const BlackHolePreset& preset);

    /**
     * @brief Set right black hole
     */
    void setRightBlackHole(const BlackHolePreset& preset);

    /**
     * @brief Get left black hole metric
     */
    physics::Metric* getLeftMetric() { return leftMetric_.get(); }
    const physics::Metric* getLeftMetric() const { return leftMetric_.get(); }

    /**
     * @brief Get right black hole metric
     */
    physics::Metric* getRightMetric() { return rightMetric_.get(); }
    const physics::Metric* getRightMetric() const { return rightMetric_.get(); }

    /**
     * @brief Get left viewport
     */
    Viewport getLeftViewport() const { return leftViewport_; }

    /**
     * @brief Get right viewport
     */
    Viewport getRightViewport() const { return rightViewport_; }

    /**
     * @brief Update viewports based on window size
     */
    void updateViewports(int windowWidth, int windowHeight);

    /**
     * @brief Set camera synchronization
     */
    void setSyncCameras(bool sync) { syncCameras_ = sync; }

    /**
     * @brief Check if cameras are synchronized
     */
    bool areCamerasSynced() const { return syncCameras_; }

    /**
     * @brief Get left camera
     */
    rendering::Camera& getLeftCamera() { return leftCamera_; }
    const rendering::Camera& getLeftCamera() const { return leftCamera_; }

    /**
     * @brief Get right camera
     */
    rendering::Camera& getRightCamera() { return rightCamera_; }
    const rendering::Camera& getRightCamera() const { return rightCamera_; }

    /**
     * @brief Sync right camera to left camera (if sync is enabled)
     */
    void syncCameras();

    /**
     * @brief Get left preset
     */
    const BlackHolePreset& getLeftPreset() const { return leftPreset_; }

    /**
     * @brief Get right preset
     */
    const BlackHolePreset& getRightPreset() const { return rightPreset_; }

    /**
     * @brief Get split mode (0 = vertical split, 1 = horizontal split)
     */
    int getSplitMode() const { return splitMode_; }

    /**
     * @brief Set split mode (0 = vertical, 1 = horizontal)
     */
    void setSplitMode(int mode);

private:
    bool enabled_;
    bool syncCameras_;
    int splitMode_;  // 0 = vertical (left/right), 1 = horizontal (top/bottom)

    // Black holes
    BlackHolePreset leftPreset_;
    BlackHolePreset rightPreset_;
    std::unique_ptr<physics::Metric> leftMetric_;
    std::unique_ptr<physics::Metric> rightMetric_;

    // Cameras
    rendering::Camera leftCamera_;
    rendering::Camera rightCamera_;

    // Viewports
    Viewport leftViewport_;
    Viewport rightViewport_;

    /**
     * @brief Create metric from preset
     */
    std::unique_ptr<physics::Metric> createMetricFromPreset(const BlackHolePreset& preset);
};

} // namespace core
} // namespace cosmic
