# Black Hole Simulator

A real-time 3D black hole visualization with accurate physics. Watch light bend around massive objects and see how spinning black holes distort spacetime differently than stationary ones.

![Black Hole Visualization](https://img.shields.io/badge/Physics-Accurate-blue) ![C++17](https://img.shields.io/badge/C++-17-green) ![OpenGL](https://img.shields.io/badge/OpenGL-4.1-red)

## What Does This Do?

This simulator shows you **what it would look like** to fly around a black hole:
- Light bends in crazy ways (gravitational lensing)
- Stars behind the black hole appear distorted
- An accretion disk (hot gas) orbits the black hole, glowing bright
- You can fly around in 3D and explore from any angle

**New:** Now supports **spinning black holes** (Kerr metric), which look different from non-rotating ones!

## Quick Start

```bash
# Clone and build
git clone <your-repo-url>
cd cosmic-simulation
mkdir build && cd build
cmake ..
make

# Run the simulator
./CosmicSimulator

# Or try the simple example
./kerr_example
```

## Controls

**Mouse & Keyboard:**
- **WASD** - Fly forward/left/back/right
- **Q/E** - Fly up/down
- **Mouse** - Look around (press TAB to capture/release mouse)
- **SHIFT** - Fly faster
- **1-9** - Jump to preset camera positions
- **H** - Hide/show UI
- **ESC** - Quit

**Try This:** Press `2` to fly close to the black hole and see extreme light bending!

## What's Happening Behind the Scenes?

### Non-Rotating Black Holes (Schwarzschild)
- Event Horizon at 2M: Nothing escapes past this point
- Photon Sphere at 3M: Light can orbit here (unstable)
- ISCO at 6M: Closest stable orbit for matter

### Spinning Black Holes (Kerr) - NEW!
- Event horizon is smaller (~1.15M for maximum spin)
- ISCO can be as close as 1.24M (5x closer than non-rotating!)
- Spacetime itself rotates (frame dragging)
- Ergosphere: Region where energy extraction is possible

**What does this mean visually?**
- Spinning black holes let matter orbit much closer
- The accretion disk is brighter (more energy released)
- Light bends differently depending on rotation direction
- The "shadow" of the black hole looks different

## Project Structure

```
cosmic-simulation/
├── src/
│   ├── main.cpp                    # Program entry point
│   ├── core/                       # Application, window, UI
│   ├── physics/                    # Black hole physics
│   │   ├── Schwarzschild.cpp       # Non-rotating black holes
│   │   ├── Kerr.cpp               # Spinning black holes (NEW!)
│   │   └── Geodesic.cpp           # Light path calculations
│   └── rendering/                  # Graphics, camera, shaders
├── shaders/
│   └── raytracer.frag             # GPU ray tracing shader
├── examples/
│   └── kerr_example.cpp           # Simple example program
└── include/                        # Header files
```

## Example Program

Want to understand the physics without the visualization? Run the example:

```bash
./kerr_example
```

This simple program shows you:
- How spinning changes a black hole's properties
- Frame dragging (spacetime rotation)
- The ergosphere and energy extraction
- No graphics - just numbers and explanations!

## Requirements

**macOS:**
```bash
brew install cmake glfw glm eigen
```

**Ubuntu/Debian:**
```bash
sudo apt install cmake libglfw3-dev libglm-dev libeigen3-dev libgl-dev
```

**What you need:**
- C++17 compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.20+
- OpenGL 4.1+

## Understanding the Physics (Simple Version)

### What is a Black Hole?
A region where gravity is so strong that nothing can escape - not even light. They form when massive stars collapse.

### Why Does Light Bend?
Gravity bends spacetime itself. Light travels in "straight lines" through curved spacetime, which looks bent to us.

### What's an Accretion Disk?
Hot gas spiraling into the black hole. Friction makes it glow - this is what we actually see when we "photograph" a black hole (like M87).

### Rotating vs Non-Rotating
Real black holes spin (formed from spinning stars). Spinning black holes:
- Have a smaller event horizon
- Drag spacetime around with them
- Let matter orbit closer
- Are more efficient at powering jets and radiation

### Geometric Units
Physics equations are simpler when we set G=c=1 (gravity constant and light speed equal 1). Distances are measured in multiples of the black hole mass M.

## Technical Details (For the Curious)

### Ray Tracing
The shader traces light rays backward from your camera through curved spacetime:
1. Start at camera position
2. Take small steps, bending the ray based on gravity
3. Check if ray hits black hole (absorbed) or disk (colored)
4. If ray escapes, show the background stars

### Metrics
- **Schwarzschild**: ds² = -(1 - 2M/r)dt² + (1 - 2M/r)⁻¹dr² + r²dΩ²
- **Kerr**: More complex, includes off-diagonal terms from rotation

### Geodesic Equation
Describes how light moves: d²x^μ/dλ² + Γ^μ_αβ (dx^α/dλ)(dx^β/dλ) = 0

Don't worry if you don't understand these - the simulator works even if you just enjoy the visuals!

## Comparison with Real Observations

The Event Horizon Telescope (EHT) took the first black hole photo in 2019 (M87). This simulator can produce similar-looking images because it uses the same physics!

**M87 Black Hole:**
- Mass: ~6.5 billion solar masses
- Spin: Unknown (probably high)
- Shadow size: ~5.2M (matches theoretical predictions)

## Performance

**Typical (M1 MacBook Pro):**
- 1920x1080: 45-60 FPS
- 1280x720: 60 FPS (vsync)

**Optimization:**
- Reduce "Max Steps" in UI for better FPS
- Increase "Step Size" for faster (but less accurate) rendering

## Future Ideas

Want to contribute? These would be cool:
- [ ] Add UI slider for black hole spin
- [ ] Neutron stars (with visible surface)
- [ ] Binary black hole systems
- [ ] Gravitational wave visualization
- [ ] Load real star maps as background
- [ ] Record videos of camera paths

## Learning Resources

Want to learn more?
- [Black Holes - Crash Course Astronomy](https://www.youtube.com/watch?v=qZWPBKULEEg)
- [Interstellar's Black Hole - Wired Article](https://www.wired.com/2014/10/astrophysics-interstellar-black-hole/)
- [EHT Black Hole Image Explained](https://eventhorizontelescope.org/)
- [PBS Spacetime - Black Holes](https://www.youtube.com/playlist?list=PLsPUh22kYmNBl4h0i4mI5zDflExXJMo_x)

## License

MIT License - Free to use for learning and education!

## Credits

Physics based on:
- Schwarzschild (1916) - First black hole solution
- Kerr (1963) - Spinning black hole solution
- Bardeen et al. (1972) - Accretion disk theory
- James et al. (2015) - Interstellar's Gargantua visualization

---

**"The black hole teaches us that space can be crumpled like a piece of paper into an infinitesimal dot, that time can be extinguished like a blown-out flame."** - Carl Sagan
