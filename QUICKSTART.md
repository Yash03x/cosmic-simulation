# Cosmic Simulator - Quick Start Guide

## 🚀 Running the Simulator

```bash
cd /Users/mailyas/repos/cosmic-simulation/build
./CosmicSimulator
```

## ⌨️ Essential Controls

| Key | Action |
|-----|--------|
| **WASD** | Move camera |
| **Q/E** | Down/Up |
| **Mouse** | Look around |
| **TAB** | Capture/release mouse |
| **H** | Toggle UI |
| **SHIFT** | Speed boost |
| **ESC** | Exit |

## 🎮 First 5 Minutes

1. **Launch** the app
2. **Press TAB** to capture mouse
3. **Use W** to fly forward toward the black hole
4. **Use mouse** to look around and see gravitational lensing
5. **Press H** to open UI panel
6. **Enable accretion disk** checkbox
7. **Adjust black hole mass** slider
8. **Enjoy the show!** ⚫✨

## 🎛️ UI Panels

### Control Panel (Left Side)
- **Black Hole Mass**: 0.1 - 10 solar masses
- **Ray Tracing Quality**: Max steps & step size
- **Camera Speed**: 1 - 50 units/sec
- **Accretion Disk**: Enable/disable toggle
- **Presets**: Quick mass settings

### Statistics Panel (Bottom Left)
- **FPS**: Frames per second
- **Frame Time**: Milliseconds per frame
- **Distance**: From black hole center
- **Warnings**: Event horizon/photon sphere alerts

## 🎯 Cool Things to Try

### 1. Gravitational Lensing
- Fly around the black hole
- Watch background stars bend
- See Einstein rings form

### 2. Accretion Disk
- Enable in UI
- Fly edge-on to see thin disk
- Fly face-on to see spiral
- Notice blue vs red sides (Doppler shift!)

### 3. Event Horizon
- Set mass to 1.0 M☉
- Event horizon is at r = 2.0
- Fly to distance 2.5-3.0
- See the perfectly black sphere

### 4. Photon Sphere
- At r = 3.0 M (for 1 M☉ black hole)
- Light orbits here!
- Extreme lensing effects
- Multiple images of stars

### 5. Mass Changes
- Start at 1.0 M☉
- Increase to 10.0 M☉
- Watch event horizon grow!
- Lensing effects change

## 🎨 Visual Quality Settings

### Performance Mode (30-60 FPS)
- Max Steps: 500
- Step Size: 0.15
- Disable Accretion Disk

### Balanced Mode (45-60 FPS) - **Default**
- Max Steps: 1000
- Step Size: 0.1
- Enable Accretion Disk

### Quality Mode (30-45 FPS)
- Max Steps: 2000
- Step Size: 0.05
- Enable Accretion Disk

### Ultra Mode (15-30 FPS)
- Max Steps: 5000
- Step Size: 0.01
- Enable Accretion Disk
- For screenshots only!

## 📊 Understanding the Physics

### Key Distances (for 1 M☉ black hole)

| Distance | Region | What You'll See |
|----------|--------|-----------------|
| r < 2.0 | Inside event horizon | **Don't go here!** Can't escape |
| r = 2.0 | Event horizon | Perfectly black boundary |
| r ≈ 3.0 | Photon sphere | Extreme light bending, rings |
| r = 6.0 | ISCO | Inner edge of accretion disk |
| r = 6-60 | Accretion disk | Glowing orange/red/blue disk |
| r > 60 | Safe zone | Normal space, mild lensing |

### Position Reference
- Start position: (0, 0, 20) - Safe distance
- Camera shows (x, y, z) in statistics panel
- Distance = √(x² + y² + z²)

## 🐛 Troubleshooting

### "Application won't start"
```bash
# Check dependencies
brew list glfw glm eigen

# Rebuild
cd build
cmake --build . --clean-first
```

### "Low FPS / Choppy"
- Reduce Max Steps to 500
- Increase Step Size to 0.15
- Disable Accretion Disk
- Lower resolution

### "UI not showing"
- Press **H** key to toggle
- Check terminal for errors

### "Mouse not captured"
- Press **TAB** to toggle
- Make sure UI doesn't have focus

### "Black screen"
- You might be inside event horizon!
- Press H, click "Reset Position"
- Or restart application

## 🎓 Physics Reference

### Geometric Units (G = c = 1)
- 1 M☉ ≈ 1.477 km
- Distances in multiples of black hole mass
- Time in geometric units

### Schwarzschild Radii
- **r<sub>s</sub> = 2M**: Event horizon
- **r<sub>ph</sub> = 3M**: Photon sphere
- **r<sub>ISCO</sub> = 6M**: Innermost stable orbit

## 📸 Taking Screenshots

### macOS
1. Press **Cmd + Shift + 4**
2. Press **Space**
3. Click window

### Best Screenshot Settings
- Max Steps: 5000
- Step Size: 0.01
- Press **H** to hide UI (if desired)
- Position camera for best view
- Enable Accretion Disk

## 🎬 Performance Tips

### For Smooth Experience
1. Start with default settings
2. Check FPS in statistics panel
3. If < 30 FPS, reduce quality
4. If > 60 FPS (vsync), can increase quality

### For Best Visuals
1. Maximize quality settings
2. Accept lower FPS (~20-30)
3. Perfect for screenshots
4. Not for real-time exploration

## 🔄 Rebuilding After Changes

```bash
cd /Users/mailyas/repos/cosmic-simulation

# Modify code...

cd build
cmake --build .
./CosmicSimulator
```

## 📦 Clean Rebuild

```bash
cd /Users/mailyas/repos/cosmic-simulation
rm -rf build
mkdir build && cd build
cmake ..
cmake --build .
./CosmicSimulator
```

## 🎯 Exploration Checklist

- [ ] Fly around black hole
- [ ] Enable accretion disk
- [ ] See gravitational lensing
- [ ] Change black hole mass
- [ ] Fly to photon sphere
- [ ] Approach event horizon (carefully!)
- [ ] View disk edge-on
- [ ] View disk face-on
- [ ] Try different quality settings
- [ ] Take screenshots
- [ ] Adjust camera speed
- [ ] Use presets (3 M☉, 10 M☉)
- [ ] Read statistics panel
- [ ] Explore all UI options

## 💡 Pro Tips

1. **Start Distant**: Begin far away to get oriented
2. **Slow Approach**: SHIFT doubles speed, use carefully near BH
3. **UI is Your Friend**: Press H frequently to check stats
4. **Save Good Views**: Note position (x,y,z) for later
5. **Quality vs FPS**: Balance based on your goal (explore vs screenshot)
6. **Disk is Expensive**: Disable for performance testing
7. **Reset Often**: UI has reset button if you get lost

## 🎨 Visual Phenomena to Observe

### Gravitational Lensing
- Stars appear to bend around black hole
- Multiple images of same star
- Einstein rings
- Background distortion

### Accretion Disk
- Blue side (approaching, blue-shifted)
- Red side (receding, red-shifted)
- Brighter on blue side (relativistic beaming)
- Temperature gradient (hot inner, cool outer)

### Event Horizon
- Perfectly black
- No light escapes
- Sharp boundary
- Size proportional to mass

### Photon Sphere
- At r = 3M
- Orbiting photons
- Extreme lensing
- Unstable (photons eventually fall or escape)

## 📚 Next Steps

1. **Experiment**: Try all settings and positions
2. **Read Code**: Check out src/ and shaders/
3. **Modify**: Change constants, colors, physics
4. **Extend**: Add new features (Kerr metric?)
5. **Share**: Show friends, use in presentations

## 🌟 Have Fun!

This is a **sandbox for exploring general relativity**.

There's no "right" way to use it - just explore, experiment, and enjoy the beauty of curved spacetime!

---

**Questions or Issues?**
Check IMPLEMENTATION_SUMMARY.md for detailed technical information.
Check README.md for comprehensive project documentation.

**Happy exploring!** ⚫✨
