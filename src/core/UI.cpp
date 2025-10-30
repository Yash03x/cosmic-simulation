#include "core/UI.hpp"
#include "physics/Schwarzschild.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <iostream>

namespace cosmic {
namespace core {

UI::UI(GLFWwindow* window)
    : window_(window),
      visible_(true),
      blackHoleMass_(1.0f),
      blackHoleMassChanged_(false),
      maxRaySteps_(1000),
      stepSize_(0.1f),
      accretionDiskEnabled_(false),
      cameraSpeed_(10.0f),
      showDemoWindow_(false) {
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
                float fps) {
    if (!visible_) return;

    // Reset change flags
    blackHoleMassChanged_ = false;

    // Render main control panel
    renderControlPanel(camera, metric, renderer);

    // Render statistics panel
    renderStatsPanel(camera, metric, deltaTime, fps);

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

    ImGui::Text("Black Hole Simulator");
    ImGui::Separator();

    // Black hole parameters
    if (ImGui::CollapsingHeader("Black Hole Parameters", ImGuiTreeNodeFlags_DefaultOpen)) {
        float oldMass = blackHoleMass_;
        ImGui::SliderFloat("Mass (M☉)", &blackHoleMass_, 0.1f, 10.0f, "%.2f");
        if (oldMass != blackHoleMass_) {
            blackHoleMassChanged_ = true;
        }

        if (metric) {
            ImGui::Text("Event Horizon: %.2f M", metric->eventHorizonRadius());
            ImGui::Text("Photon Sphere: %.2f M", metric->photonSphereRadius());
            ImGui::Text("ISCO: %.2f M", metric->iscoRadius());
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
            ImGui::Text("Coming soon: Full implementation");
            ImGui::Text("- Temperature profile");
            ImGui::Text("- Doppler shifts");
            ImGui::Text("- Relativistic beaming");
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Presets
    if (ImGui::Button("Stellar Mass BH (3 M☉)")) {
        blackHoleMass_ = 3.0f;
        blackHoleMassChanged_ = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Supermassive BH (10 M☉)")) {
        blackHoleMass_ = 10.0f;
        blackHoleMassChanged_ = true;
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
                          float fps) {
    ImGui::SetNextWindowPos(ImVec2(10, 430), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(350, 200), ImGuiCond_FirstUseEver);

    ImGui::Begin("Statistics", nullptr, ImGuiWindowFlags_None);

    // Performance
    ImGui::Text("Performance");
    ImGui::Separator();
    ImGui::Text("FPS: %.1f", fps);
    ImGui::Text("Frame time: %.2f ms", deltaTime * 1000.0f);
    ImGui::Text("GPU time: TBD");

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
