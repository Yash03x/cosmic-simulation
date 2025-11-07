# Cosmic Simulator Architecture

This document describes the modular architecture of the Cosmic Simulator framework.

## 🏛️ Architecture Overview

The Cosmic Simulator follows a **modular plugin architecture** that separates the core framework from individual simulation modules.

```
┌─────────────────────────────────────────────────────────┐
│                   Application Layer                     │
│  (Window, Input, Main Loop, Camera)                    │
└──────────────────────┬──────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────┐
│                  ModuleManager                          │
│  - Module registration and lifecycle                    │
│  - Module switching                                     │
│  - Module selector UI                                   │
└──────────────────────┬──────────────────────────────────┘
                       │
        ┌──────────────┼──────────────┐
        │              │              │
┌───────▼──────┐ ┌────▼─────┐ ┌──────▼──────┐
│ Black Hole   │ │ Galaxy   │ │  Other      │
│   Module     │ │  Module  │ │  Modules... │
└──────────────┘ └──────────┘ └─────────────┘
     │                │              │
     └────────────────┼──────────────┘
                      │
        ┌─────────────┼─────────────┐
        │             │             │
┌───────▼────┐ ┌──────▼─────┐ ┌────▼─────┐
│  Physics   │ │ Rendering  │ │   UI     │
│  Systems   │ │  Systems   │ │ Systems  │
└────────────┘ └────────────┘ └──────────┘
```

---

## 📦 Core Components

### 1. Module Interface (`include/core/Module.hpp`)

Base interface that all simulation modules implement:

```cpp
class Module {
public:
    // Module identity
    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;
    virtual std::string getCategory() const = 0;

    // Lifecycle
    virtual bool initialize(GLFWwindow* window) = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render(Camera& camera, int w, int h) = 0;
    virtual void renderUI() = 0;
    virtual void cleanup() = 0;
    virtual void reset() = 0;

    // Input (optional)
    virtual void handleKeyboard(int key, int action) {}
    virtual void handleMouse(int button, int action) {}
};
```

**Responsibilities:**
- Define standard interface for all modules
- Ensure consistent lifecycle management
- Provide extension points for input handling

### 2. ModuleManager (`include/core/ModuleManager.hpp`)

Manages all registered modules:

```cpp
class ModuleManager {
public:
    void registerModule(std::unique_ptr<Module> module);
    bool switchModule(const std::string& name);

    Module* getActiveModule();
    std::vector<std::string> getModuleNames() const;

    void update(float deltaTime);
    void render(Camera& camera, int w, int h);
    void renderUI();
};
```

**Responsibilities:**
- Register and store all available modules
- Handle module switching
- Forward update/render calls to active module
- Provide module selector UI

### 3. Application (`include/core/Application.hpp`)

Main application framework:

**Responsibilities:**
- Window management (GLFW)
- Input handling
- Main game loop
- Camera control
- Integration with ModuleManager

---

## 🔌 Module Structure

Each module is self-contained and implements the Module interface:

```
modules/
└── your_module/
    ├── YourModule.hpp          # Module interface
    ├── YourModule.cpp          # Module implementation
    ├── physics/                # Module-specific physics
    │   ├── YourPhysics.hpp
    │   └── YourPhysics.cpp
    ├── rendering/              # Module-specific rendering
    │   ├── YourRenderer.hpp
    │   └── YourRenderer.cpp
    └── ui/                     # Module-specific UI
        ├── YourUI.hpp
        └── YourUI.cpp
```

### Module Lifecycle

```
┌──────────────┐
│ Registration │
└──────┬───────┘
       │
       ▼
┌──────────────┐
│ Initialize   │ ◄─── Module::initialize()
└──────┬───────┘
       │
       ▼
   ┌───────┐
   │ LOOP  │
   └───┬───┘
       │
       ├──► Update   ◄─── Module::update()
       │
       ├──► Render   ◄─── Module::render()
       │
       ├──► UI       ◄─── Module::renderUI()
       │
       └──► Back to LOOP

       ▼
┌──────────────┐
│  Cleanup     │ ◄─── Module::cleanup()
└──────────────┘
```

---

## 🎨 Rendering Architecture

### Rendering Pipeline

Each module controls its own rendering:

```cpp
void YourModule::render(Camera& camera, int width, int height) {
    // 1. Setup
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 2. Get view/projection matrices from camera
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 proj = camera.getProjectionMatrix(width, height);

    // 3. Render your scene
    useShader(yourShader);
    setUniforms(view, proj, ...);
    drawYourObjects();
}
```

### Shared Rendering Resources

- **Camera**: Provided by application, shared across modules
- **Shaders**: Module-specific or shared utilities
- **Buffers**: Module-managed VAO/VBO
- **Textures**: Module-managed

---

## 🧮 Physics Architecture

### Physics Integration

Each module implements its own physics:

```cpp
void YourModule::update(float deltaTime) {
    // 1. Update simulation time
    simulationTime_ += deltaTime;

    // 2. Physics calculations
    for (auto& object : objects_) {
        // Compute forces
        glm::vec3 force = calculateForce(object);

        // Integrate motion
        object.velocity += (force / object.mass) * deltaTime;
        object.position += object.velocity * deltaTime;
    }

    // 3. Handle collisions, boundaries, etc.
    handleConstraints();
}
```

### Shared Physics Utilities

Located in `include/physics/`:

- **Metric.hpp**: Base class for spacetime metrics
- **Schwarzschild.hpp**: Schwarzschild black hole metric
- **Kerr.hpp**: Kerr rotating black hole metric
- **Geodesic.hpp**: Geodesic integration utilities

Modules can use these or implement their own physics.

---

## 🖥️ UI Architecture

### ImGui Integration

Each module provides its own UI panels:

```cpp
void YourModule::renderUI() {
    ImGui::Begin("Your Module Controls");

    // Parameters
    if (ImGui::SliderFloat("Speed", &speed_, 0.1f, 10.0f)) {
        onSpeedChanged();
    }

    // Actions
    if (ImGui::Button("Reset")) {
        reset();
    }

    // Visualization options
    ImGui::Checkbox("Show Grid", &showGrid_);

    ImGui::End();
}
```

### UI Organization

- **Module Selector**: Provided by ModuleManager
- **Module Controls**: Each module's `renderUI()`
- **Shared Panels**: Stats, help, about (in Application)

---

## 📂 Directory Structure

```
cosmic-simulation/
├── include/                      # Header files
│   ├── core/                     # Core framework
│   │   ├── Module.hpp
│   │   ├── ModuleManager.hpp
│   │   ├── Application.hpp
│   │   ├── Window.hpp
│   │   └── UI.hpp
│   ├── modules/                  # Simulation modules
│   │   ├── blackhole/
│   │   │   └── BlackHoleModule.hpp
│   │   ├── galaxy/
│   │   │   └── GalaxyModule.hpp
│   │   └── ...
│   ├── physics/                  # Shared physics
│   │   ├── Metric.hpp
│   │   ├── Schwarzschild.hpp
│   │   └── Kerr.hpp
│   ├── rendering/                # Shared rendering
│   │   ├── Camera.hpp
│   │   ├── Shader.hpp
│   │   └── Renderer.hpp
│   └── tools/                    # Shared tools
│       ├── MeasurementTools.hpp
│       └── ParticleTrajectory.hpp
├── src/                          # Implementation files
│   ├── core/
│   ├── modules/
│   ├── physics/
│   ├── rendering/
│   └── tools/
├── shaders/                      # GLSL shaders
├── assets/                       # Textures, models, etc.
├── docs/                         # Documentation
│   ├── ARCHITECTURE.md
│   ├── MODULE_DEVELOPMENT.md
│   └── ...
├── CMakeLists.txt
└── README.md
```

---

## 🔄 Data Flow

### Initialization Phase

```
Application::initialize()
    ├─► ModuleManager::registerModule(BlackHoleModule)
    ├─► ModuleManager::registerModule(GalaxyModule)
    ├─► ModuleManager::registerModule(...)
    └─► ModuleManager::switchModule("Black Holes")
            └─► BlackHoleModule::initialize()
```

### Main Loop

```
while (running) {
    // 1. Input
    Application::processInput()
        └─► ModuleManager::activeModule->handleKeyboard()

    // 2. Update
    Application::update(deltaTime)
        └─► ModuleManager::update(deltaTime)
                └─► activeModule->update(deltaTime)

    // 3. Render
    Application::render()
        └─► ModuleManager::render(camera, w, h)
                └─► activeModule->render(camera, w, h)

    // 4. UI
    Application::renderUI()
        ├─► ModuleManager::renderModuleSelector()
        └─► ModuleManager::renderUI()
                └─► activeModule->renderUI()
}
```

---

## 🎯 Design Principles

### 1. **Separation of Concerns**

- Modules are independent
- Core framework is module-agnostic
- Physics, rendering, and UI are decoupled

### 2. **Plugin Architecture**

- Modules can be added without modifying core
- Module registration is simple
- Easy to enable/disable modules

### 3. **Extensibility**

- New modules implement Module interface
- Shared utilities available but optional
- Custom physics/rendering per module

### 4. **Maintainability**

- Clear directory structure
- Module-specific code is isolated
- Shared code is in common libraries

### 5. **Performance**

- Only active module is updated/rendered
- Modules manage their own resources
- LOD and optimization per module

---

## 🚀 Adding a New Module

See [MODULE_DEVELOPMENT.md](MODULE_DEVELOPMENT.md) for detailed guide.

**Quick Steps:**

1. Create module files in `include/modules/your_module/`
2. Implement Module interface
3. Add to CMakeLists.txt
4. Register in Application::initialize()
5. Done! Module appears in selector

---

## 📊 Performance Considerations

### Module Resource Management

- Initialize resources in `initialize()`
- Clean up in `cleanup()`
- Lazy-load expensive resources
- Unload when module is inactive

### Update Optimization

- Use delta time for frame-rate independence
- Implement physics substeps if needed
- Profile and optimize hot paths

### Render Optimization

- Frustum culling
- Level of detail (LOD)
- Batch rendering
- Shader optimization

---

## 🔒 Thread Safety

Currently single-threaded, but future considerations:

- Physics updates could be threaded
- Particle systems could be parallel
- Resource loading could be async

---

## 🧪 Testing

### Module Testing

Each module should be testable independently:

```cpp
TEST(YourModuleTest, Initialization) {
    YourModule module;
    EXPECT_TRUE(module.initialize(mockWindow));
}

TEST(YourModuleTest, PhysicsUpdate) {
    YourModule module;
    module.initialize(mockWindow);
    module.update(0.016f);
    // Verify state changes
}
```

### Integration Testing

Test module switching:

```cpp
TEST(ModuleManagerTest, SwitchModules) {
    ModuleManager mgr;
    mgr.registerModule(std::make_unique<BlackHoleModule>());
    mgr.registerModule(std::make_unique<GalaxyModule>());

    EXPECT_TRUE(mgr.switchModule("Black Holes"));
    EXPECT_EQ(mgr.getActiveModuleName(), "Black Holes");
}
```

---

## 📚 Further Reading

- [Module Development Guide](MODULE_DEVELOPMENT.md)
- [Physics System Documentation](PHYSICS.md)
- [Rendering Pipeline](RENDERING.md)
- [UI Guidelines](UI_GUIDELINES.md)

---

For questions or contributions, see [README.md](../README.md).
