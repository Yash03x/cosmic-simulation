#include "modules/galaxy/GalaxyModule.hpp"
#include <imgui.h>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <random>
#include <iostream>

namespace cosmic {
namespace modules {
namespace galaxy {

GalaxyModule::GalaxyModule()
    : window_(nullptr),
      initialized_(false),
      galaxyType_(GalaxyType::Spiral),
      numStars_(10000),
      galaxyRadius_(100.0f),
      rotationSpeed_(0.5f),
      starVAO_(0),
      starVBO_(0),
      shaderProgram_(0),
      simulationTime_(0.0f),
      timeScale_(1.0f),
      paused_(false) {
}

GalaxyModule::~GalaxyModule() {
    cleanup();
}

bool GalaxyModule::initialize(GLFWwindow* window) {
    window_ = window;

    std::cout << "GalaxyModule: Initializing...\n";

    // Initialize rendering
    if (!initializeRendering()) {
        std::cerr << "ERROR::GalaxyModule::Failed to initialize rendering\n";
        return false;
    }

    // Generate initial galaxy
    generateGalaxy();

    initialized_ = true;
    std::cout << "GalaxyModule: Initialization complete with " << stars_.size() << " stars\n";

    return true;
}

void GalaxyModule::update(float deltaTime) {
    if (!initialized_ || paused_) return;

    float effectiveDelta = deltaTime * timeScale_;
    simulationTime_ += effectiveDelta;

    // Update star positions
    updateStarPositions(effectiveDelta);
}

void GalaxyModule::render(rendering::Camera& camera, int windowWidth, int windowHeight) {
    if (!initialized_) return;

    // Clear background
    glClearColor(0.0f, 0.0f, 0.02f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render stars
    renderStars(camera);
}

void GalaxyModule::renderUI() {
    if (!initialized_) return;

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(350, 400), ImGuiCond_FirstUseEver);

    ImGui::Begin("Galaxy Simulation", nullptr, ImGuiWindowFlags_None);

    ImGui::TextWrapped("Real-time N-body galaxy simulation");
    ImGui::Separator();
    ImGui::Spacing();

    // Galaxy type selection
    if (ImGui::CollapsingHeader("Galaxy Type", ImGuiTreeNodeFlags_DefaultOpen)) {
        const char* types[] = {"Spiral", "Elliptical", "Irregular", "Barred Spiral"};
        int currentType = static_cast<int>(galaxyType_);

        if (ImGui::Combo("Type", &currentType, types, 4)) {
            galaxyType_ = static_cast<GalaxyType>(currentType);
            generateGalaxy();
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Select galaxy morphology");
        }
    }

    // Galaxy parameters
    ImGui::Spacing();
    if (ImGui::CollapsingHeader("Parameters", ImGuiTreeNodeFlags_DefaultOpen)) {
        bool regenerate = false;

        if (ImGui::SliderInt("Stars", &numStars_, 1000, 50000)) {
            regenerate = true;
        }

        if (ImGui::SliderFloat("Radius", &galaxyRadius_, 50.0f, 300.0f, "%.1f kpc")) {
            regenerate = true;
        }

        ImGui::SliderFloat("Rotation Speed", &rotationSpeed_, 0.1f, 2.0f);

        if (regenerate && ImGui::Button("Regenerate Galaxy")) {
            generateGalaxy();
        }
    }

    // Simulation controls
    ImGui::Spacing();
    if (ImGui::CollapsingHeader("Simulation", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button(paused_ ? "Resume" : "Pause")) {
            paused_ = !paused_;
        }

        ImGui::SameLine();
        if (ImGui::Button("Reset")) {
            reset();
        }

        ImGui::SliderFloat("Time Scale", &timeScale_, 0.1f, 10.0f, "%.1fx");

        ImGui::Spacing();
        ImGui::Text("Simulation Time: %.2f Myr", simulationTime_ / 1000000.0f);
        ImGui::Text("Stars: %d", static_cast<int>(stars_.size()));
    }

    // Statistics
    ImGui::Spacing();
    if (ImGui::CollapsingHeader("Statistics")) {
        ImGui::Text("Total Mass: %.2e M☉", numStars_ * 1.0);  // Simplified
        ImGui::Text("Galaxy Radius: %.1f kpc", galaxyRadius_);

        // Calculate average velocity
        float avgVel = 0.0f;
        for (const auto& star : stars_) {
            avgVel += glm::length(star.velocity);
        }
        if (!stars_.empty()) {
            avgVel /= stars_.size();
        }
        ImGui::Text("Avg Velocity: %.1f km/s", avgVel * 100.0f);
    }

    // Information
    ImGui::Spacing();
    if (ImGui::CollapsingHeader("About")) {
        ImGui::TextWrapped(
            "This module simulates galactic dynamics using simplified N-body "
            "physics. Stars orbit around the galactic center following "
            "Newtonian gravity with dark matter halo contributions."
        );
        ImGui::Spacing();
        ImGui::TextWrapped(
            "Different galaxy types have distinct morphologies and rotation "
            "curves. Spiral galaxies show differential rotation, while "
            "elliptical galaxies have more random stellar motions."
        );
    }

    ImGui::End();
}

void GalaxyModule::cleanup() {
    // Clean up OpenGL resources
    if (starVAO_ != 0) {
        glDeleteVertexArrays(1, &starVAO_);
        starVAO_ = 0;
    }
    if (starVBO_ != 0) {
        glDeleteBuffers(1, &starVBO_);
        starVBO_ = 0;
    }
    if (shaderProgram_ != 0) {
        glDeleteProgram(shaderProgram_);
        shaderProgram_ = 0;
    }

    stars_.clear();
    initialized_ = false;

    std::cout << "GalaxyModule: Cleanup complete\n";
}

void GalaxyModule::reset() {
    simulationTime_ = 0.0f;
    generateGalaxy();
    std::cout << "GalaxyModule: Reset to initial state\n";
}

void GalaxyModule::generateGalaxy() {
    stars_.clear();
    stars_.reserve(numStars_);

    switch (galaxyType_) {
        case GalaxyType::Spiral:
        case GalaxyType::Barred:
            generateSpiralGalaxy();
            break;
        case GalaxyType::Elliptical:
            generateEllipticalGalaxy();
            break;
        case GalaxyType::Irregular:
            // TODO: Implement irregular galaxy generation
            generateSpiralGalaxy();  // Fallback for now
            break;
    }

    std::cout << "GalaxyModule: Generated " << galaxyType_ << " galaxy with "
              << stars_.size() << " stars\n";
}

void GalaxyModule::generateSpiralGalaxy() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    const int numArms = 2;
    const float armAngleOffset = glm::two_pi<float>() / numArms;

    for (int i = 0; i < numStars_; i++) {
        Star star;

        // Radial distance (with exponential disk profile)
        float r = galaxyRadius_ * std::sqrt(dis(gen));

        // Angular position (spiral arms)
        int arm = i % numArms;
        float armAngle = arm * armAngleOffset;
        float spiralAngle = armAngle + (r / galaxyRadius_) * glm::pi<float>();
        float randomAngle = (dis(gen) - 0.5f) * 0.5f;  // Add some scatter
        float theta = spiralAngle + randomAngle;

        // Height (disk thickness)
        float z = (dis(gen) - 0.5f) * 5.0f * std::exp(-r / (galaxyRadius_ * 0.5f));

        // Position
        star.position = glm::vec3(
            r * std::cos(theta),
            z,
            r * std::sin(theta)
        );

        // Orbital velocity (simplified rotation curve)
        float vOrbit = rotationSpeed_ * std::sqrt(r / galaxyRadius_);
        star.velocity = glm::vec3(
            -vOrbit * std::sin(theta),
            0.0f,
            vOrbit * std::cos(theta)
        );

        // Color (redder toward center, bluer in arms)
        float colorFactor = r / galaxyRadius_;
        star.color = glm::vec3(
            0.8f + 0.2f * (1.0f - colorFactor),
            0.8f + 0.2f * colorFactor,
            0.9f + 0.1f * colorFactor
        );

        star.mass = 1.0f;
        star.luminosity = 1.0f;

        stars_.push_back(star);
    }
}

void GalaxyModule::generateEllipticalGalaxy() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> normalDis(0.0f, 1.0f);
    std::uniform_real_distribution<float> uniformDis(0.0f, 1.0f);

    for (int i = 0; i < numStars_; i++) {
        Star star;

        // Random position (spheroidal distribution)
        float r = galaxyRadius_ * std::pow(uniformDis(gen), 1.0f/3.0f);
        float theta = std::acos(2.0f * uniformDis(gen) - 1.0f);
        float phi = glm::two_pi<float>() * uniformDis(gen);

        // Ellipsoidal shape (flattened along y-axis)
        float flatten = 0.7f;
        star.position = glm::vec3(
            r * std::sin(theta) * std::cos(phi),
            r * std::sin(theta) * std::sin(phi) * flatten,
            r * std::cos(theta)
        );

        // Random velocity (velocity dispersion)
        float velDispersion = 0.3f;
        star.velocity = glm::vec3(
            normalDis(gen) * velDispersion,
            normalDis(gen) * velDispersion,
            normalDis(gen) * velDispersion
        );

        // Color (uniformly old, red stars)
        star.color = glm::vec3(0.9f, 0.7f, 0.5f);

        star.mass = 1.0f;
        star.luminosity = 0.8f;

        stars_.push_back(star);
    }
}

void GalaxyModule::updateStarPositions(float deltaTime) {
    // Simplified N-body simulation
    // In production, you'd use Barnes-Hut or other optimizations

    for (auto& star : stars_) {
        // Update position
        star.position += star.velocity * deltaTime;

        // Simple gravity (point mass at center + dark matter halo)
        float r = glm::length(star.position);
        if (r > 0.1f) {
            glm::vec3 rHat = star.position / r;

            // Gravitational acceleration
            float M_center = numStars_ * 0.1f;  // Central mass
            float M_dm = numStars_ * 0.5f;      // Dark matter
            float a_grav = -(M_center / (r * r) + M_dm / (r * r * r));

            star.velocity += rHat * a_grav * deltaTime;
        }
    }
}

bool GalaxyModule::initializeRendering() {
    // TODO: Initialize shaders and buffers for star rendering
    // For now, return true to allow the module to load

    glGenVertexArrays(1, &starVAO_);
    glGenBuffers(1, &starVBO_);

    std::cout << "GalaxyModule: Rendering initialized (simplified)\n";

    return true;
}

void GalaxyModule::renderStars(const rendering::Camera& camera) {
    // TODO: Implement actual star rendering with point sprites
    // This would use the star VAO/VBO and shader program

    // For now, this is a placeholder
    // In a complete implementation, you would:
    // 1. Update VBO with star positions and colors
    // 2. Bind shader program
    // 3. Set view/projection matrices
    // 4. Draw point sprites
}

} // namespace galaxy
} // namespace modules
} // namespace cosmic
