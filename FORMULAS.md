# Physics Formulas - Cosmic Simulator

This document explains ALL the physics and math used in the simulator. Every formula is listed with clear explanations.

## Table of Contents
1. [Units and Constants](#units-and-constants)
2. [Schwarzschild Black Holes](#schwarzschild-black-holes)
3. [Kerr (Spinning) Black Holes](#kerr-spinning-black-holes)
4. [Light Path Calculations](#light-path-calculations)
5. [Accretion Disk Physics](#accretion-disk-physics)
6. [Visual Effects](#visual-effects)

---

## Units and Constants

### Geometric Units (G = c = 1)
We use **geometric units** where:
- Gravitational constant G = 1
- Speed of light c = 1
- Distances measured in units of black hole mass M

**Conversion to real units:**
- 1 M☉ (solar mass) = 1.477 km = 4.926 μs (in geometric units)

### Physical Constants (CGS units in shader)
```
Stefan-Boltzmann constant: σ = 5.67×10⁻⁸ W/(m²K⁴)
Planck constant: h = 6.626×10⁻³⁴ J·s
Boltzmann constant: k = 1.381×10⁻²³ J/K
Speed of light: c = 2.998×10⁸ m/s
Proton mass: m_p = 1.673×10⁻²⁷ kg
```

---

## Schwarzschild Black Holes

### Metric (Non-Rotating)
The spacetime geometry around a non-rotating black hole:

```
ds² = -(1 - 2M/r)dt² + (1 - 2M/r)⁻¹dr² + r²(dθ² + sin²θ dφ²)
```

**Components:**
- `g_tt = -(1 - 2M/r)` - Time component
- `g_rr = 1/(1 - 2M/r)` - Radial component
- `g_θθ = r²` - Polar component
- `g_φφ = r² sin²θ` - Azimuthal component

**Code location:** `shaders/raytracer.frag:273-318`

### Critical Radii

1. **Event Horizon** (Point of no return):
   ```
   r_s = 2M
   ```

2. **Photon Sphere** (Unstable light orbit):
   ```
   r_ph = 3M
   ```

3. **ISCO** (Innermost Stable Circular Orbit):
   ```
   r_ISCO = 6M
   ```

**Code:** `src/physics/Schwarzschild.cpp:39-47`

### Gravitational Redshift
Light loses energy climbing out of gravity well:

```
1 + z = 1/√(1 - 2M/r)
```

**At different radii:**
- r = 2M (horizon): z → ∞ (infinite redshift)
- r = 3M: z = 0.732 (73% redshift)
- r = 6M (ISCO): z = 0.414 (41% redshift)

**Code:** `shaders/raytracer.frag:231-238`

---

## Kerr (Spinning) Black Holes

### Metric (Boyer-Lindquist Coordinates)
For a black hole with spin parameter a (where |a| ≤ M):

```
ds² = -(1 - 2Mr/Σ)dt² - (4Mra sin²θ/Σ)dtdφ + (Σ/Δ)dr² + Σdθ² + A sin²θ/Σ dφ²
```

**Where:**
```
Σ = r² + a² cos²θ
Δ = r² - 2Mr + a²
A = (r² + a²)² - a²Δ sin²θ
a = J/M (dimensionless spin, 0 ≤ |a| ≤ 0.998)
```

**Key difference from Schwarzschild:** The `g_tφ` term (frame dragging)

**Code:** `shaders/raytracer.frag:320-411`, `src/physics/Kerr.cpp`

### Critical Radii (Spin-Dependent)

1. **Event Horizon:**
   ```
   r_+ = M + √(M² - a²)
   ```
   - a = 0: r_+ = 2M (Schwarzschild)
   - a = 0.998M: r_+ ≈ 1.06M (much smaller!)

2. **Ergosphere** (Region of spacetime dragging):
   ```
   r_ergo = M + √(M² - a² cos²θ)
   ```
   Energy extraction possible in r_+ < r < r_ergo

3. **ISCO** (Prograde orbit):
   ```
   z₁ = 1 + ∛(1 - a²/M²)[∛(1 + a/M) + ∛(1 - a/M)]
   z₂ = √(3a²/M² + z₁²)
   r_ISCO = M[3 + z₂ - √((3 - z₁)(3 + z₁ + 2z₂))]
   ```

   **Values:**
   - a = 0: r_ISCO = 6M
   - a = 0.5M: r_ISCO ≈ 4.23M
   - a = 0.998M: r_ISCO ≈ 1.24M (5x closer!)

**Code:** `src/physics/Kerr.cpp:211-233`

### Frame Dragging
Spacetime itself rotates near a spinning black hole. Zero Angular Momentum Observers (ZAMOs) rotate at:

```
ω = dφ/dt = 2Mar / (r³ + a²r + 2Ma²)
```

**At the horizon:**
```
ω_horizon = a / (2Mr_+)
```

**Code:** `src/physics/Kerr.cpp:289-300`

---

## Light Path Calculations

### Geodesic Equation
Light follows "straight lines" in curved spacetime:

```
d²x^μ/dλ² + Γ^μ_αβ (dx^α/dλ)(dx^β/dλ) = 0
```

**Where:**
- x^μ = (t, r, θ, φ) - Spacetime coordinates
- λ - Affine parameter (measures distance along path)
- Γ^μ_αβ - Christoffel symbols (how space curves)

### Christoffel Symbols
Describe how basis vectors change in curved space:

```
Γ^λ_μν = (1/2) g^λσ (∂_μ g_νσ + ∂_ν g_μσ - ∂_σ g_μν)
```

**For Schwarzschild, key components:**
```
Γ^t_tr = M/(r(r-2M))
Γ^r_tt = M(r-2M)/r³
Γ^r_rr = -M/(r(r-2M))
Γ^r_θθ = -(r-2M)
Γ^r_φφ = -(r-2M)sin²θ
```

**Code:** `src/physics/Schwarzschild.cpp:113-176`

For Kerr metric, computed from metric derivatives:
**Code:** `shaders/raytracer.frag:420-480`

### Null Geodesic Constraint
For light (massless):

```
g_μν (dx^μ/dλ)(dx^ν/dλ) = 0
```

This ensures light travels at speed c.

**Code:** `src/physics/Geodesic.cpp:195-201`

---

## Accretion Disk Physics

### Shakura-Sunyaev Model
Standard thin disk model with viscous heating:

#### Temperature Profile
```
T(r) = T_norm × (r_ISCO/r)^(3/4) × f(r)^(1/4)
```

Where boundary condition:
```
f(r) = 1 - √(r_ISCO/r)
```

This zero-torque inner boundary ensures realistic temperatures.

**Typical values:**
- At ISCO: T ≈ 10,000 K (visible/UV)
- At 10M: T ≈ 3,000 K (infrared)
- At 100M: T ≈ 500 K (far-infrared)

**Code:** `shaders/raytracer.frag:656-668`

#### Keplerian Velocity
Matter orbits at:

```
v_K = √(M/r)
ω_K = √(M/r³)
```

**In Kerr metric:** Add frame dragging correction

#### Scale Height
Vertical thickness of disk:

```
H/r = c_s/(rΩ_K)
```

Where sound speed:
```
c_s = √(kT/(μm_p))
```

Typical: H/r ≈ 0.01-0.1 (thin disk)

**Code:** `shaders/raytracer.frag:166-185`

#### Vertical Density Profile
Hydrostatic equilibrium gives Gaussian:

```
ρ(z) = ρ₀ exp(-z²/2H²)
```

**Code:** `shaders/raytracer.frag:673-685`

#### Optical Depth
How opaque the disk is:

```
τ = κΣ/2
```

Where:
- κ ≈ 0.4 cm²/g (Thomson scattering opacity)
- Σ = surface density

**For optically thick (τ >> 1):** Emits as blackbody
**For optically thin (τ << 1):** Transparent

**Code:** `shaders/raytracer.frag:197-229`, `shaders/raytracer.frag:688`

### Radiative Transfer
Emission from disk surface:

```
I = B(T)[1 - exp(-τ_eff)]
```

Where B(T) is Planck function (blackbody).

**Code:** `shaders/raytracer.frag:728-752`

---

## Visual Effects

### Relativistic Doppler Shift
Light from moving matter is shifted:

**Doppler factor:**
```
D = 1 / [γ(1 - β·cosθ)]
```

Where:
- γ = 1/√(1 - v²/c²) - Lorentz factor
- β = v/c - Velocity in units of c
- θ - Angle between velocity and line of sight

**Effects:**
- Approaching side: Blueshifted (D > 1)
- Receding side: Redshifted (D < 1)
- Transverse: Time dilation redshift (γ factor)

**Code:** `shaders/raytracer.frag:705-719`

### Relativistic Beaming
Intensity enhanced in direction of motion:

```
I_obs = I_emit × D^(3+α)
```

For continuum emission (α ≈ 0):
```
I_obs = I_emit × D³
```

**Result:** Approaching side appears much brighter!

**Code:** `shaders/raytracer.frag:726`

### Blackbody Radiation (Planck's Law)
Temperature determines color:

**Wien's displacement law:**
```
λ_max = 2.898×10⁻³ / T [meters]
```

**Color mapping:**
- T < 2000 K: Red
- T = 3000 K: Orange
- T = 5000 K: Yellow-white
- T = 10000 K: White
- T > 20000 K: Blue-white

**Code:** `shaders/raytracer.frag:108-163`

### Tone Mapping
Map physically correct intensities to displayable range:

**Reinhard operator:**
```
L_out = L_in / (1 + L_in)
```

Plus gamma correction:
```
C_final = C^(1/2.2)
```

**Code:** `shaders/raytracer.frag:886-888`

---

## References

### Foundational Papers
1. **Schwarzschild, K. (1916)** - Original black hole solution
2. **Kerr, R. P. (1963)** - Spinning black hole solution
3. **Shakura & Sunyaev (1973)** - Accretion disk model
4. **Bardeen et al. (1972)** - Kerr black hole orbits and ISCO

### Textbooks
- **Misner, Thorne & Wheeler** - "Gravitation" (1973)
- **Chandrasekhar** - "The Mathematical Theory of Black Holes" (1983)
- **Frank, King & Raine** - "Accretion Power in Astrophysics" (2002)

### Modern Observations
- **Event Horizon Telescope (2019)** - First black hole image (M87)
- **GRAVITY Collaboration (2020)** - Observed Schwarzschild precession

---

## Quick Reference Table

| Property | Schwarzschild | Kerr (a=0.5M) | Kerr (a=0.998M) |
|----------|---------------|---------------|-----------------|
| Event Horizon | 2M | 1.73M | 1.06M |
| Photon Sphere | 3M | 2.4M | 1.8M |
| ISCO | 6M | 4.23M | 1.24M |
| Efficiency η | 5.7% | 12% | 42% |
| Frame Drag | None | Moderate | Extreme |

---

**All formulas in this document are implemented in the simulator and can be found in the code locations listed.**

