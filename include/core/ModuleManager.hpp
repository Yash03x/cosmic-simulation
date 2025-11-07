#pragma once

#include "Module.hpp"
#include <vector>
#include <memory>
#include <map>
#include <string>

namespace cosmic {
namespace core {

/**
 * @brief Manages all simulation modules
 *
 * Handles loading, switching, and lifecycle of different cosmic simulation modules
 */
class ModuleManager {
public:
    /**
     * @brief Construct module manager
     */
    ModuleManager();

    /**
     * @brief Register a new module
     * @param module Unique pointer to module
     */
    void registerModule(std::unique_ptr<Module> module);

    /**
     * @brief Get list of all registered modules
     */
    std::vector<std::string> getModuleNames() const;

    /**
     * @brief Get modules organized by category
     */
    std::map<std::string, std::vector<std::string>> getModulesByCategory() const;

    /**
     * @brief Switch to a different module
     * @param moduleName Name of module to activate
     * @return True if successful
     */
    bool switchModule(const std::string& moduleName);

    /**
     * @brief Get current active module
     */
    Module* getActiveModule() { return activeModule_; }
    const Module* getActiveModule() const { return activeModule_; }

    /**
     * @brief Get current active module name
     */
    std::string getActiveModuleName() const { return activeModuleName_; }

    /**
     * @brief Update active module
     */
    void update(float deltaTime);

    /**
     * @brief Render active module
     */
    void render(rendering::Camera& camera, int windowWidth, int windowHeight);

    /**
     * @brief Render active module UI
     */
    void renderUI();

    /**
     * @brief Render module selector UI
     * Allows user to switch between modules
     */
    void renderModuleSelector();

    /**
     * @brief Cleanup all modules
     */
    void cleanup();

    /**
     * @brief Get module by name
     */
    Module* getModule(const std::string& name);

    /**
     * @brief Check if a module exists
     */
    bool hasModule(const std::string& name) const;

private:
    std::vector<std::unique_ptr<Module>> modules_;
    std::map<std::string, size_t> moduleIndices_;  // name -> index in modules_
    Module* activeModule_;
    std::string activeModuleName_;
};

} // namespace core
} // namespace cosmic
