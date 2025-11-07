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
      selectedGalaxyIndex_(0),
      starVAO_(0),
      starVBO_(0),
      shaderProgram_(0),
      simulationTime_(0.0f),
      timeScale_(1.0f),
      paused_(false) {

    // Load all galaxy presets
    availableGalaxies_ = GalaxyPresets::getAllPresets();

    // Start with Milky Way
    if (!availableGalaxies_.empty()) {
        currentGalaxy_ = availableGalaxies_[0];
    }
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
    ImGui::SetNextWindowSize(ImVec2(400, 550), ImGuiCond_FirstUseEver);

    ImGui::Begin("Galaxy Simulation", nullptr, ImGuiWindowFlags_None);

    ImGui::TextWrapped("Explore real galaxies with accurate astronomical data!");
    ImGui::Separator();
    ImGui::Spacing();

    // Galaxy preset selection
    if (ImGui::CollapsingHeader("Famous Galaxies", ImGuiTreeNodeFlags_DefaultOpen)) {
        std::vector<const char*> galaxyNames;
        for (const auto& galaxy : availableGalaxies_) {
            galaxyNames.push_back(galaxy.name.c_str());
        }

        int currentIndex = selectedGalaxyIndex_;
        if (ImGui::Combo("Select Galaxy", &currentIndex, galaxyNames.data(),
                       static_cast<int>(galaxyNames.size()))) {
            selectedGalaxyIndex_ = currentIndex;
            currentGalaxy_ = availableGalaxies_[selectedGalaxyIndex_];
            generateGalaxy();
        }

        ImGui::Spacing();
        ImGui::TextWrapped("%s", currentGalaxy_.description.c_str());

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Based on real astronomical observations");
        }
    }

    // Physical Properties
    ImGui::Spacing();
    if (ImGui::CollapsingHeader("Physical Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Total Mass: %.2e M☉", currentGalaxy_.totalMass);
        ImGui::Text("Stellar Mass: %.2e M☉", currentGalaxy_.stellarMass);
        ImGui::Text("Dark Matter: %.2e M☉", currentGalaxy_.darkMatterMass);

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("~%.0f%% of galaxy mass is dark matter",
                            (currentGalaxy_.darkMatterMass / currentGalaxy_.totalMass) * 100.0f);
        }

        ImGui::Spacing();
        ImGui::Text("Disk Radius: %.1f kpc (%.0f ly)",
                   currentGalaxy_.radius,
                   currentGalaxy_.radius * 3262.0);  // Convert kpc to light-years

        ImGui::Text("Disk Thickness: %.2f kpc (%.0f ly)",
                   currentGalaxy_.scaleHeight,
                   currentGalaxy_.scaleHeight * 3262.0);

        ImGui::Text("Stars: %.2e", static_cast<double>(currentGalaxy_.actualStarCount));
        ImGui::Text("Luminosity: %.2e L☉", currentGalaxy_.luminosity);
    }

    // Dynamics
    ImGui::Spacing();
    if (ImGui::CollapsingHeader("Dynamics", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (currentGalaxy_.type != GalaxyType::Elliptical) {
            ImGui::Text("Rotation Velocity: %.0f km/s", currentGalaxy_.rotationVelocity);

            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Orbital velocity at R ≈ 8.5 kpc");
            }

            ImGui::Text("Orbital Period: %.0f Myr",
                       (2.0 * 3.14159 * 8.5 * 3262.0 * 9.461e12) /  // Circumference in km
                       (currentGalaxy_.rotationVelocity * 365.25 * 24 * 3600 * 1e6));  // Period in Myr
        }

        ImGui::Text("Velocity Dispersion: %.0f km/s", currentGalaxy_.velocityDispersion);

        if (currentGalaxy_.type != GalaxyType::Elliptical) {
            ImGui::Spacing();
            ImGui::Text("Spiral Arms: %d", currentGalaxy_.numSpiralArms);
            if (currentGalaxy_.numSpiralArms > 0) {
                ImGui::Text("Arm Pitch Angle: %.0f°", currentGalaxy_.armPitch);
            }
            if (currentGalaxy_.barLength > 0) {
                ImGui::Text("Bar Length: %.1f kpc", currentGalaxy_.barLength * 2.0);
            }
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
    stars_.reserve(currentGalaxy_.numStarsSimulated);

    switch (currentGalaxy_.type) {
        case GalaxyType::Spiral:
        case GalaxyType::Barred:
            generateSpiralGalaxy();
            break;
        case GalaxyType::Elliptical:
            generateEllipticalGalaxy();
            break;
        case GalaxyType::Irregular:
            generateSpiralGalaxy();  // Irregular uses modified spiral
            break;
    }

    std::cout << "GalaxyModule: Generated " << currentGalaxy_.name << " with "
              << stars_.size() << " stars (representing " << currentGalaxy_.actualStarCount << " actual stars)\n";
}

void GalaxyModule::generateSpiralGalaxy() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    std::normal_distribution<float> normalDis(0.0f, 1.0f);

    const int numArms = currentGalaxy_.numSpiralArms > 0 ? currentGalaxy_.numSpiralArms : 2;
    const float armAngleOffset = glm::two_pi<float>() / numArms;
    const float galaxyRadius = static_cast<float>(currentGalaxy_.radius);  // kpc
    const float scaleHeight = static_cast<float>(currentGalaxy_.scaleHeight);  // kpc
    const float rotVel = static_cast<float>(currentGalaxy_.rotationVelocity);  // km/s

    for (int i = 0; i < currentGalaxy_.numStarsSimulated; i++) {
        Star star;

        // Radial distance (exponential disk profile)
        float u = dis(gen);
        float r = -galaxyRadius * 0.5f * std::log(1.0f - u * 0.95f);  // Scale length = R/2
        r = std::min(r, galaxyRadius);  // Cap at galaxy radius

        // Angular position (spiral arms with pitch angle)
        int arm = i % numArms;
        float armAngle = arm * armAngleOffset;
        float pitchRad = glm::radians(static_cast<float>(currentGalaxy_.armPitch));
        float spiralAngle = armAngle + (r / galaxyRadius) * 2.0f * glm::pi<float>() / std::tan(pitchRad);

        // Add scatter around spiral arms
        float armScatter = (dis(gen) - 0.5f) * 0.8f;
        float theta = spiralAngle + armScatter;

        // Add bar if present
        if (currentGalaxy_.barLength > 0 && r < currentGalaxy_.barLength) {
            // Stars in bar region - align more with bar
            float barAngle = (arm < numArms / 2) ? 0.0f : glm::pi<float>();
            theta = barAngle + (dis(gen) - 0.5f) * 0.3f;
        }

        // Height (Gaussian vertical distribution)
        float z = normalDis(gen) * scaleHeight * std::exp(-r / (galaxyRadius * 0.7f));

        // Position in kpc
        star.position = glm::vec3(
            r * std::cos(theta),
            z,
            r * std::sin(theta)
        );

        // Realistic rotation curve (Flat rotation curve with slight rise)
        // V(r) = V_max * (1 - exp(-r/r_s)) for dark matter halo
        float vOrbit = rotVel * (1.0f - std::exp(-r / (galaxyRadius * 0.15f)));

        // Orbital velocity in km/s
        star.velocity = glm::vec3(
            -vOrbit * std::sin(theta),
            normalDis(gen) * currentGalaxy_.velocityDispersion * 0.3f,  // Vertical motion
            vOrbit * std::cos(theta)
        );

        // Color based on stellar population
        // Blue young stars in arms, red old stars in bulge
        float colorFactor = r / galaxyRadius;
        float armProximity = std::abs(std::sin((theta - spiralAngle) * numArms));

        if (r < currentGalaxy_.centralBulgeRadius) {
            // Bulge stars (old, red)
            star.color = glm::vec3(0.9f, 0.7f, 0.5f);
            star.temperature = 4500.0f;  // K-type stars
            star.mass = 0.8f;  // Solar masses
            star.luminosity = 0.4f;  // Solar luminosities
        } else if (armProximity > 0.7f) {
            // Spiral arm stars (young, blue)
            star.color = glm::vec3(0.6f, 0.7f, 1.0f);
            star.temperature = 10000.0f;  // A/B-type stars
            star.mass = 2.0f;
            star.luminosity = 20.0f;
        } else {
            // Disk stars (intermediate)
            star.color = glm::vec3(0.9f, 0.9f, 0.8f);
            star.temperature = 5800.0f;  // G-type (like Sun)
            star.mass = 1.0f;
            star.luminosity = 1.0f;
        }

        stars_.push_back(star);
    }
}

void GalaxyModule::generateEllipticalGalaxy() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> normalDis(0.0f, 1.0f);
    std::uniform_real_distribution<float> uniformDis(0.0f, 1.0f);

    const float effectiveRadius = static_cast<float>(currentGalaxy_.radius);  // kpc
    const float velDispersion = static_cast<float>(currentGalaxy_.velocityDispersion);  // km/s

    for (int i = 0; i < currentGalaxy_.numStarsSimulated; i++) {
        Star star;

        // de Vaucouleurs profile (R^(1/4) law) for elliptical galaxies
        float u = uniformDis(gen);
        float r = effectiveRadius * std::pow(-std::log(1.0f - u), 0.25f);  // Approximate R^(1/4)
        r = std::min(r, effectiveRadius * 3.0f);  // Cap at 3x effective radius

        float theta = std::acos(2.0f * uniformDis(gen) - 1.0f);
        float phi = glm::two_pi<float>() * uniformDis(gen);

        // Ellipsoidal shape (E0-E7 classification)
        // For M87 (E0-1), nearly spherical
        float flatten = 0.9f;  // E0 is nearly round

        star.position = glm::vec3(
            r * std::sin(theta) * std::cos(phi),
            r * std::sin(theta) * std::sin(phi) * flatten,
            r * std::cos(theta)
        );

        // Velocity dispersion (isotropic, no net rotation for ellipticals)
        // Higher dispersion in center, lower at large radii
        float localDispersion = velDispersion * std::exp(-r / (effectiveRadius * 2.0f));

        star.velocity = glm::vec3(
            normalDis(gen) * localDispersion,
            normalDis(gen) * localDispersion,
            normalDis(gen) * localDispersion
        );

        // Elliptical galaxies have old stellar populations
        // Color-magnitude relation: redder in center (metal-rich), bluer outskirts
        float colorGradient = std::max(0.0f, 1.0f - r / effectiveRadius);

        star.color = glm::vec3(
            0.85f + 0.1f * colorGradient,   // Red
            0.65f + 0.1f * colorGradient,   // Green
            0.45f + 0.05f * colorGradient   // Blue
        );

        // Old, low-mass stars dominate elliptical galaxies
        star.temperature = 4000.0f + 1000.0f * colorGradient;  // 4000-5000 K (K/M-type)
        star.mass = 0.6f + 0.3f * uniformDis(gen);  // 0.6-0.9 solar masses
        star.luminosity = 0.3f + 0.4f * colorGradient;  // 0.3-0.7 solar luminosities

        stars_.push_back(star);
    }
}

void GalaxyModule::updateStarPositions(float deltaTime) {
    // Simplified N-body simulation with realistic galaxy mass
    // Uses point mass + NFW dark matter halo

    // Gravitational constant in units where velocities are in km/s,
    // distances in kpc, masses in solar masses, and time in Myr
    const float G = 4.302e-3;  // kpc * (km/s)^2 / M_sun

    // Convert deltaTime from seconds to millions of years
    const float deltaTime_Myr = deltaTime / (365.25 * 24 * 3600 * 1e6);

    // Galaxy mass components (in solar masses)
    const float M_bulge = static_cast<float>(currentGalaxy_.stellarMass * 0.2);  // 20% in bulge
    const float M_dm_total = static_cast<float>(currentGalaxy_.darkMatterMass);

    for (auto& star : stars_) {
        // Update position (v * dt, with dt in Myr, v in km/s, gives position change in kpc)
        star.position += star.velocity * deltaTime_Myr * 1.0226e-6f;  // Convert km/s * Myr to kpc

        // Gravitational acceleration from galaxy mass distribution
        float r = glm::length(star.position);
        if (r > 0.01f) {  // Avoid singularity at center
            glm::vec3 rHat = star.position / r;

            // 1. Central bulge (point mass approximation)
            float a_bulge = -G * M_bulge / (r * r);

            // 2. NFW dark matter halo
            // M_DM(r) = M_200 * [ln(1+r/r_s) - (r/r_s)/(1+r/r_s)]
            float r_s = static_cast<float>(currentGalaxy_.radius) * 0.2f;  // Scale radius ~20% of disk radius
            float x = r / r_s;
            float M_dm_enclosed = M_dm_total * (std::log(1.0f + x) - x / (1.0f + x)) /
                                 (std::log(1.0f + 5.0f) - 5.0f / 6.0f);  // Normalize
            float a_dm = -G * M_dm_enclosed / (r * r);

            // Total acceleration
            float a_total = a_bulge + a_dm;

            // Update velocity (a * dt in km/s)
            star.velocity += rHat * a_total * (deltaTime_Myr * 1e6 * 365.25 * 24 * 3600 / 1e3);  // Convert to km/s
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
