#pragma once

#include "../../core/Module.hpp"
#include "../../core/UI.hpp"
#include "../../rendering/Renderer.hpp"
#include "../../physics/Metric.hpp"
#include "../../cosmic/BlackHolePresets.hpp"
#include <memory>

namespace cosmic {
namespace modules {
namespace blackhole {

/**
 * @brief Black hole simulation module
 *
 * Comprehensive black hole visualization with:
 * - Schwarzschild and Kerr metrics
 * - Accretion disks
 * - Gravitational lensing
 * - Particle trajectories
 * - Binary mergers
 * - Neutron stars and pulsars
 * - Gravitational waves
 * - Measurement tools
 * - And much more!
 */
class BlackHoleModule : public core::Module {
public:
    /**
     * @brief Construct black hole module
     */
    BlackHoleModule();

    /**
     * @brief Destructor
     */
    ~BlackHoleModule() override;

    // Module interface implementation
    std::string getName() const override { return "Black Holes"; }

    std::string getDescription() const override {
        return "Explore supermassive and stellar black holes with "
               "gravitational lensing, accretion disks, particle trajectories, "
               "binary mergers, and more. Includes 14 major feature systems!";
    }

    std::string getCategory() const override { return "Compact Objects"; }

    bool initialize(GLFWwindow* window) override;
    void update(float deltaTime) override;
    void render(rendering::Camera& camera, int windowWidth, int windowHeight) override;
    void renderUI() override;
    void handleKeyboard(int key, int action) override;
    void cleanup() override;
    void reset() override;

    bool wantsCaptureMouse() const override;
    bool wantsCaptureKeyboard() const override;

private:
    GLFWwindow* window_;

    // Core systems (reuse existing code)
    std::unique_ptr<core::UI> ui_;
    std::unique_ptr<rendering::Renderer> renderer_;
    std::unique_ptr<physics::Metric> metric_;

    // Module state
    bool initialized_;
    float simulationTime_;

    /**
     * @brief Handle black hole preset changes from UI
     */
    void handlePresetChanges();

    /**
     * @brief Handle metric type changes
     */
    void handleMetricChanges();
};

} // namespace blackhole
} // namespace modules
} // namespace cosmic
