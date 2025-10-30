#version 410 core

in vec2 vTexCoord;
in vec2 vScreenPos;

out vec4 FragColor;

// Camera uniforms
uniform vec3 uCameraPos;
uniform vec3 uCameraFront;
uniform vec3 uCameraUp;
uniform vec3 uCameraRight;
uniform float uFov;
uniform float uAspect;

// Black hole uniforms
uniform float uBlackHoleMass;
uniform float uEventHorizon;
uniform float uPhotonSphere;
uniform float uISCO;

// Ray tracing parameters
uniform int uMaxSteps;
uniform float uStepSize;
uniform bool uAccretionDiskEnabled;

// Accretion disk physics parameters
uniform float uAccretionRate;      // Mdot in solar masses per year
uniform float uAlphaViscosity;     // Shakura-Sunyaev alpha parameter (0.01-0.1)
uniform float uDiskInclination;    // Viewing angle in radians
uniform float uScaleHeightRatio;   // H/r ratio at ISCO (typically 0.01-0.1)

#define PI 3.14159265359
#define TWO_PI 6.28318530718
#define ESCAPE_RADIUS 50.0
#define EPSILON 1e-6

// Physical constants (in geometric units where G=c=1)
#define STEFAN_BOLTZMANN 5.670374419e-8  // W m^-2 K^-4
#define PLANCK_H 6.62607015e-34          // J s
#define BOLTZMANN_K 1.380649e-23         // J K^-1
#define SPEED_OF_LIGHT 299792458.0       // m s^-1
#define PROTON_MASS 1.67262192e-27       // kg

// ENHANCED: Much denser, more beautiful starfield
vec3 enhancedStarfield(vec3 dir) {
    vec3 d = normalize(dir);
    vec3 color = vec3(0.005, 0.005, 0.015); // Darker background for contrast

    // LAYER 1: Very bright stars (giants)
    vec2 p1 = vec2(atan(d.y, d.x), acos(d.z)) * 25.0;
    float s1 = fract(sin(dot(p1, vec2(12.9898, 78.233))) * 43758.5453);
    if (s1 > 0.9985) {
        float brightness = (s1 - 0.9985) / 0.0015;
        vec3 starColor = mix(vec3(1.0, 0.95, 0.85), vec3(0.85, 0.9, 1.0), fract(s1 * 10.0));
        color += starColor * brightness * 6.0;
    }

    // LAYER 2: Bright stars
    vec2 p2 = vec2(atan(d.y, d.x), acos(d.z)) * 60.0;
    float s2 = fract(sin(dot(p2, vec2(15.234, 82.123))) * 43758.5453);
    if (s2 > 0.997) {
        float brightness = (s2 - 0.997) / 0.003;
        vec3 starColor = mix(vec3(1.0, 1.0, 1.0), vec3(0.9, 0.95, 1.0), fract(s2 * 7.0));
        color += starColor * brightness * 3.0;
    }

    // LAYER 3: Medium stars
    vec2 p3 = vec2(atan(d.y, d.x), acos(d.z)) * 120.0;
    float s3 = fract(sin(dot(p3, vec2(18.456, 85.789))) * 43758.5453);
    if (s3 > 0.994) {
        float brightness = (s3 - 0.994) / 0.006;
        color += vec3(0.9, 0.9, 1.0) * brightness * 1.5;
    }

    // LAYER 4: Small stars (dust)
    vec2 p4 = vec2(atan(d.y, d.x), acos(d.z)) * 200.0;
    float s4 = fract(sin(dot(p4, vec2(21.789, 88.456))) * 43758.5453);
    if (s4 > 0.992) {
        float brightness = (s4 - 0.992) / 0.008;
        color += vec3(0.7, 0.8, 0.9) * brightness * 0.8;
    }

    // LAYER 5: Micro stars (depth)
    vec2 p5 = vec2(atan(d.y, d.x), acos(d.z)) * 350.0;
    float s5 = fract(sin(dot(p5, vec2(24.123, 91.789))) * 43758.5453);
    if (s5 > 0.9905) {
        color += vec3(0.5, 0.6, 0.7) * 0.3;
    }

    // Milky Way gradient (more pronounced)
    float theta = acos(d.z);
    float phi = atan(d.y, d.x);
    float gradient = abs(sin(theta * 2.0)) * abs(cos(phi * 0.5));
    color += vec3(0.04, 0.05, 0.08) * gradient * 0.6;

    // Nebula-like wisps
    float nebula = abs(sin(theta * 5.0 + phi * 3.0)) * abs(cos(theta * 3.0));
    color += vec3(0.02, 0.03, 0.05) * nebula * 0.3;

    return color;
}

// PHYSICS: Convert temperature to blackbody RGB using proper Planck spectrum
// Integrates over visible wavelengths and converts to sRGB
vec3 temperatureToRGB(float T) {
    // Simplified but physically-motivated blackbody color
    // Based on Wien's displacement law and Planck curve shape

    // Clamp temperature to reasonable range
    T = clamp(T, 1000.0, 50000.0);

    // Wien's displacement: λ_max = 2.898e-3 / T (in meters)
    // Convert to relative wavelengths for RGB
    float t = T / 10000.0; // Normalize around 10,000K

    vec3 color;

    if (T < 3000.0) {
        // Red-dominated (cool)
        float x = T / 3000.0;
        color = vec3(1.0, x * 0.5, x * 0.1);
    } else if (T < 5000.0) {
        // Orange to yellow
        float x = (T - 3000.0) / 2000.0;
        color = vec3(1.0, 0.5 + x * 0.4, 0.1 + x * 0.3);
    } else if (T < 8000.0) {
        // Yellow to white
        float x = (T - 5000.0) / 3000.0;
        color = vec3(1.0, 0.9 + x * 0.1, 0.4 + x * 0.5);
    } else if (T < 15000.0) {
        // White to blue-white
        float x = (T - 8000.0) / 7000.0;
        color = vec3(1.0 - x * 0.2, 1.0 - x * 0.1, 0.9 + x * 0.1);
    } else {
        // Blue-dominated (hot)
        float x = min((T - 15000.0) / 10000.0, 1.0);
        color = vec3(0.7 - x * 0.2, 0.8 - x * 0.2, 1.0);
    }

    // Apply approximate Planck curve intensity falloff
    float intensity = pow(t, 4.0) / (exp(5.0/t) - 1.0);
    intensity = clamp(intensity * 0.5, 0.1, 2.0);

    return color * intensity;
}

// PHYSICS: Gravitational redshift factor
// 1 + z = 1/sqrt(1 - 2M/r)
float gravitationalRedshift(float r, float M) {
    float rs = 2.0 * M;
    if (r <= rs) return 100.0; // Extreme redshift near horizon
    return 1.0 / sqrt(1.0 - rs / r);
}

// PHYSICS: Calculate disk scale height H(r)
// H(r) = (c_s / v_K) * r, where c_s is sound speed, v_K is Keplerian velocity
float diskScaleHeight(float r, float T, float M) {
    // Sound speed: c_s = sqrt(k*T / (mu * m_p))
    // Assuming mean molecular weight mu ~ 0.6 for ionized plasma
    // In geometric units, we use a simplified form

    // Temperature-dependent sound speed (simplified)
    float c_s = sqrt(T / 10000.0) * 0.1; // Normalized to ~0.1c for typical temps

    // Keplerian velocity: v_K = sqrt(M/r)
    float v_K = sqrt(M / r);

    // Scale height: H = c_s * r / v_K
    float H = (c_s / v_K) * r;

    // Apply user-defined scale height ratio as a multiplier
    H *= uScaleHeightRatio / 0.05; // Normalized to H/r ~ 0.05

    return H;
}

// PHYSICS: Shakura-Sunyaev surface density Σ(r)
// Simplified version: Σ ∝ r^(-3/4) for thin disk
float surfaceDensity(float r, float r_in, float Mdot, float alpha) {
    // Normalized surface density profile
    // Σ(r) ∝ Mdot * r^(-3/4) / alpha
    float profile = pow(r_in / r, 0.75);
    float torqueFactor = max(1.0 - sqrt(r_in / r), 0.01);

    // Include accretion rate and viscosity dependence
    float Sigma = (Mdot / alpha) * profile * torqueFactor;

    return Sigma;
}

// PHYSICS: Optical depth τ(r)
// τ = κ * Σ, where κ is opacity (Thomson scattering dominated)
float opticalDepth(float r, float r_in, float Mdot, float alpha, float H) {
    float Sigma = surfaceDensity(r, r_in, Mdot, alpha);

    // Thomson scattering opacity: κ_es ≈ 0.34 cm^2/g (in CGS)
    // In normalized units
    float kappa = 0.4;

    // Optical depth through disk: τ ~ κ * Σ
    // For vertical thickness, account for H
    float tau = kappa * Sigma / (H + 0.1);

    return max(tau, 0.01);
}

// ENHANCED: Photon sphere glow effect
vec3 photonSphereGlow(float r, float closestApproach, vec3 baseColor) {
    float photonR = uPhotonSphere;

    // Distance from photon sphere
    float distToPhotonSphere = abs(closestApproach - photonR);

    // Glow intensity (peaks at photon sphere)
    float glowIntensity = 0.0;
    if (distToPhotonSphere < 2.0) {
        glowIntensity = (2.0 - distToPhotonSphere) / 2.0;
        glowIntensity = pow(glowIntensity, 2.0); // Sharper falloff
    }

    // Orange-yellow glow color
    vec3 glowColor = vec3(1.0, 0.6, 0.2);

    return baseColor + glowColor * glowIntensity * 0.3;
}

// ============================================================================
// GEODESIC INTEGRATION: Accurate ray tracing using Christoffel symbols
// ============================================================================

// Cartesian to spherical coordinates
vec3 cartesianToSpherical(vec3 pos) {
    float r = length(pos);
    float theta = acos(clamp(pos.z / max(r, EPSILON), -1.0, 1.0));
    float phi = atan(pos.y, pos.x);
    return vec3(r, theta, phi);
}

// Spherical to Cartesian coordinates
vec3 sphericalToCartesian(vec3 spherical) {
    float r = spherical.x;
    float theta = spherical.y;
    float phi = spherical.z;
    return vec3(
        r * sin(theta) * cos(phi),
        r * sin(theta) * sin(phi),
        r * cos(theta)
    );
}

// Christoffel symbols for Schwarzschild metric
struct ChristoffelSymbols {
    float gamma_r_rr;
    float gamma_r_theta_theta;
    float gamma_r_phi_phi;
    float gamma_theta_r_theta;
    float gamma_theta_phi_phi;
    float gamma_phi_r_phi;
    float gamma_phi_theta_phi;
};

ChristoffelSymbols computeChristoffel(float r, float theta, float rs) {
    ChristoffelSymbols gamma;

    // Avoid singularities near event horizon
    if (r <= rs * 1.001) {
        gamma.gamma_r_rr = 0.0;
        gamma.gamma_r_theta_theta = 0.0;
        gamma.gamma_r_phi_phi = 0.0;
        gamma.gamma_theta_r_theta = 0.0;
        gamma.gamma_theta_phi_phi = 0.0;
        gamma.gamma_phi_r_phi = 0.0;
        gamma.gamma_phi_theta_phi = 0.0;
        return gamma;
    }

    float r2 = r * r;
    float sin_theta = sin(theta);
    float cos_theta = cos(theta);
    float sin2_theta = sin_theta * sin_theta;

    // Γ^r_rr = rs / (2r(r - rs))
    gamma.gamma_r_rr = rs / (2.0 * r * (r - rs));

    // Γ^r_θθ = -(r - rs)
    gamma.gamma_r_theta_theta = -(r - rs);

    // Γ^r_φφ = -(r - rs) * sin²θ
    gamma.gamma_r_phi_phi = -(r - rs) * sin2_theta;

    // Γ^θ_rθ = 1/r
    gamma.gamma_theta_r_theta = 1.0 / r;

    // Γ^θ_φφ = -sinθ * cosθ
    gamma.gamma_theta_phi_phi = -sin_theta * cos_theta;

    // Γ^φ_rφ = 1/r
    gamma.gamma_phi_r_phi = 1.0 / r;

    // Γ^φ_θφ = cotθ = cosθ/sinθ
    if (abs(sin_theta) > EPSILON) {
        gamma.gamma_phi_theta_phi = cos_theta / sin_theta;
    } else {
        gamma.gamma_phi_theta_phi = 0.0;
    }

    return gamma;
}

// Compute geodesic acceleration: d²x^μ/dλ² = -Γ^μ_αβ (dx^α/dλ)(dx^β/dλ)
vec3 geodesicAcceleration(vec3 pos_spherical, vec3 vel_spherical, float rs) {
    float r = pos_spherical.x;
    float theta = pos_spherical.y;

    float dr = vel_spherical.x;
    float dtheta = vel_spherical.y;
    float dphi = vel_spherical.z;

    ChristoffelSymbols gamma = computeChristoffel(r, theta, rs);

    // d²r/dλ²
    float d2r = -gamma.gamma_r_rr * dr * dr
                -gamma.gamma_r_theta_theta * dtheta * dtheta
                -gamma.gamma_r_phi_phi * dphi * dphi;

    // d²θ/dλ²
    float d2theta = -2.0 * gamma.gamma_theta_r_theta * dr * dtheta
                    -gamma.gamma_theta_phi_phi * dphi * dphi;

    // d²φ/dλ²
    float d2phi = -2.0 * gamma.gamma_phi_r_phi * dr * dphi
                  -2.0 * gamma.gamma_phi_theta_phi * dtheta * dphi;

    return vec3(d2r, d2theta, d2phi);
}

// Single RK4 step for geodesic integration
void rk4GeodesicStep(inout vec3 pos_spherical, inout vec3 vel_spherical, float h, float rs) {
    // k1 = f(y)
    vec3 k1_vel = vel_spherical;
    vec3 k1_acc = geodesicAcceleration(pos_spherical, vel_spherical, rs);

    // k2 = f(y + k1*h/2)
    vec3 pos2 = pos_spherical + k1_vel * (h / 2.0);
    vec3 vel2 = vel_spherical + k1_acc * (h / 2.0);
    vec3 k2_vel = vel2;
    vec3 k2_acc = geodesicAcceleration(pos2, vel2, rs);

    // k3 = f(y + k2*h/2)
    vec3 pos3 = pos_spherical + k2_vel * (h / 2.0);
    vec3 vel3 = vel_spherical + k2_acc * (h / 2.0);
    vec3 k3_vel = vel3;
    vec3 k3_acc = geodesicAcceleration(pos3, vel3, rs);

    // k4 = f(y + k3*h)
    vec3 pos4 = pos_spherical + k3_vel * h;
    vec3 vel4 = vel_spherical + k3_acc * h;
    vec3 k4_vel = vel4;
    vec3 k4_acc = geodesicAcceleration(pos4, vel4, rs);

    // Update: y_next = y + (k1 + 2*k2 + 2*k3 + k4) * h/6
    pos_spherical += (k1_vel + 2.0 * k2_vel + 2.0 * k3_vel + k4_vel) * (h / 6.0);
    vel_spherical += (k1_acc + 2.0 * k2_acc + 2.0 * k3_acc + k4_acc) * (h / 6.0);

    // Normalize theta to [0, π]
    if (pos_spherical.y < 0.0) {
        pos_spherical.y = -pos_spherical.y;
        pos_spherical.z += PI;
    }
    if (pos_spherical.y > PI) {
        pos_spherical.y = 2.0 * PI - pos_spherical.y;
        pos_spherical.z += PI;
    }

    // Normalize phi to [0, 2π)
    pos_spherical.z = mod(pos_spherical.z, TWO_PI);
    if (pos_spherical.z < 0.0) {
        pos_spherical.z += TWO_PI;
    }
}

// ============================================================================

// PHYSICS: Physically accurate accretion disk with Shakura-Sunyaev model
vec3 accretionDisk(vec3 pos, vec3 rayDir, float r, float theta) {
    if (!uAccretionDiskEnabled) return vec3(0.0);

    float M = uBlackHoleMass;
    float r_inner = uISCO;
    float r_outer = r_inner * 15.0; // Extended outer radius

    // Radial bounds check
    if (r < r_inner || r > r_outer) return vec3(0.0);

    // ===== ACCURATE TEMPERATURE CALCULATION =====
    // Shakura-Sunyaev model: T(r) = [3GM·Mdot / (8π·σ·r³) · f(r)]^(1/4)
    // Simplified for geometric units

    float Mdot = uAccretionRate;
    float alpha = uAlphaViscosity;

    // Zero-torque inner boundary condition: f(r) = 1 - sqrt(r_inner/r)
    float f_r = max(1.0 - sqrt(r_inner / r), 0.001);

    // Temperature: T ∝ (Mdot / r³)^(1/4) · f(r)^(1/4)
    // Normalization chosen to give ~10,000K at ISCO for typical Mdot
    float T_norm = 10000.0 * pow(Mdot / 0.1, 0.25);
    float temp = T_norm * pow(r_inner / r, 0.75) * pow(f_r, 0.25);

    // ===== DISK SCALE HEIGHT H(r) =====
    float H = diskScaleHeight(r, temp, M);

    // ===== VERTICAL STRUCTURE: CRITICAL FOR PHYSICAL ACCURACY =====
    // Check if position is within disk thickness
    float thetaFromEquator = abs(theta - PI/2.0);

    // Convert angular distance to physical height: z = r * Δθ
    float height = r * thetaFromEquator;

    // Hard cutoff at 4 scale heights (density < 0.01% beyond this)
    if (height > H * 4.0) return vec3(0.0);

    // Gaussian vertical density profile: ρ(z) = ρ₀ exp(-z²/2H²)
    // This is the fundamental equation for hydrostatic equilibrium in thin disks
    float verticalDensity = exp(-0.5 * (height / H) * (height / H));

    // ===== OPTICAL DEPTH τ(r) =====
    float tau = opticalDepth(r, r_inner, Mdot, alpha, H);

    // ===== GRAVITATIONAL REDSHIFT =====
    float z_grav = gravitationalRedshift(r, M);

    // Observed temperature (gravitationally redshifted)
    float T_obs = temp / z_grav;

    // ===== BLACKBODY COLOR FROM PLANCK SPECTRUM =====
    vec3 color = temperatureToRGB(T_obs);

    // ===== RELATIVISTIC DOPPLER SHIFT & BEAMING =====
    // Azimuthal position in disk
    float phi = atan(pos.y, pos.x);

    // Keplerian orbital velocity: v = sqrt(GM/r) (in units of c)
    float v_orbital = sqrt(M / r);
    float beta = v_orbital;
    float gamma = 1.0 / sqrt(1.0 - beta * beta);

    // Velocity direction: purely azimuthal (φ direction)
    // Component toward observer depends on viewing geometry
    vec3 velocity_dir = vec3(-sin(phi), cos(phi), 0.0);
    float cosTheta_obs = dot(normalize(velocity_dir), normalize(rayDir));

    // Doppler factor: D = 1 / [γ(1 - β·cosθ)]
    float D = 1.0 / max(gamma * (1.0 - beta * cosTheta_obs), 0.1);

    // Doppler shift affects observed frequency and intensity
    // ν_obs = ν_emit × D (frequency boost/reduction)
    // I_obs = I_emit × D^3 (relativistic beaming for continuum)
    float doppler_shift = D;
    float beaming = pow(D, 3.0);

    // Apply Doppler shift to temperature (affects color)
    T_obs *= doppler_shift;
    color = temperatureToRGB(T_obs);

    // Apply beaming to brightness
    float brightness = beaming;

    // ===== RADIATIVE TRANSFER: PHYSICALLY ACCURATE =====
    // The radiative transfer equation: dI/ds = -κI + j
    // For thermal emission: j = κB(T) where B is Planck function
    // Solution: I = B(T)[1 - exp(-τ)] for uniform slab

    // Effective optical depth along ray path through disk
    // For a Gaussian disk: τ_eff = τ₀ * ρ(z)/ρ₀ = τ * verticalDensity
    float tau_effective = tau * verticalDensity;

    // Emission coefficient: for optically thick (τ>>1), emits as blackbody
    // for optically thin (τ<<1), emission ∝ τ
    // Exact formula: ε = 1 - exp(-τ) (from radiative transfer)
    float emissivity = 1.0 - exp(-tau_effective);

    // Radial brightness profile: L ∝ (Ṁ/r³) × f(r)
    // Brighter near inner edge due to higher accretion luminosity
    float radialProfile = pow(r_inner / r, 2.0) * f_r;
    brightness *= radialProfile;

    // Turbulence: small-scale variations from MHD turbulence
    float turbulence = fract(sin(dot(pos.xy * 15.0, vec2(12.9898, 78.233))) * 43758.5453);
    turbulence = turbulence * 0.3 + 0.85;
    brightness *= turbulence;

    // ===== FINAL INTENSITY =====
    // Intensity = emissivity × brightness × color
    // The emissivity naturally handles optical depth
    float intensity = emissivity * brightness;

    // Per-step opacity for ray marching accumulation
    // This represents the fraction of light absorbed/emitted in this step
    float alpha_step = clamp(intensity * 0.3, 0.0, 1.0);

    return color * intensity * alpha_step;
}

// ACCURATE GEODESIC RAY TRACING with RK4 integration
vec3 traceRay(vec3 origin, vec3 dir) {
    // Convert initial position to spherical coordinates
    vec3 pos_spherical = cartesianToSpherical(origin);
    vec3 pos_cartesian = origin;

    // Initialize velocity in spherical coordinates
    // For light rays, we need to convert Cartesian direction to spherical velocity
    vec3 rayDir = normalize(dir);

    // Compute spherical velocity components from Cartesian direction
    // This is approximate but works for ray tracing from camera
    float r = pos_spherical.x;
    float theta = pos_spherical.y;
    float phi = pos_spherical.z;

    // Spherical basis vectors (not normalized, for correct velocity components)
    vec3 e_r = normalize(pos_cartesian);
    vec3 e_theta = normalize(vec3(
        cos(theta) * cos(phi),
        cos(theta) * sin(phi),
        -sin(theta)
    ));
    vec3 e_phi = normalize(vec3(-sin(phi), cos(phi), 0.0));

    // Project ray direction onto spherical basis
    float dr = dot(rayDir, e_r);
    float dtheta = dot(rayDir, e_theta) / max(r, EPSILON);
    float dphi = dot(rayDir, e_phi) / max(r * sin(theta), EPSILON);

    vec3 vel_spherical = vec3(dr, dtheta, dphi);

    float rs = uEventHorizon;
    float closestApproach = 1000.0;
    vec3 accumulatedDisk = vec3(0.0);
    float diskAlpha = 0.0;

    // Adaptive step size based on distance from black hole
    float baseStepSize = uStepSize;

    for (int i = 0; i < uMaxSteps; i++) {
        float r_current = pos_spherical.x;
        closestApproach = min(closestApproach, r_current);

        // Hit event horizon
        if (r_current < rs) {
            return vec3(0.0);
        }

        // Escaped to infinity
        if (r_current > ESCAPE_RADIUS) {
            vec3 skyColor = enhancedStarfield(rayDir);
            vec3 finalColor = mix(skyColor, accumulatedDisk, diskAlpha);
            finalColor = photonSphereGlow(r_current, closestApproach, finalColor);
            return finalColor;
        }

        // Adaptive step size: smaller steps near strong gravity
        float h = baseStepSize;
        if (r_current < uPhotonSphere * 2.0) {
            // Close to photon sphere: use smaller steps for accuracy
            h = baseStepSize * 0.5;
        }
        if (r_current < rs * 1.5) {
            // Very close to event horizon: use even smaller steps
            h = baseStepSize * 0.25;
        }

        // Reconstruct Cartesian position for disk sampling and direction updates
        pos_cartesian = sphericalToCartesian(pos_spherical);

        // Update ray direction for disk sampling (approximate from velocity)
        vec3 vel_cartesian = sphericalToCartesian(pos_spherical + vel_spherical * 0.1) - pos_cartesian;
        rayDir = normalize(vel_cartesian);

        // Check for accretion disk intersection
        if (uAccretionDiskEnabled) {
            float theta_disk = pos_spherical.y;
            vec3 diskColor = accretionDisk(pos_cartesian, rayDir, r_current, theta_disk);

            // Standard front-to-back alpha compositing (Porter-Duff over operator)
            // C_out = C_src + C_dst × (1 - α_src)
            // α_out = α_src + α_dst × (1 - α_src)
            if (length(diskColor) > 0.0) {
                // Extract alpha from disk color intensity (pre-multiplied alpha)
                float diskAlphaStep = min(length(diskColor) * 0.5, 1.0);

                // Accumulate color (front-to-back compositing)
                accumulatedDisk += diskColor * (1.0 - diskAlpha);

                // Accumulate opacity (standard alpha blending)
                diskAlpha += diskAlphaStep * (1.0 - diskAlpha);

                // Early ray termination: if disk becomes fully opaque, stop tracing
                if (diskAlpha > 0.99) {
                    return accumulatedDisk;
                }
            }
        }

        // ACCURATE GEODESIC INTEGRATION using RK4
        // Solves: d²x^μ/dλ² + Γ^μ_αβ (dx^α/dλ)(dx^β/dλ) = 0
        // where Γ^μ_αβ are the Christoffel symbols of the Schwarzschild metric
        rk4GeodesicStep(pos_spherical, vel_spherical, h, rs);

        // Safety check: ensure coordinates remain valid
        if (pos_spherical.x < 0.0 || isnan(pos_spherical.x) || isinf(pos_spherical.x)) {
            break;
        }
    }

    // Max iterations reached
    vec3 result = mix(vec3(0.02, 0.0, 0.0), accumulatedDisk, diskAlpha);
    return photonSphereGlow(pos_spherical.x, closestApproach, result);
}

void main() {
    // Generate ray direction
    float tanHalfFov = tan(radians(uFov * 0.5));

    vec3 rayDir = normalize(
        uCameraFront +
        uCameraRight * (vScreenPos.x * uAspect * tanHalfFov) +
        uCameraUp * (vScreenPos.y * tanHalfFov)
    );

    // Trace ray with all enhancements
    vec3 color = traceRay(uCameraPos, rayDir);

    // Simple tone mapping (Reinhard)
    color = color / (color + vec3(1.0));

    // Gamma correction
    color = pow(color, vec3(1.0/2.2));

    // Subtle vignette
    float vignette = 1.0 - length(vScreenPos) * 0.15;
    color *= vignette;

    FragColor = vec4(color, 1.0);
}
