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
      neutronStar_(nullptr),
      selectedNeutronStarIndex_(0),
      selectedGWEventIndex_(0),
      showDemoWindow_(false) {
    // Load all available presets
    availablePresets_ = BlackHolePresets::getAllPresets();
    // Initialize with first preset (M87*)
    if (!availablePresets_.empty()) {
        selectedPreset_ = availablePresets_[0];
    }

    // Load neutron star presets
    availableNeutronStars_ = NeutronStarPresets::getAllPresets();

    // Load gravitational wave events
    availableGWEvents_ = GravitationalWavePresets::getAllPresets();
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

    // Render neutron star panel
    renderNeutronStarPanel();

    // Render gravitational wave panel
    renderGravitationalWavePanel();

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
    ImGui::Text("Photon Paths (Light Rays):");

    static float photonStartRadius = 15.0f;
    static float photonAngle = 45.0f;

    ImGui::SliderFloat("Launch Radius", &photonStartRadius, static_cast<float>(isco), 40.0f, "%.1f M");
    ImGui::SliderFloat("Launch Angle", &photonAngle, -90.0f, 90.0f, "%.1f°");

    if (ImGui::Button("Launch Photon (Tangential)", ImVec2(-1, 0))) {
        // Photon moving tangentially
        ParticleInitialConditions ic;
        float theta = glm::radians(photonAngle);
        ic.position = glm::vec3(photonStartRadius * cos(theta), 0.0f, photonStartRadius * sin(theta));
        ic.velocity = glm::normalize(glm::vec3(-sin(theta), 0.0f, cos(theta)));  // Tangent direction
        ic.isMassive = false;  // Photon!
        ic.energy = 1.0f;
        ic.angularMomentum = photonStartRadius;
        particleSystem_.addParticle(ic, metric);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Launch a photon tangent to the circle\nShows gravitational lensing!");
    }

    if (ImGui::Button("Launch Photon (Radial)", ImVec2(-1, 0))) {
        // Photon moving radially inward
        ParticleInitialConditions ic;
        float theta = glm::radians(photonAngle);
        ic.position = glm::vec3(photonStartRadius * cos(theta), 0.0f, photonStartRadius * sin(theta));
        ic.velocity = -glm::normalize(ic.position);  // Inward
        ic.isMassive = false;  // Photon!
        ic.energy = 1.0f;
        ic.angularMomentum = 0.0f;
        particleSystem_.addParticle(ic, metric);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Launch a photon directly toward black hole\nDemonstrates light capture!");
    }

    if (ImGui::Button("Photon Ring (Multiple Rays)", ImVec2(-1, 0))) {
        // Launch multiple photons in a ring pattern
        int numPhotons = 12;
        for (int i = 0; i < numPhotons; i++) {
            float angle = (2.0f * glm::pi<float>() * i) / numPhotons;
            ParticleInitialConditions ic;
            ic.position = glm::vec3(photonStartRadius * cos(angle), 0.0f, photonStartRadius * sin(angle));
            ic.velocity = glm::normalize(glm::vec3(-sin(angle), 0.1f, cos(angle)));  // Slight inward component
            ic.isMassive = false;
            ic.energy = 1.0f;
            ic.angularMomentum = photonStartRadius * 0.9f;
            particleSystem_.addParticle(ic, metric);
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Launch 12 photons in a ring pattern\nBeautiful gravitational lensing!");
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

void UI::renderNeutronStarPanel() {
    ImGui::SetNextWindowPos(ImVec2(1530, 430), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(350, 450), ImGuiCond_FirstUseEver);

    ImGui::Begin("Neutron Stars", nullptr, ImGuiWindowFlags_None);

    ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Ultra-Dense Compact Objects");
    ImGui::Separator();
    ImGui::Spacing();

    // Neutron star visibility toggle
    bool visible = (neutronStar_ != nullptr && neutronStar_->isVisible());
    bool previousVisible = visible;

    if (ImGui::Checkbox("Show Neutron Star", &visible)) {
        if (visible && !previousVisible) {
            // Create neutron star if it doesn't exist
            if (!neutronStar_ && !availableNeutronStars_.empty()) {
                neutronStar_ = std::make_shared<NeutronStar>(availableNeutronStars_[selectedNeutronStarIndex_]);
                neutronStar_->setPosition(glm::vec3(50.0f, 0.0f, 0.0f));  // Position to the side
            }
            if (neutronStar_) {
                neutronStar_->setVisible(true);
            }
        } else if (!visible && previousVisible) {
            if (neutronStar_) {
                neutronStar_->setVisible(false);
            }
        }
    }

    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Toggle neutron star visibility\nWill appear alongside black hole");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Preset selection
    ImGui::Text("Select Neutron Star:");

    std::vector<const char*> nsNames;
    for (const auto& ns : availableNeutronStars_) {
        nsNames.push_back(ns.name.c_str());
    }

    int previousIndex = selectedNeutronStarIndex_;
    if (ImGui::Combo("##NSPreset", &selectedNeutronStarIndex_, nsNames.data(), nsNames.size())) {
        if (selectedNeutronStarIndex_ != previousIndex &&
            selectedNeutronStarIndex_ >= 0 &&
            selectedNeutronStarIndex_ < static_cast<int>(availableNeutronStars_.size())) {

            // Create new neutron star with selected preset
            neutronStar_ = std::make_shared<NeutronStar>(availableNeutronStars_[selectedNeutronStarIndex_]);
            neutronStar_->setPosition(glm::vec3(50.0f, 0.0f, 0.0f));
            neutronStar_->setVisible(visible);
        }
    }

    ImGui::Spacing();

    // Display current neutron star info
    if (neutronStar_ && selectedNeutronStarIndex_ >= 0 &&
        selectedNeutronStarIndex_ < static_cast<int>(availableNeutronStars_.size())) {

        const auto& props = availableNeutronStars_[selectedNeutronStarIndex_];

        ImGui::Separator();
        ImGui::TextWrapped("%s", props.description.c_str());
        ImGui::Spacing();

        // Physical properties
        ImGui::Text("Physical Properties:");
        ImGui::BulletText("Mass: %.2f M☉", props.mass);
        ImGui::BulletText("Radius: %.1f km", props.radius);
        ImGui::BulletText("Rotation: %.4f sec", props.rotationPeriod);

        if (props.rotationPeriod < 0.01f) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "(%.0f Hz!)", 1.0f / props.rotationPeriod);
        }

        ImGui::BulletText("Magnetic Field: %.1e T", props.magneticFieldStrength);
        ImGui::BulletText("Surface Temp: %.1e K", props.surfaceTemperature);

        ImGui::Spacing();

        // Extreme conditions
        ImGui::Separator();
        ImGui::Text("Extreme Conditions:");

        float surfaceGravity = neutronStar_->getSurfaceGravity();
        ImGui::BulletText("Surface gravity: %.2e g", surfaceGravity);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("That's %.0f billion times Earth gravity!", surfaceGravity / 1e9f);
        }

        float surfaceRedshift = neutronStar_->getSurfaceRedshift();
        ImGui::BulletText("Gravitational redshift: z = %.3f", surfaceRedshift);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Light leaving surface is shifted to longer wavelengths\nTime dilation factor: %.2fx", 1.0f + surfaceRedshift);
        }

        float escapeVelocity = std::sqrt(1.0f - 1.0f / ((1.0f + surfaceRedshift) * (1.0f + surfaceRedshift)));
        ImGui::BulletText("Escape velocity: %.2f%% c", escapeVelocity * 100.0f);

        ImGui::Spacing();

        // Visual features
        ImGui::Separator();
        ImGui::Text("Visual Features:");
        if (props.hasHotspots) {
            ImGui::BulletText("Magnetic pole hotspots");
        }
        if (props.hasMountains) {
            ImGui::BulletText("Crustal mountains (<5mm!)");
        }
        if (props.crustalDeformation > 0.01f) {
            ImGui::BulletText("Oblate shape (%.1f%% deformation)", props.crustalDeformation * 100.0f);
        }

        ImGui::Spacing();

        // Fun facts
        ImGui::Separator();
        ImGui::Text("Fun Facts:");

        if (props.name.find("PSR J1748") != std::string::npos) {
            ImGui::BulletText("Spins 716 times per second!");
            ImGui::BulletText("Surface moving at 24% light speed");
        } else if (props.name.find("Crab") != std::string::npos) {
            ImGui::BulletText("Visible supernova from 1054 AD");
            ImGui::BulletText("Chinese astronomers recorded it");
        } else if (props.name.find("Magnetar") != std::string::npos) {
            ImGui::BulletText("Magnetic field tears atoms apart");
            ImGui::BulletText("Would erase credit cards from 1000 km!");
        } else if (props.name.find("Tolman") != std::string::npos) {
            ImGui::BulletText("Any heavier = black hole");
            ImGui::BulletText("At the edge of stability");
        }

        ImGui::Spacing();

        // Density comparison
        ImGui::Separator();
        ImGui::Text("Density:");
        ImGui::TextWrapped("A teaspoon of neutron star material weighs ~1 billion tons!");
        ImGui::TextWrapped("That's like compressing Mount Everest into a sugar cube.");
    }

    ImGui::End();
}

void UI::renderGravitationalWavePanel() {
    ImGui::SetNextWindowPos(ImVec2(370, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(380, 500), ImGuiCond_FirstUseEver);

    ImGui::Begin("Gravitational Waves", nullptr, ImGuiWindowFlags_None);

    ImGui::TextColored(ImVec4(0.3f, 1.0f, 1.0f, 1.0f), "Spacetime Ripples from LIGO");
    ImGui::Separator();
    ImGui::Spacing();

    // System active toggle
    bool active = gwSystem_.isActive();
    if (ImGui::Checkbox("Show Gravitational Waves", &active)) {
        gwSystem_.setActive(active);
        if (active) {
            gwSystem_.clear();  // Clear old waves when activating
        }
    }

    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Visualize spacetime ripples\nPropagating at the speed of light!");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Event selection
    ImGui::Text("Select LIGO/Virgo Detection:");

    std::vector<const char*> gwNames;
    for (const auto& event : availableGWEvents_) {
        gwNames.push_back(event.name.c_str());
    }

    int previousIndex = selectedGWEventIndex_;
    if (ImGui::Combo("##GWEvent", &selectedGWEventIndex_, gwNames.data(), gwNames.size())) {
        if (selectedGWEventIndex_ != previousIndex &&
            selectedGWEventIndex_ >= 0 &&
            selectedGWEventIndex_ < static_cast<int>(availableGWEvents_.size())) {

            gwSystem_.setEvent(availableGWEvents_[selectedGWEventIndex_]);
            gwSystem_.clear();
        }
    }

    ImGui::Spacing();

    // Display current event info
    if (selectedGWEventIndex_ >= 0 &&
        selectedGWEventIndex_ < static_cast<int>(availableGWEvents_.size())) {

        const auto& event = availableGWEvents_[selectedGWEventIndex_];

        ImGui::Separator();
        ImGui::TextWrapped("%s", event.description.c_str());
        ImGui::Spacing();

        ImGui::Text("Detected: %s", event.detectionDate.c_str());
        ImGui::Spacing();

        // Source properties
        ImGui::Separator();
        ImGui::Text("Source Properties:");
        ImGui::BulletText("Primary mass: %.1f M☉", event.mass1);
        ImGui::BulletText("Secondary mass: %.1f M☉", event.mass2);
        ImGui::BulletText("Final mass: %.1f M☉", event.finalMass);

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f),
                          "Energy radiated: %.1f M☉c²", event.energyRadiated);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("That's %.0e Joules!\nEquivalent to the Sun's entire mass-energy!",
                             event.energyRadiated * 1.8e47);
        }

        ImGui::BulletText("Distance: %.0f Mpc", event.distance);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Megaparsecs: %.0f million light-years", event.distance * 3.26);
        }

        ImGui::Spacing();

        // Waveform properties
        ImGui::Separator();
        ImGui::Text("Gravitational Wave Properties:");
        ImGui::BulletText("Peak frequency: %.0f Hz", event.peakFrequency);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Spacetime oscillates this many times per second!");
        }

        ImGui::BulletText("Peak strain: %.1e", event.peakStrain);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Fractional change in distance:\nΔL/L ~ 10^-21\nSmaller than a proton compared to Sun-Earth distance!");
        }

        ImGui::BulletText("Chirp mass: %.1f M☉", event.chirpMass);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("M_chirp = (m1·m2)^0.6 / (m1+m2)^0.2\nDetermines frequency evolution");
        }

        ImGui::BulletText("Observable duration: %.1f sec", event.duration);

        ImGui::Spacing();

        // Event type
        ImGui::Separator();
        ImGui::Text("Event Type:");
        switch (event.type) {
            case GWEventProperties::Type::BinaryBlackHole:
                ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Binary Black Hole (BBH)");
                break;
            case GWEventProperties::Type::BinaryNeutronStar:
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Binary Neutron Star (BNS)");
                ImGui::Text("Had electromagnetic counterpart!");
                break;
            case GWEventProperties::Type::NeutronStarBlackHole:
                ImGui::TextColored(ImVec4(0.8f, 0.3f, 1.0f, 1.0f), "Neutron Star - Black Hole (NSBH)");
                break;
        }

        ImGui::Spacing();

        // Visualization controls
        ImGui::Separator();
        ImGui::Text("Visualization:");

        int vizMode = static_cast<int>(gwSystem_.getVisualizationMode());
        const char* vizModes[] = {"Rings", "Grid", "Rings + Grid"};
        if (ImGui::Combo("Mode", &vizMode, vizModes, 3)) {
            gwSystem_.setVisualizationMode(static_cast<GravitationalWaveSystem::VisualizationMode>(vizMode));
        }

        bool chirp = gwSystem_.isChirpEnabled();
        if (ImGui::Checkbox("Frequency Chirp", &chirp)) {
            gwSystem_.setChirpEnabled(chirp);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Frequency increases as objects spiral in\nClassic 'chirp' sound in LIGO data");
        }

        if (ImGui::Button("Emit Wave Pulse", ImVec2(-1, 0))) {
            gwSystem_.emitPulse();
        }

        if (ImGui::Button("Clear All Waves", ImVec2(-1, 0))) {
            gwSystem_.clear();
        }

        ImGui::Spacing();

        // Active pulses
        size_t pulseCount = gwSystem_.getActivePulseCount();
        ImGui::Text("Active wave pulses: %zu", pulseCount);

        // Fun facts
        if (event.name == "GW150914") {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.3f, 1.0f), "Historic Moment!");
            ImGui::TextWrapped("First direct detection of gravitational waves, confirming Einstein's 1916 prediction!");
            ImGui::TextWrapped("Nobel Prize in Physics 2017");
        } else if (event.name == "GW170817") {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.3f, 1.0f), "Multi-Messenger Astronomy!");
            ImGui::TextWrapped("First GW event with EM counterpart: gamma-rays, optical, X-rays, radio!");
            ImGui::TextWrapped("Produced heavy elements (gold, platinum)");
        }
    }

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
