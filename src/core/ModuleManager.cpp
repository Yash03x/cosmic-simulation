#include "core/ModuleManager.hpp"
#include <imgui.h>
#include <algorithm>
#include <iostream>

namespace cosmic {
namespace core {

ModuleManager::ModuleManager()
    : activeModule_(nullptr),
      activeModuleName_("") {
}

void ModuleManager::registerModule(std::unique_ptr<Module> module) {
    if (!module) {
        std::cerr << "ERROR::ModuleManager::Cannot register null module\n";
        return;
    }

    std::string name = module->getName();

    // Check for duplicate names
    if (moduleIndices_.find(name) != moduleIndices_.end()) {
        std::cerr << "ERROR::ModuleManager::Module '" << name << "' already registered\n";
        return;
    }

    // Add to modules list
    size_t index = modules_.size();
    moduleIndices_[name] = index;
    modules_.push_back(std::move(module));

    std::cout << "ModuleManager: Registered module '" << name << "'\n";

    // If this is the first module, activate it
    if (modules_.size() == 1) {
        activeModule_ = modules_[0].get();
        activeModuleName_ = name;
        std::cout << "ModuleManager: Auto-activated first module '" << name << "'\n";
    }
}

std::vector<std::string> ModuleManager::getModuleNames() const {
    std::vector<std::string> names;
    names.reserve(modules_.size());

    for (const auto& module : modules_) {
        names.push_back(module->getName());
    }

    return names;
}

std::map<std::string, std::vector<std::string>> ModuleManager::getModulesByCategory() const {
    std::map<std::string, std::vector<std::string>> categorized;

    for (const auto& module : modules_) {
        std::string category = module->getCategory();
        categorized[category].push_back(module->getName());
    }

    return categorized;
}

bool ModuleManager::switchModule(const std::string& moduleName) {
    // Check if module exists
    auto it = moduleIndices_.find(moduleName);
    if (it == moduleIndices_.end()) {
        std::cerr << "ERROR::ModuleManager::Module '" << moduleName << "' not found\n";
        return false;
    }

    // Don't switch if already active
    if (activeModuleName_ == moduleName) {
        return true;
    }

    // Get the module
    size_t index = it->second;
    Module* newModule = modules_[index].get();

    // Switch active module
    activeModule_ = newModule;
    activeModuleName_ = moduleName;

    std::cout << "ModuleManager: Switched to module '" << moduleName << "'\n";

    // Reset the new module
    activeModule_->reset();

    return true;
}

void ModuleManager::update(float deltaTime) {
    if (activeModule_) {
        activeModule_->update(deltaTime);
    }
}

void ModuleManager::render(rendering::Camera& camera, int windowWidth, int windowHeight) {
    if (activeModule_) {
        activeModule_->render(camera, windowWidth, windowHeight);
    }
}

void ModuleManager::renderUI() {
    if (activeModule_) {
        activeModule_->renderUI();
    }
}

void ModuleManager::renderModuleSelector() {
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(320, 250), ImGuiCond_FirstUseEver);

    ImGui::Begin("Simulation Modules", nullptr, ImGuiWindowFlags_None);

    ImGui::TextWrapped("Select a cosmic phenomenon to explore:");
    ImGui::Separator();
    ImGui::Spacing();

    // Get modules organized by category
    auto categorized = getModulesByCategory();

    // Display modules by category
    for (const auto& [category, moduleNames] : categorized) {
        if (ImGui::CollapsingHeader(category.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            for (const auto& name : moduleNames) {
                bool isActive = (name == activeModuleName_);

                if (isActive) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
                }

                if (ImGui::Button((name + (isActive ? " (Active)" : "")).c_str(), ImVec2(-1, 0))) {
                    switchModule(name);
                }

                if (isActive) {
                    ImGui::PopStyleColor();
                }

                // Show description on hover
                if (ImGui::IsItemHovered()) {
                    Module* module = getModule(name);
                    if (module) {
                        ImGui::SetTooltip("%s", module->getDescription().c_str());
                    }
                }
            }
            ImGui::Spacing();
        }
    }

    // Current module info
    if (activeModule_) {
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "Active Module:");
        ImGui::Text("%s", activeModuleName_.c_str());
        ImGui::TextWrapped("%s", activeModule_->getDescription().c_str());
    }

    ImGui::End();
}

void ModuleManager::cleanup() {
    for (auto& module : modules_) {
        module->cleanup();
    }
    modules_.clear();
    moduleIndices_.clear();
    activeModule_ = nullptr;
    activeModuleName_ = "";
}

Module* ModuleManager::getModule(const std::string& name) {
    auto it = moduleIndices_.find(name);
    if (it == moduleIndices_.end()) {
        return nullptr;
    }
    return modules_[it->second].get();
}

bool ModuleManager::hasModule(const std::string& name) const {
    return moduleIndices_.find(name) != moduleIndices_.end();
}

} // namespace core
} // namespace cosmic
