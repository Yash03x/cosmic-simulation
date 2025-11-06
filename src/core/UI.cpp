#include "core/UI.hpp"
#include "physics/Schwarzschild.hpp"
#include "physics/Kerr.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>

namespace cosmic {
namespace core {

UI::UI(GLFWwindow* window)
    : window_(window),
      visible_(true),
      blackHoleMass_(1.0f),
      blackHoleMassChanged_(false),
      metricType_(0),
      metricTypeChanged_(false),
      spin_(0.0f),
      spinChanged_(false),
      maxRaySteps_(1000),
      stepSize_(0.1f),
      accretionDiskEnabled_(false),
      cameraSpeed_(10.0f),
      accretionRate_(0.1f),
      alphaViscosity_(0.1f),
      diskInclination_(1.0f),
      scaleHeightRatio_(0.05f),
      selectedPresetIndex_(0),
      presetChanged_(false),
      timeScale_(1.0f),
      paused_(false),
      showDemoWindow_(false) {
    // Load all available presets
    availablePresets_ = BlackHolePresets::getAllPresets();
    // Initialize with first preset (M87*)
    if (!availablePresets_.empty()) {
        selectedPreset_ = availablePresets_[0];
    }
}

UI::~UI() {
    cleanup();
}

bool UI::initialize() {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    const char* glsl_version = "#version 410";
    if (!ImGui_ImplGlfw_InitForOpenGL(window_, true)) {
        std::cerr << "ERROR::UI::IMGUI_GLFW_INIT_FAILED\n";
        return false;
    }
    if (!ImGui_ImplOpenGL3_Init(glsl_version)) {
        std::cerr << "ERROR::UI::IMGUI_OPENGL3_INIT_FAILED\n";
        return false;
    }

    std::cout << "ImGui initialized successfully\n";
    std::cout << "  Version: " << IMGUI_VERSION << "\n";

    return true;
}

void UI::newFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UI::render(rendering::Camera& camera,
                physics::Metric* metric,
                rendering::Renderer& renderer,
                float deltaTime,
                float fps,
                int activePresetNumber,
                const std::string& activePresetDescription) {
    if (!visible_) return;

    // Reset change flags
    blackHoleMassChanged_ = false;
    metricTypeChanged_ = false;
    spinChanged_ = false;
    presetChanged_ = false;

    // Render main control panel
    renderControlPanel(camera, metric, renderer);

    // Render statistics panel
    renderStatsPanel(camera, metric, deltaTime, fps, activePresetNumber, activePresetDescription);

    // Render measurement tools panel
    renderMeasurementPanel(metric);

    // Render particle trajectory panel
    renderParticlePanel(metric);

    // Render about panel
    renderAboutPanel();

    // Optional: Show ImGui demo window
    if (showDemoWindow_) {
        ImGui::ShowDemoWindow(&showDemoWindow_);
    }
}

void UI::renderControlPanel(rendering::Camera& camera,
                            physics::Metric* metric,
                            rendering::Renderer& renderer) {
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(350, 400), ImGuiCond_FirstUseEver);

    ImGui::Begin("Cosmic Simulator Controls", nullptr, ImGuiWindowFlags_None);

    ImGui::Text("Black Hole Simulator v2.0");
    ImGui::Separator();

    // Famous Black Hole Presets
    if (ImGui::CollapsingHeader("Famous Black Holes", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Select a preset:");

        // Create array of preset names for combo
        std::vector<const char*> presetNames;
        for (const auto& preset : availablePresets_) {
            presetNames.push_back(preset.name.c_str());
        }

        int previousIndex = selectedPresetIndex_;
        if (ImGui::Combo("Preset", &selectedPresetIndex_, presetNames.data(), presetNames.size())) {
            if (selectedPresetIndex_ != previousIndex && selectedPresetIndex_ >= 0 &&
                selectedPresetIndex_ < static_cast<int>(availablePresets_.size())) {
                selectedPreset_ = availablePresets_[selectedPresetIndex_];
                presetChanged_ = true;

                // Auto-apply preset values
                blackHoleMass_ = selectedPreset_.mass;
                blackHoleMassChanged_ = true;
                spin_ = selectedPreset_.spin;
                spinChanged_ = true;
                metricType_ = (std::abs(selectedPreset_.spin) > 0.01f) ? 1 : 0;
                metricTypeChanged_ = true;

                // Apply accretion and jet settings
                accretionRate_ = selectedPreset_.accretionRate;
                renderer.setAccretionRate(accretionRate_);
                renderer.setJetsEnabled(selectedPreset_.hasJets);

                if (selectedPreset_.accretionRate > 0.001f) {
                    accretionDiskEnabled_ = true;
                    renderer.setAccretionDiskEnabled(true);
                }
            }
        }

        // Display description
        if (selectedPresetIndex_ >= 0 && selectedPresetIndex_ < static_cast<int>(availablePresets_.size())) {
            ImGui::Spacing();
            ImGui::TextWrapped("%s", selectedPreset_.description.c_str());
            ImGui::Spacing();
            ImGui::Text("Mass: %.2e M☉", selectedPreset_.mass);
            ImGui::Text("Spin: %.3f", selectedPreset_.spin);
            ImGui::Text("Distance: %.2f kpc", selectedPreset_.distance);
            if (selectedPreset_.hasJets) {
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "✓ Relativistic Jets");
            }
        }
    }

    ImGui::Spacing();

    // Time Controls
    if (ImGui::CollapsingHeader("Time Control", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button(paused_ ? "▶ Play" : "⏸ Pause")) {
            paused_ = !paused_;
        }

        ImGui::SameLine();
        if (ImGui::Button("⏮ Reset")) {
            timeScale_ = 1.0f;
            paused_ = false;
        }

        ImGui::SliderFloat("Speed", &timeScale_, 0.1f, 10.0f, "%.2fx");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Simulation time scale\n"
                             "< 1.0 = Slow motion\n"
                             "> 1.0 = Fast forward");
        }

        ImGui::Text("Status: %s", paused_ ? "⏸ PAUSED" : "▶ RUNNING");
        ImGui::Text("Time Scale: %.2fx", timeScale_);
    }

    ImGui::Spacing();

    // Black hole parameters
    if (ImGui::CollapsingHeader("Black Hole Parameters", ImGuiTreeNodeFlags_DefaultOpen)) {
        float oldMass = blackHoleMass_;
        ImGui::SliderFloat("Mass (M☉)", &blackHoleMass_, 0.1f, 10.0f, "%.2f");
        if (oldMass != blackHoleMass_) {
            blackHoleMassChanged_ = true;
        }

        const char* metricOptions[] = {"Schwarzschild", "Kerr (rotating)"};
        int previousMetric = metricType_;
        if (ImGui::Combo("Metric", &metricType_, metricOptions, IM_ARRAYSIZE(metricOptions))) {
            if (metricType_ != previousMetric) {
                metricTypeChanged_ = true;
            }
        }

        if (metricType_ == 1) {
            float oldSpin = spin_;
            if (ImGui::SliderFloat("Spin (a/M)", &spin_, -0.998f, 0.998f, "%.3f")) {
                if (std::abs(oldSpin - spin_) > 1e-6f) {
                    spinChanged_ = true;
                }
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Dimensionless spin parameter a/M (0 = Schwarzschild, 0.998 ≈ astrophysical limit)");
            }
        }

    if (metric) {
        ImGui::Text("Event Horizon: %.2f M", metric->eventHorizonRadius());
        ImGui::Text("Photon Sphere: %.2f M", metric->photonSphereRadius());
        ImGui::Text("ISCO: %.2f M", metric->iscoRadius());
        if (auto* kerr = dynamic_cast<physics::Kerr*>(metric)) {
            ImGui::Text("Spin (a/M): %.3f", kerr->getSpin());
        }
    }
    }

    ImGui::Spacing();

    // Ray tracing parameters
    if (ImGui::CollapsingHeader("Ray Tracing", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::SliderInt("Max Steps", &maxRaySteps_, 100, 5000)) {
            renderer.setMaxRaySteps(maxRaySteps_);
        }

        if (ImGui::SliderFloat("Step Size", &stepSize_, 0.01f, 0.5f, "%.3f")) {
            renderer.setStepSize(stepSize_);
        }

        ImGui::Text("Max Steps: Higher = more accurate but slower");
        ImGui::Text("Step Size: Lower = more accurate but slower");
    }

    ImGui::Spacing();

    // Camera parameters
    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::SliderFloat("Speed", &cameraSpeed_, 1.0f, 50.0f, "%.1f")) {
            camera.setMovementSpeed(cameraSpeed_);
        }

        glm::vec3 pos = camera.getPosition();
        ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);

        float distance = glm::length(pos);
        ImGui::Text("Distance from BH: %.2f M", distance);

        if (ImGui::Button("Reset Position")) {
            camera.setPosition(glm::vec3(0.0f, 0.0f, 20.0f));
        }
    }

    ImGui::Spacing();

    // Accretion disk
    if (ImGui::CollapsingHeader("Accretion Disk")) {
        if (ImGui::Checkbox("Enable Accretion Disk", &accretionDiskEnabled_)) {
            renderer.setAccretionDiskEnabled(accretionDiskEnabled_);
        }

        if (accretionDiskEnabled_) {
            ImGui::Spacing();
            ImGui::Text("Physics Parameters:");
            ImGui::Separator();

            // Accretion rate
            if (ImGui::SliderFloat("Accretion Rate (Mdot)", &accretionRate_, 0.01f, 1.0f, "%.3f M☉/yr")) {
                renderer.setAccretionRate(accretionRate_);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Mass accretion rate in solar masses per year\n"
                                 "Higher = hotter, brighter disk");
            }

            // Alpha viscosity
            if (ImGui::SliderFloat("Alpha Viscosity", &alphaViscosity_, 0.01f, 0.3f, "%.3f")) {
                renderer.setAlphaViscosity(alphaViscosity_);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Shakura-Sunyaev viscosity parameter\n"
                                 "Controls angular momentum transport");
            }

            // Scale height ratio
            if (ImGui::SliderFloat("Scale Height (H/r)", &scaleHeightRatio_, 0.01f, 0.15f, "%.3f")) {
                renderer.setScaleHeightRatio(scaleHeightRatio_);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Disk thickness ratio\n"
                                 "Lower = thinner disk");
            }

            // Disk inclination
            float inclinationDeg = diskInclination_ * 180.0f / 3.14159f;
            if (ImGui::SliderFloat("Inclination", &inclinationDeg, 0.0f, 90.0f, "%.1f°")) {
                diskInclination_ = inclinationDeg * 3.14159f / 180.0f;
                renderer.setDiskInclination(diskInclination_);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Viewing angle\n"
                                 "0° = face-on, 90° = edge-on");
            }

            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Active Features:");
            ImGui::BulletText("Shakura-Sunyaev temperature");
            ImGui::BulletText("Proper scale height H(r)");
            ImGui::BulletText("Optical depth τ(r)");
            ImGui::BulletText("Gravitational redshift");
            ImGui::BulletText("Doppler shifts & beaming");
            ImGui::BulletText("Radiative transfer");
            ImGui::BulletText("Planck spectrum colors");
        }
    }

    ImGui::Spacing();

    // Advanced Visualizations
    if (ImGui::CollapsingHeader("Visualizations")) {
        // Jets
        bool jetsEnabled = renderer.isJetsEnabled();
        if (ImGui::Checkbox("Relativistic Jets", &jetsEnabled)) {
            renderer.setJetsEnabled(jetsEnabled);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Blandford-Znajek powered jets\n"
                             "Only visible for spinning black holes\n"
                             "Velocity: 0.9-0.99c");
        }

        // Ergosphere
        bool ergosphereVisible = renderer.isErgosphereVisible();
        if (ImGui::Checkbox("Ergosphere", &ergosphereVisible)) {
            renderer.setErgosphereVisible(ergosphereVisible);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Frame-dragging region (Kerr only)\n"
                             "Shows where nothing can remain stationary\n"
                             "Penrose process region");
        }

        // Tidal Forces
        bool tidalForces = renderer.isTidalForcesVisible();
        if (ImGui::Checkbox("Tidal Forces (Spaghettification)", &tidalForces)) {
            renderer.setTidalForcesVisible(tidalForces);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Danger zones:\n"
                             "🔴 Red: Steel torn apart\n"
                             "🟠 Orange: Rocks disrupted\n"
                             "🟡 Yellow: Humans don't survive");
        }

        // Bloom
        bool bloomEnabled = renderer.isBloomEnabled();
        if (ImGui::Checkbox("Bloom (HDR Glow)", &bloomEnabled)) {
            renderer.setBloomEnabled(bloomEnabled);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Cinematic post-processing\n"
                             "HDR bloom with ACES tone mapping");
        }

        if (bloomEnabled) {
            float bloomIntensity = renderer.getBloomIntensity();
            if (ImGui::SliderFloat("Bloom Intensity", &bloomIntensity, 0.0f, 2.0f, "%.2f")) {
                renderer.setBloomIntensity(bloomIntensity);
            }

            float bloomThreshold = renderer.getBloomThreshold();
            if (ImGui::SliderFloat("Bloom Threshold", &bloomThreshold, 0.5f, 2.0f, "%.2f")) {
                renderer.setBloomThreshold(bloomThreshold);
            }
        }

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Active Features:");
        if (jetsEnabled) ImGui::BulletText("Jets (synchrotron radiation)");
        if (ergosphereVisible) ImGui::BulletText("Ergosphere boundary");
        if (tidalForces) ImGui::BulletText("Tidal zones (3 levels)");
        if (bloomEnabled) ImGui::BulletText("Bloom post-processing");
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Help
    ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Controls:");
    ImGui::Text("WASD - Move camera");
    ImGui::Text("Q/E - Move up/down");
    ImGui::Text("Mouse - Look around");
    ImGui::Text("TAB - Toggle mouse capture");
    ImGui::Text("H - Toggle UI");
    ImGui::Text("ESC - Exit");

    ImGui::Spacing();

    if (ImGui::Checkbox("Show ImGui Demo", &showDemoWindow_)) {
        // Demo window toggle
    }

    ImGui::End();
}

void UI::renderStatsPanel(rendering::Camera& camera,
                          physics::Metric* metric,
                          float deltaTime,
                          float fps,
                          int activePresetNumber,
                          const std::string& activePresetDescription) {
    ImGui::SetNextWindowPos(ImVec2(10, 430), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(350, 200), ImGuiCond_FirstUseEver);

    ImGui::Begin("Statistics", nullptr, ImGuiWindowFlags_None);

    // Performance
    ImGui::Text("Performance");
    ImGui::Separator();
    ImGui::Text("FPS: %.1f", fps);
    ImGui::Text("Frame time: %.2f ms", deltaTime * 1000.0f);
    ImGui::TextDisabled("(Frame time includes GPU ray tracing)");

    ImGui::Spacing();

    // Camera Preset
    if (activePresetNumber > 0) {
        ImGui::Text("Camera");
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f),
                          "Preset %d: %s", activePresetNumber, activePresetDescription.c_str());
        ImGui::Spacing();
    }

    ImGui::Spacing();

    // Physics
    ImGui::Text("Physics");
    ImGui::Separator();

    if (metric) {
        glm::vec3 pos = camera.getPosition();
        float distance = glm::length(pos);
        double eventHorizon = metric->eventHorizonRadius();
        double photonSphere = metric->photonSphereRadius();
        double isco = metric->iscoRadius();

        ImGui::Text("Distance from BH: %.2f M", distance);

        if (distance < eventHorizon) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
                             "WARNING: Inside event horizon!");
        } else if (distance < photonSphere) {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
                             "Inside photon sphere");
        } else if (distance < isco) {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f),
                             "Inside ISCO");
        } else {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
                             "Safe distance");
        }
    }

    ImGui::End();
}

void UI::renderAboutPanel() {
    // Only show on first launch or when explicitly opened
    static bool showAbout = false;

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                showAbout = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (showAbout) {
        ImGui::SetNextWindowPos(ImVec2(400, 200), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(400, 250), ImGuiCond_FirstUseEver);

        ImGui::Begin("About Cosmic Simulator", &showAbout);

        ImGui::Text("Cosmic Simulator v1.0");
        ImGui::Separator();

        ImGui::TextWrapped("A physics-accurate black hole visualization "
                          "using general relativity and ray tracing.");

        ImGui::Spacing();
        ImGui::Text("Features:");
        ImGui::BulletText("Schwarzschild metric (non-rotating black holes)");
        ImGui::BulletText("Geodesic ray tracing using RK4 integration");
        ImGui::BulletText("Gravitational lensing effects");
        ImGui::BulletText("Real-time parameter adjustment");

        ImGui::Spacing();
        ImGui::Text("Coming Soon:");
        ImGui::BulletText("Kerr metric (rotating black holes)");
        ImGui::BulletText("Accretion disk with Doppler shifts");
        ImGui::BulletText("Neutron stars and pulsars");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Built with C++, OpenGL, ImGui");

        ImGui::End();
    }
}

void UI::renderMeasurementPanel(physics::Metric* metric) {
    ImGui::SetNextWindowPos(ImVec2(10, 650), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(350, 300), ImGuiCond_FirstUseEver);

    ImGui::Begin("Measurement Tools", nullptr, ImGuiWindowFlags_None);

    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Measure physical quantities");
    ImGui::Separator();
    ImGui::Spacing();

    // Measurement mode toggle
    bool active = measurementTools_.isActive();
    if (ImGui::Checkbox("Measurement Mode Active", &active)) {
        measurementTools_.setActive(active);
    }

    if (!active) {
        ImGui::TextDisabled("Enable to start measuring");
        ImGui::End();
        return;
    }

    ImGui::Spacing();

    // Mode selection
    ImGui::Text("Measurement Type:");
    int modeIndex = static_cast<int>(measurementTools_.getMode());
    const char* modes[] = {"Distance", "Angle", "Redshift"};

    if (ImGui::Combo("Mode", &modeIndex, modes, 3)) {
        measurementTools_.setMode(static_cast<tools::MeasurementTools::Mode>(modeIndex));
        measurementTools_.clear();  // Clear points when changing mode
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Mode description
    switch (measurementTools_.getMode()) {
        case tools::MeasurementTools::Mode::Distance:
            ImGui::TextWrapped("Click 'Add Point' twice to measure distance between two points.");
            ImGui::Text("Points needed: 2");
            break;
        case tools::MeasurementTools::Mode::Angle:
            ImGui::TextWrapped("Add 3 points: vertex, then two rays to measure angle.");
            ImGui::Text("Points needed: 3");
            break;
        case tools::MeasurementTools::Mode::Redshift:
            ImGui::TextWrapped("Add 2 points: observer position, then emission point.");
            ImGui::Text("Points needed: 2");
            break;
    }

    ImGui::Spacing();

    // Current points
    size_t pointCount = measurementTools_.getPointCount();
    ImGui::Text("Current points: %zu", pointCount);

    // Controls
    ImGui::Spacing();
    if (ImGui::Button("Add Point (at origin)")) {
        // In a full implementation, this would use ray casting from mouse
        // For now, add points at predefined locations for demonstration
        static float radius = 10.0f;
        static float angle = 0.0f;
        glm::vec3 pos(radius * cos(angle), 0.0f, radius * sin(angle));
        measurementTools_.addPoint(pos);
        angle += glm::pi<float>() / 4.0f;  // 45 degrees
        radius += 2.0f;
    }

    ImGui::SameLine();
    if (ImGui::Button("Clear All")) {
        measurementTools_.clear();
    }

    if (pointCount > 0 && ImGui::Button("Remove Last")) {
        measurementTools_.removeLastPoint();
    }

    // Complete measurement button
    ImGui::Spacing();
    bool canComplete = false;
    switch (measurementTools_.getMode()) {
        case tools::MeasurementTools::Mode::Distance:
        case tools::MeasurementTools::Mode::Redshift:
            canComplete = (pointCount >= 2);
            break;
        case tools::MeasurementTools::Mode::Angle:
            canComplete = (pointCount >= 3);
            break;
    }

    if (canComplete) {
        if (ImGui::Button("Complete Measurement", ImVec2(-1, 0))) {
            measurementTools_.completeMeasurement(metric);
        }
    }

    // Display current measurement (live preview)
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Live Preview:");

    if (pointCount >= 2) {
        switch (measurementTools_.getMode()) {
            case tools::MeasurementTools::Mode::Distance: {
                auto result = measurementTools_.measureDistance(metric);
                if (result) {
                    ImGui::Text("Flat distance: %.3f M", result->euclideanDistance);
                    ImGui::Text("Proper distance: %.3f M", result->properDistance);
                }
                break;
            }
            case tools::MeasurementTools::Mode::Redshift: {
                auto result = measurementTools_.measureRedshift(metric);
                if (result) {
                    ImGui::Text("Redshift z: %.4f", result->gravitationalRedshift);
                    ImGui::Text("Time dilation: %.3fx", result->timeDialation);
                    ImGui::Text("Escape velocity: %.2f%% c", result->escapeVelocity * 100.0f);
                }
                break;
            }
            default:
                break;
        }
    }

    if (pointCount >= 3 && measurementTools_.getMode() == tools::MeasurementTools::Mode::Angle) {
        auto result = measurementTools_.measureAngle();
        if (result) {
            ImGui::Text("Angle: %.2f°", result->angleDegrees);
        }
    }

    // Display completed measurements
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Completed Measurements:");

    const auto& distances = measurementTools_.getDistanceMeasurements();
    const auto& angles = measurementTools_.getAngleMeasurements();
    const auto& redshifts = measurementTools_.getRedshiftMeasurements();

    if (!distances.empty()) {
        ImGui::Text("Distances: %zu", distances.size());
    }
    if (!angles.empty()) {
        ImGui::Text("Angles: %zu", angles.size());
    }
    if (!redshifts.empty()) {
        ImGui::Text("Redshifts: %zu", redshifts.size());
    }

    ImGui::End();
}

void UI::renderParticlePanel(physics::Metric* metric) {
    ImGui::SetNextWindowPos(ImVec2(1530, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(350, 400), ImGuiCond_FirstUseEver);

    ImGui::Begin("Particle Trajectories", nullptr, ImGuiWindowFlags_None);

    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Geodesic Integration");
    ImGui::Separator();
    ImGui::Spacing();

    if (!metric) {
        ImGui::TextDisabled("No metric available");
        ImGui::End();
        return;
    }

    // Particle system active toggle
    bool active = particleSystem_.isActive();
    if (ImGui::Checkbox("System Active", &active)) {
        particleSystem_.setActive(active);
    }

    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Enable to integrate trajectories over time");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Display current trajectories
    size_t count = particleSystem_.getTrajectoryCount();
    ImGui::Text("Active trajectories: %zu", count);

    ImGui::Spacing();

    // Preset trajectories
    ImGui::Text("Add Preset Orbits:");

    double isco = metric->iscoRadius();
    double photonSphere = metric->photonSphereRadius();

    if (ImGui::Button("Stable Circular Orbit", ImVec2(-1, 0))) {
        float radius = static_cast<float>(isco) * 1.5f;
        particleSystem_.addCircularOrbit(radius, metric);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Add a stable circular orbit at 1.5x ISCO");
    }

    if (ImGui::Button("ISCO Orbit", ImVec2(-1, 0))) {
        particleSystem_.addCircularOrbit(static_cast<float>(isco), metric);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Innermost Stable Circular Orbit\nOrbits closer are unstable!");
    }

    if (ImGui::Button("Photon Sphere Orbit", ImVec2(-1, 0))) {
        particleSystem_.addCircularOrbit(static_cast<float>(photonSphere), metric);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Unstable orbit at photon sphere\nPhotons can orbit here!");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Elliptical Orbits:");

    static float periapsis = 8.0f;
    static float apoapsis = 15.0f;

    ImGui::SliderFloat("Periapsis", &periapsis, static_cast<float>(isco),  30.0f, "%.1f M");
    ImGui::SliderFloat("Apoapsis", &apoapsis, periapsis, 50.0f, "%.1f M");

    if (ImGui::Button("Add Elliptical Orbit", ImVec2(-1, 0))) {
        particleSystem_.addEllipticalOrbit(periapsis, apoapsis, metric);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Add an elliptical orbit with given periapsis/apoapsis");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Radial Trajectories:");

    static float infallRadius = 20.0f;
    ImGui::SliderFloat("Start Radius", &infallRadius, static_cast<float>(isco), 50.0f, "%.1f M");

    if (ImGui::Button("Radial Infall", ImVec2(-1, 0))) {
        particleSystem_.addRadialInfall(infallRadius, metric);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Particle falls straight into black hole\nShows time dilation and spaghettification!");
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Controls
    if (count > 0) {
        if (ImGui::Button("Clear All Trajectories", ImVec2(-1, 0))) {
            particleSystem_.clear();
        }
    }

    ImGui::Spacing();

    // Physics info
    ImGui::Separator();
    ImGui::Text("Critical Radii:");
    ImGui::Text("  Event horizon: %.2f M", metric->eventHorizonRadius());
    ImGui::Text("  Photon sphere: %.2f M", photonSphere);
    ImGui::Text("  ISCO: %.2f M", isco);

    ImGui::End();
}

void UI::endFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool UI::wantsCaptureMouse() const {
    ImGuiIO& io = ImGui::GetIO();
    return io.WantCaptureMouse;
}

bool UI::wantsCaptureKeyboard() const {
    ImGuiIO& io = ImGui::GetIO();
    return io.WantCaptureKeyboard;
}

void UI::cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

} // namespace core
} // namespace cosmic
