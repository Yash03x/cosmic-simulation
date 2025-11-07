#pragma once

#include "../../core/Module.hpp"
#include "GalaxyPresets.hpp"
#include <glm/glm.hpp>
#include <vector>

namespace cosmic {
namespace modules {
namespace galaxy {

/**
 * @brief Star particle for galaxy simulation
 */
struct Star {
    glm::vec3 position;      // Position in kpc
    glm::vec3 velocity;      // Velocity in km/s
    glm::vec3 color;         // RGB color (0-1)
    float mass;              // Mass in solar masses
    float luminosity;        // Luminosity in solar luminosities
    float temperature;       // Surface temperature in Kelvin
};

/**
 * @brief Galaxy simulation module
 *
 * Real-time N-body galaxy simulation with:
 * - Multiple galaxy types
 * - Star formation
 * - Orbital mechanics
 * - Dark matter visualization
 * - Galaxy collisions
 */
class GalaxyModule : public core::Module {
public:
    GalaxyModule();
    ~GalaxyModule() override;

    // Module interface
    std::string getName() const override { return "Galaxies"; }

    std::string getDescription() const override {
        return "Explore galactic dynamics with real-time N-body simulations. "
               "Create spiral, elliptical, and barred galaxies. Simulate "
               "galaxy collisions and observe orbital mechanics at scale!";
    }

    std::string getCategory() const override { return "Galaxies"; }

    bool initialize(GLFWwindow* window) override;
    void update(float deltaTime) override;
    void render(rendering::Camera& camera, int windowWidth, int windowHeight) override;
    void renderUI() override;
    void cleanup() override;
    void reset() override;

private:
    GLFWwindow* window_;
    bool initialized_;

    // Galaxy preset system
    GalaxyProperties currentGalaxy_;
    std::vector<GalaxyProperties> availableGalaxies_;
    int selectedGalaxyIndex_;

    // Star data
    std::vector<Star> stars_;

    // Rendering
    unsigned int starVAO_;
    unsigned int starVBO_;
    unsigned int shaderProgram_;

    // Simulation
    float simulationTime_;
    float timeScale_;
    bool paused_;

    /**
     * @brief Generate galaxy based on type
     */
    void generateGalaxy();

    /**
     * @brief Generate spiral galaxy
     */
    void generateSpiralGalaxy();

    /**
     * @brief Generate elliptical galaxy
     */
    void generateEllipticalGalaxy();

    /**
     * @brief Update star positions (N-body simulation)
     */
    void updateStarPositions(float deltaTime);

    /**
     * @brief Initialize rendering resources
     */
    bool initializeRendering();

    /**
     * @brief Render all stars
     */
    void renderStars(const rendering::Camera& camera);
};

} // namespace galaxy
} // namespace modules
} // namespace cosmic
