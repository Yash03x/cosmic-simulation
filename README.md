# Cosmic Simulator - Real-Time Space Visualization

**A complete, production-ready simulator** for black holes and other extreme cosmic objects. Everything is fully integrated and working - not just code sitting unused!

![Physics Accurate](https://img.shields.io/badge/Physics-Accurate-blue) ![C++17](https://img.shields.io/badge/C++-17-green) ![OpenGL](https://img.shields.io/badge/OpenGL-4.1-red) ![Animated](https://img.shields.io/badge/Animation-RealTime-orange)

## ✅ What's Working NOW

### Fully Integrated Features
- ✨ **Real-time Animation** - Not static! Everything moves and evolves with time
- 🌀 **Spinning Black Holes (Kerr)** - Full implementation with UI controls
- ⚫ **Non-rotating Black Holes (Schwarzschild)** - Complete with exact physics
- 💿 **Animated Accretion Disk** - Orbiting material with proper rotation
- ⭐ **Time-varying Starfield** - Twinkling stars that rotate
- 🎮 **Interactive UI** - Change black hole spin in real-time
- 📊 **Live Statistics** - FPS, camera position, physics info
- 🎥 **First-person Flight** - Fly around and explore from any angle

**Everything you see is animated - this is a video simulation, not static images!**

## Quick Start

\`\`\`bash
# Clone and build
git clone <repo-url>
cd cosmic-simulation

# Install dependencies (macOS)
brew install cmake glfw glm eigen

# Or Ubuntu/Debian
sudo apt install cmake libglfw3-dev libglm-dev libeigen3-dev libgl-dev

# Build
mkdir build && cd build
cmake ..
make

# Run!
./CosmicSimulator

# Or try the example
./kerr_example
\`\`\`

## Controls - Fly Around in Real-Time!

**Movement:**
- \`WASD\` - Fly forward/left/back/right
- \`Q/E\` - Up/down
- \`Mouse\` - Look around (TAB to capture/release)
- \`SHIFT\` - Fly faster
- \`1-9\` - Jump to preset views

**Interface:**
- \`H\` - Hide/show UI
- \`ESC\` - Quit

**Try this:** Press \`2\` to fly close, then watch the disk rotate and stars twinkle!

## What Makes This Special

### 1. Everything is INTEGRATED
No unused code. Every physics module is connected to the visual renderer.

### 2. Real Animation, Not Static
- Stars twinkle and rotate
- Disk material orbits continuously
- Everything updates with time

### 3. Physically Accurate
All physics matches published research. See \`FORMULAS.md\` for every equation!

### 4. Ready for Expansion
Framework ready for neutron stars, pulsars, and more.

## Features

### Black Holes
- **Schwarzschild:** Event horizon 2M, ISCO 6M
- **Kerr:** Spin up to 0.998, ISCO as close as 1.24M
- **Frame Dragging:** Spacetime rotation visualization

### Accretion Disk
- Orbital motion at Keplerian velocity
- Temperature gradient (hot inner, cool outer)
- Doppler shifts (red/blue)
- Relativistic beaming

## Documentation

- **FORMULAS.md** - Every physics equation explained
- **examples/kerr_example.cpp** - Simple demo program
- **include/cosmic/** - Framework for future objects

## Coming Soon
- Neutron stars with visible surface
- Pulsars with rotating beams
- Binary black hole systems

## License
MIT - Free for education and research!

---

**Status: Production Ready ✅**

Clone, build, and explore black holes TODAY!
