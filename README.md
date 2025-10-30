# Cosmic Simulator - Physics-Accurate Black Hole Visualization

A real-time 3D black hole simulator built with C++ and OpenGL, featuring physically accurate gravitational lensing, accretion disk visualization, and interactive parameter control.

## Features

### ✨ Current Implementation

**Physics Engine:**
- ⚫ **Schwarzschild Metric** - Non-rotating black hole simulation
- 🌀 **Geodesic Ray Tracing** - RK4 integration for accurate light paths
- 🔵 **Gravitational Lensing** - Real-time spacetime curvature effects
- 📐 **Proper Coordinate Transforms** - Accurate Cartesian ↔ Spherical conversions

**Accretion Disk:**
- 💿 **Realistic Geometry** - ISCO (r = 6M) to outer radius
- 🌡️  **Temperature Profile** - Shakura-Sunyaev model
- 🎨 **Blackbody Radiation** - Physics-based color temperature
- 🔴🔵 **Doppler Shifts** - Red/blue shifting from orbital motion
- ⚡ **Relativistic Beaming** - Brightness enhancement effects

**Rendering:**
- 🌟 **Procedural Starfield** - Multi-layer star generation
- 🎬 **Real-time Ray Tracing** - Fragment shader-based geodesic integration
- 🖼️  **Tone Mapping** - Reinhard + gamma correction
- 🎮 **Interactive UI** - ImGui-based parameter controls

**User Interface:**
- 🎛️  **Live Parameter Adjustment** - Black hole mass, ray tracing quality
- 📊 **Real-time Statistics** - FPS, position, physics warnings
- ⌨️  **Keyboard + Mouse** - Full 3D navigation
- 📋 **Presets** - Stellar mass vs supermassive black holes

### 🚀 Planned Features

- **Kerr Metric** - Rotating black holes with frame dragging
- **Neutron Stars** - Surface rendering with magnetic fields
- **Pulsars** - Rotating beam emission
- **Binary Systems** - Two black holes orbiting
- **Skybox Textures** - Real astronomical images
- **GPU Optimization** - Compute shaders for physics
- **Video Export** - Record camera paths

## Requirements

- **macOS** 10.15+ (or Linux/Windows with modifications)
- **CMake** 3.20+
- **C++17** compiler
- **OpenGL 4.1+**

## Dependencies

Install via Homebrew (macOS):
```bash
brew install cmake glfw glm eigen
```

Or via package manager on Linux:
```bash
# Ubuntu/Debian
sudo apt install cmake libglfw3-dev libglm-dev libeigen3-dev

# Arch
sudo pacman -S cmake glfw-x11 glm eigen
```

## Building

```bash
# Clone repository
cd cosmic-simulation

# Install GLAD (OpenGL loader)
python3 -m pip install --user glad
python3 -m glad --profile=core --api="gl=4.1" --generator=c --spec=gl --out-path=external/glad

# Download ImGui (already included in repo)
# Already cloned in external/imgui

# Download stb_image (already included)
# Already in external/stb

# Build
mkdir build && cd build
cmake ..
cmake --build .

# Run
./CosmicSimulator
```

## Controls

### Camera
- **WASD** - Move forward/left/back/right
- **Q/E** - Move down/up
- **Mouse** - Look around (press TAB to capture/release cursor)
- **SHIFT** - Speed boost (2x faster movement)

### UI
- **H** - Toggle UI visibility
- **TAB** - Toggle mouse capture
- **P** - Print statistics to console
- **ESC** - Exit application

### UI Panels
- **Black Hole Parameters** - Adjust mass (0.1 - 10 M☉)
- **Ray Tracing** - Max steps, step size tuning
- **Camera** - Speed control, position reset
- **Accretion Disk** - Enable/disable, see parameters
- **Statistics** - FPS, frame time, position info

## Physics

### Geometric Units (G = c = 1)

All calculations use geometric units where the gravitational constant and speed of light equal 1, simplifying equations dramatically.

**Conversion:**
- 1 M☉ (solar mass) ≈ 1.477 km ≈ 4.926×10⁻⁶ seconds

### Key Radii

For a black hole of mass M:

| Region | Radius | Description |
|--------|--------|-------------|
| **Event Horizon** | r = 2M | Point of no return |
| **Photon Sphere** | r = 3M | Unstable orbit for light |
| **ISCO** | r = 6M | Innermost stable circular orbit |

### Schwarzschild Metric

```
ds² = -(1 - 2M/r)dt² + (1 - 2M/r)⁻¹dr² + r²(dθ² + sin²θ dφ²)
```

### Geodesic Equation

Light paths follow null geodesics:

```
d²x^μ/dλ² + Γ^μ_αβ (dx^α/dλ)(dx^β/dλ) = 0
```

Integrated numerically using **RK4** (Runge-Kutta 4th order) for accuracy.

### Accretion Disk Physics

**Temperature Profile** (Shakura-Sunyaev):
```
T(r) = T_max · (r_ISCO/r)^(3/4) · [1 - √(r_ISCO/r)]^(1/4)
```

**Doppler Shift:**
```
f_obs = f_emit · √((1-β)/(1+β)) · γ
```

**Relativistic Beaming:**
```
I_obs = I_emit · D^(3+α)
where D = 1/(γ(1 - β·cosθ))
```

## Project Structure

```
cosmic-simulation/
├── CMakeLists.txt          # Build configuration
├── README.md               # This file
├── include/                # Header files
│   ├── core/               # Application, Window, UI
│   ├── physics/            # Schwarzschild, Geodesic, Metric
│   ├── rendering/          # Camera, Shader, Renderer
│   └── utils/              # Constants
├── src/                    # Implementation files
│   ├── core/
│   ├── physics/
│   └── rendering/
├── shaders/                # GLSL shaders
│   ├── raytracer.vert      # Vertex shader
│   ├── raytracer.frag      # Fragment shader (ray tracing)
│   └── raytracer_v1.frag   # Backup of original shader
├── external/               # Third-party libraries
│   ├── glad/               # OpenGL loader
│   ├── imgui/              # UI library
│   └── stb/                # Image loading
└── assets/                 # Textures, models
    └── skybox/             # Background images
```

## Performance

### Typical Performance (M1 MacBook Pro)

- **1280x720**: ~60 FPS (vsync)
- **1920x1080**: ~45-60 FPS
- **Max Steps: 1000**: Balanced quality/performance
- **Step Size: 0.1**: Good accuracy

### Optimization Tips

1. **Reduce Max Steps** - Lower for distant views, higher near event horizon
2. **Increase Step Size** - Faster but less accurate
3. **Disable Accretion Disk** - Significant performance boost
4. **Lower Resolution** - If GPU-bound

## Technical Details

### Architecture

- **Separation of Concerns** - Physics, rendering, and UI are modular
- **Extensible Design** - Easy to add new metrics (Kerr, Reissner-Nordström)
- **Modern C++17** - Smart pointers, RAII, move semantics
- **Shader-Based Physics** - Geodesic integration in fragment shader

### Ray Tracing Pipeline

1. **Camera** generates ray directions (NDC → world space)
2. **Transform** to spherical coordinates (r, θ, φ)
3. **Convert Velocity** using proper Jacobian matrices
4. **Integrate** geodesic equations (RK4, adaptive step size)
5. **Check Termination** - Event horizon, escape, disk intersection
6. **Sample Color** - Starfield, disk emission, blackbody radiation
7. **Tone Map** - Reinhard + gamma correction

### Numerical Methods

- **RK4 Integration** - 4th order Runge-Kutta for geodesics
- **Adaptive Stepping** - Smaller steps near event horizon
- **Coordinate Normalization** - Prevent θ, φ overflow
- **Singularity Avoidance** - Special handling at r = 2M

## References

### Papers
1. **"Gravitational Lensing by Spinning Black Holes"** - James, von Tunzelmann, Franklin, Thorne (2015)
   - Used for Interstellar's black hole visualization
   - Comprehensive geodesic integration methods

2. **"Exploring Black Holes: Introduction to General Relativity"** - Taylor & Wheeler
   - Educational approach to GR and black hole physics

3. **"Black Hole Visualization"** - Alain Riazuelo
   - Ray tracing techniques in curved spacetime

### Resources
- [LearnOpenGL.com](https://learnopengl.com/) - OpenGL tutorials
- [Shadertoy](https://www.shadertoy.com/) - Shader examples
- [ImGui Documentation](https://github.com/ocornut/imgui) - UI framework

## Development Timeline

**Week 1-2:** ✅ Foundation (CMake, OpenGL, GLFW, Camera)
**Week 3-4:** ✅ Schwarzschild Metric, Geodesic Integration
**Week 5-6:** ✅ Gravitational Lensing, Improved Ray Tracing
**Week 7-8:** ✅ Accretion Disk, Doppler Shifts, Relativistic Beaming
**Week 9-10:** ✅ ImGui UI, Parameter Controls, Polish
**Future:** Kerr Metric, Neutron Stars, Pulsars, Performance Optimization

## License

MIT License - Educational/Portfolio Project

Feel free to use this code for learning, research, or portfolio purposes!

## Contributing

This is a personal learning project, but suggestions and improvements are welcome!

## Author

Created as a physics-accurate hobby project exploring general relativity and computer graphics.

---

**⚫ "The black hole is the ultimate gravitational trap. It is a region of spacetime from which nothing, not even light, can escape."**
- Kip Thorne
