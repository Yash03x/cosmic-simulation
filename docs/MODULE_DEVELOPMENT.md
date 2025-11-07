# Module Development Guide

This guide explains how to add new simulation modules to the Cosmic Simulator framework.

## 📋 Table of Contents

- [Architecture Overview](#architecture-overview)
- [Creating a New Module](#creating-a-new-module)
- [Module Interface](#module-interface)
- [Integration Steps](#integration-steps)
- [Best Practices](#best-practices)
- [Examples](#examples)

---

## 🏗️ Architecture Overview

The Cosmic Simulator uses a **modular plugin architecture** where different cosmic phenomena are implemented as independent modules:

```
cosmic-simulation/
├── include/
│   ├── core/
│   │   ├── Module.hpp          # Base module interface
│   │   └── ModuleManager.hpp   # Module management system
│   └── modules/
│       ├── blackhole/          # Black hole simulation module
│       │   └── BlackHoleModule.hpp
│       ├── galaxy/             # Galaxy simulation module
│       │   └── GalaxyModule.hpp
│       └── your_module/        # Your new module!
│           └── YourModule.hpp
└── src/
    └── modules/
        ├── blackhole/
        │   └── BlackHoleModule.cpp
        ├── galaxy/
        │   └── GalaxyModule.cpp
        └── your_module/
            └── YourModule.cpp
```

### Key Components

- **Module**: Base interface that all modules implement
- **ModuleManager**: Handles module registration, switching, and lifecycle
- **Application**: Main application that uses ModuleManager

---

## 🆕 Creating a New Module

### Step 1: Create Module Files

Create header and implementation files for your module:

```bash
# Create directories
mkdir -p include/modules/your_module
mkdir -p src/modules/your_module

# Create files
touch include/modules/your_module/YourModule.hpp
touch src/modules/your_module/YourModule.cpp
```

### Step 2: Define Module Header

**include/modules/your_module/YourModule.hpp**:

```cpp
#pragma once

#include "../../core/Module.hpp"
#include "../../rendering/Camera.hpp"

namespace cosmic {
namespace modules {
namespace your_module {

class YourModule : public core::Module {
public:
    YourModule();
    ~YourModule() override;

    // Required module interface methods
    std::string getName() const override {
        return "Your Module Name";
    }

    std::string getDescription() const override {
        return "Brief description of what your module does";
    }

    std::string getCategory() const override {
        return "Category Name";  // e.g., "Stellar", "Cosmology", etc.
    }

    bool initialize(GLFWwindow* window) override;
    void update(float deltaTime) override;
    void render(rendering::Camera& camera, int windowWidth, int windowHeight) override;
    void renderUI() override;
    void cleanup() override;
    void reset() override;

    // Optional methods
    void handleKeyboard(int key, int action) override;
    void handleMouse(int button, int action) override;
    bool wantsCaptureMouse() const override;
    bool wantsCaptureKeyboard() const override;

private:
    GLFWwindow* window_;
    bool initialized_;

    // Your module-specific data and methods
    void initializeSimulation();
    void updatePhysics(float deltaTime);
};

} // namespace your_module
} // namespace modules
} // namespace cosmic
```

### Step 3: Implement Module

**src/modules/your_module/YourModule.cpp**:

```cpp
#include "modules/your_module/YourModule.hpp"
#include <imgui.h>
#include <iostream>

namespace cosmic {
namespace modules {
namespace your_module {

YourModule::YourModule()
    : window_(nullptr),
      initialized_(false) {
}

YourModule::~YourModule() {
    cleanup();
}

bool YourModule::initialize(GLFWwindow* window) {
    window_ = window;

    std::cout << "YourModule: Initializing...\n";

    // Initialize your simulation here
    initializeSimulation();

    initialized_ = true;
    std::cout << "YourModule: Initialization complete\n";

    return true;
}

void YourModule::update(float deltaTime) {
    if (!initialized_) return;

    // Update your simulation physics
    updatePhysics(deltaTime);
}

void YourModule::render(rendering::Camera& camera, int windowWidth, int windowHeight) {
    if (!initialized_) return;

    // Render your simulation
    // Use OpenGL calls here
}

void YourModule::renderUI() {
    if (!initialized_) return;

    ImGui::Begin("Your Module Control Panel");

    // Add ImGui controls here
    ImGui::Text("This is your module!");

    if (ImGui::Button("Reset")) {
        reset();
    }

    ImGui::End();
}

void YourModule::cleanup() {
    // Clean up resources
    initialized_ = false;
    std::cout << "YourModule: Cleanup complete\n";
}

void YourModule::reset() {
    // Reset simulation to initial state
    initializeSimulation();
}

void YourModule::initializeSimulation() {
    // Initialize your simulation data
}

void YourModule::updatePhysics(float deltaTime) {
    // Update physics simulation
}

} // namespace your_module
} // namespace modules
} // namespace cosmic
```

### Step 4: Register Module

Update **CMakeLists.txt** to include your module:

```cmake
# Module files
set(MODULE_SOURCES
    # Black Hole Module
    src/modules/blackhole/BlackHoleModule.cpp

    # Galaxy Module
    src/modules/galaxy/GalaxyModule.cpp

    # Your Module
    src/modules/your_module/YourModule.cpp
)
```

### Step 5: Register in Application

In your main application code, register the module:

```cpp
#include "modules/your_module/YourModule.hpp"

// In Application::initialize() or similar:
moduleManager_.registerModule(std::make_unique<modules::your_module::YourModule>());
```

---

## 🔌 Module Interface

### Required Methods

All modules **must** implement these methods:

```cpp
// Module identification
std::string getName() const override;
std::string getDescription() const override;
std::string getCategory() const override;

// Lifecycle
bool initialize(GLFWwindow* window) override;
void update(float deltaTime) override;
void render(rendering::Camera& camera, int windowWidth, int windowHeight) override;
void renderUI() override;
void cleanup() override;
void reset() override;
```

### Optional Methods

These methods have default implementations but can be overridden:

```cpp
// Input handling
void handleKeyboard(int key, int action) override;
void handleMouse(int button, int action) override;

// Input capture
bool wantsCaptureMouse() const override;
bool wantsCaptureKeyboard() const override;
```

---

## 🔧 Integration Steps

### 1. Physics Simulation

If your module needs physics calculations:

```cpp
// In your module header
private:
    float simulationTime_;
    std::vector<PhysicsObject> objects_;

// In update()
void YourModule::update(float deltaTime) {
    simulationTime_ += deltaTime;

    // Update each object
    for (auto& obj : objects_) {
        obj.update(deltaTime);
    }
}
```

### 2. OpenGL Rendering

For rendering with OpenGL:

```cpp
// In your module header
private:
    GLuint vao_, vbo_;
    GLuint shaderProgram_;

// Initialize OpenGL resources
bool YourModule::initialize(GLFWwindow* window) {
    // Create VAO/VBO
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);

    // Load shaders
    shaderProgram_ = createShaderProgram();

    return true;
}

// Render
void YourModule::render(rendering::Camera& camera, int windowWidth, int windowHeight) {
    glUseProgram(shaderProgram_);

    // Set uniforms
    setUniform("view", camera.getViewMatrix());
    setUniform("projection", camera.getProjectionMatrix());

    // Draw
    glBindVertexArray(vao_);
    glDrawArrays(GL_POINTS, 0, numVertices);
}
```

### 3. UI Controls

Use ImGui for user interface:

```cpp
void YourModule::renderUI() {
    ImGui::Begin("Module Controls");

    // Sliders
    ImGui::SliderFloat("Parameter", &param_, 0.0f, 1.0f);

    // Buttons
    if (ImGui::Button("Action")) {
        performAction();
    }

    // Collapsing headers
    if (ImGui::CollapsingHeader("Advanced Options")) {
        // Advanced controls here
    }

    ImGui::End();
}
```

---

## ✅ Best Practices

### 1. **Namespace Organization**

```cpp
namespace cosmic {
namespace modules {
namespace your_module {
    // Your code here
}}}
```

### 2. **Resource Management**

Always clean up resources in `cleanup()`:

```cpp
void YourModule::cleanup() {
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (shaderProgram_) glDeleteProgram(shaderProgram_);
}
```

### 3. **Initialization Checks**

Check initialization state:

```cpp
void YourModule::update(float deltaTime) {
    if (!initialized_) return;
    // ... rest of code
}
```

### 4. **Error Handling**

Return false from initialize() on errors:

```cpp
bool YourModule::initialize(GLFWwindow* window) {
    if (!initializePhysics()) {
        std::cerr << "ERROR::YourModule::Physics init failed\n";
        return false;
    }
    return true;
}
```

### 5. **Performance**

- Use delta time for physics updates
- Implement level-of-detail for large simulations
- Profile and optimize render loops

---

## 📚 Examples

### Example 1: Solar System Module

```cpp
class SolarSystemModule : public Module {
public:
    std::string getName() const override { return "Solar System"; }
    std::string getCategory() const override { return "Stellar"; }

private:
    std::vector<Planet> planets_;
    Sun sun_;

    void updateOrbits(float deltaTime);
    void renderPlanets(const Camera& camera);
};
```

### Example 2: Supernova Module

```cpp
class SupernovaModule : public Module {
public:
    std::string getName() const override { return "Supernova"; }
    std::string getCategory() const override { return "Stellar"; }

private:
    ParticleSystem explosionParticles_;
    float explosionTime_;

    void simulateExplosion(float deltaTime);
    void renderShockwave();
};
```

### Example 3: Dark Matter Module

```cpp
class DarkMatterModule : public Module {
public:
    std::string getName() const override { return "Dark Matter Halo"; }
    std::string getCategory() const override { return "Cosmology"; }

private:
    std::vector<DMParticle> dmParticles_;

    void updateNBody(float deltaTime);
    void renderDensityField();
};
```

---

## 🎯 Module Categories

Organize modules into logical categories:

- **Compact Objects**: Black holes, neutron stars, white dwarfs
- **Galaxies**: Spiral, elliptical, galaxy collisions
- **Stellar**: Stars, planetary systems, supernovae
- **Cosmology**: Large-scale structure, CMB, dark matter
- **Solar System**: Planets, moons, asteroids
- **Nebulae**: Emission nebulae, planetary nebulae
- **Educational**: Simplified demonstrations, tutorials

---

## 🚀 Next Steps

1. **Read existing modules**: Study `BlackHoleModule` and `GalaxyModule`
2. **Start simple**: Create a basic module first
3. **Iterate**: Add features incrementally
4. **Test thoroughly**: Ensure stability and performance
5. **Document**: Add comments and tooltips

Happy coding! 🌌

---

For questions or contributions, please see the main [README.md](../README.md).
