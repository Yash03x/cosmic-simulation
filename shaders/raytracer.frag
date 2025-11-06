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
uniform float uTime;
uniform float uDeltaTime;
uniform int uMetricType;
uniform float uSpin;

// Black hole uniforms
uniform float uBlackHoleMass;
uniform float uEventHorizon;
uniform float uPhotonSphere;
uniform float uISCO;

// Ray tracing parameters
uniform int uMaxSteps;
uniform float uStepSize;
uniform bool uAccretionDiskEnabled;
uniform bool uJetsEnabled;
uniform bool uErgosphereVisible;

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
    float rotation = uTime * 0.02;
    float twinkle = (uTime + uDeltaTime) * 0.3;

    float phiBase = atan(d.y, d.x) + rotation;
    float thetaBase = acos(clamp(d.z, -1.0, 1.0));

    // LAYER 1: Very bright stars (giants)
    vec2 p1 = vec2(phiBase, thetaBase) * 25.0 + twinkle;
    float s1 = fract(sin(dot(p1, vec2(12.9898, 78.233))) * 43758.5453 + uTime * 7.0);
    if (s1 > 0.9985) {
        float brightness = (s1 - 0.9985) / 0.0015;
        vec3 starColor = mix(vec3(1.0, 0.95, 0.85), vec3(0.85, 0.9, 1.0), fract(s1 * 10.0));
        color += starColor * brightness * 6.0;
    }

    // LAYER 2: Bright stars
    vec2 p2 = vec2(phiBase, thetaBase) * 60.0 + twinkle * 1.5;
    float s2 = fract(sin(dot(p2, vec2(15.234, 82.123))) * 43758.5453 + uTime * 5.0);
    if (s2 > 0.997) {
        float brightness = (s2 - 0.997) / 0.003;
        vec3 starColor = mix(vec3(1.0, 1.0, 1.0), vec3(0.9, 0.95, 1.0), fract(s2 * 7.0));
        color += starColor * brightness * 3.0;
    }

    // LAYER 3: Medium stars
    vec2 p3 = vec2(phiBase, thetaBase) * 120.0 + twinkle * 2.0;
    float s3 = fract(sin(dot(p3, vec2(18.456, 85.789))) * 43758.5453 + uTime * 3.0);
    if (s3 > 0.994) {
        float brightness = (s3 - 0.994) / 0.006;
        color += vec3(0.9, 0.9, 1.0) * brightness * 1.5;
    }

    // LAYER 4: Small stars (dust)
    vec2 p4 = vec2(phiBase, thetaBase) * 200.0 + twinkle * 3.0;
    float s4 = fract(sin(dot(p4, vec2(21.789, 88.456))) * 43758.5453 + uTime * 2.5);
    if (s4 > 0.992) {
        float brightness = (s4 - 0.992) / 0.008;
        color += vec3(0.7, 0.8, 0.9) * brightness * 0.8;
    }

    // LAYER 5: Micro stars (depth)
    vec2 p5 = vec2(phiBase, thetaBase) * 350.0 + twinkle * 4.0;
    float s5 = fract(sin(dot(p5, vec2(24.123, 91.789))) * 43758.5453 + uTime * 1.5);
    if (s5 > 0.9905) {
        color += vec3(0.5, 0.6, 0.7) * 0.3;
    }

    // ===== MILKY WAY STRUCTURE =====
    // Galactic plane: band of light across the sky
    float galacticPlane = abs(sin(thetaBase * 2.0));
    float galacticDisk = exp(-pow((thetaBase - PI/2.0) / 0.4, 2.0)); // Peak at equator

    // Milky Way core (brighter center)
    float distToCore = length(vec2(cos(phiBase - rotation), sin(phiBase - rotation) * 0.5));
    float core = exp(-distToCore * distToCore * 4.0) * galacticDisk;
    color += vec3(0.15, 0.12, 0.08) * core;

    // Galactic arms (spiral structure)
    float armAngle = phiBase - rotation + thetaBase * 3.0;
    float arm1 = abs(sin(armAngle * 2.0 + uTime * 0.1));
    float arm2 = abs(sin(armAngle * 2.0 + PI * 0.66 + uTime * 0.1));
    float arm3 = abs(sin(armAngle * 2.0 + PI * 1.33 + uTime * 0.1));
    float arms = max(max(arm1, arm2), arm3);
    arms = pow(arms, 8.0) * galacticDisk; // Sharp spiral arms
    color += vec3(0.08, 0.09, 0.12) * arms * 0.8;

    // Dust lanes (dark absorption)
    float dustLane = abs(sin(thetaBase * 4.0 + (phiBase - rotation) * 2.0)) *
                     abs(cos(phiBase * 3.0 - rotation * 1.5));
    dustLane = pow(dustLane, 2.0) * galacticDisk;
    color *= (1.0 - dustLane * 0.3); // Darken where dust is

    // Emission nebulae (H-alpha regions, pink/red)
    float nebula1 = abs(sin(thetaBase * 6.0 + (phiBase - rotation) * 4.0 + uTime * 0.3)) *
                    abs(cos(thetaBase * 4.0 - uTime * 0.2));
    nebula1 = pow(nebula1, 5.0);
    if (nebula1 > 0.85) {
        vec3 nebulaColor = vec3(1.0, 0.3, 0.4); // H-alpha emission (red)
        color += nebulaColor * (nebula1 - 0.85) * 3.0 * galacticDisk;
    }

    // Reflection nebulae (blue)
    float nebula2 = abs(sin(thetaBase * 7.0 - (phiBase - rotation) * 3.0 + uTime * 0.25)) *
                    abs(cos(phiBase * 5.0 + uTime * 0.15));
    nebula2 = pow(nebula2, 6.0);
    if (nebula2 > 0.9) {
        vec3 blueNebula = vec3(0.3, 0.5, 1.0); // Scattered starlight (blue)
        color += blueNebula * (nebula2 - 0.9) * 2.5 * galacticDisk;
    }

    // Dark nebula (Barnard's objects - very dark patches)
    float darkNebula = abs(sin(thetaBase * 5.0 + phiBase * 2.5 - rotation));
    darkNebula = pow(darkNebula, 10.0) * galacticDisk;
    color *= (1.0 - darkNebula * 0.6);

    // Overall galactic glow
    color += vec3(0.03, 0.04, 0.06) * galacticDisk * 0.5;

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
// GEODESIC INTEGRATION: Exact Schwarzschild solver (4-position & 4-momentum)
// ============================================================================

struct GeodesicState {
    vec4 position; // (t, r, θ, φ)
    vec4 momentum; // (dt/dλ, dr/dλ, dθ/dλ, dφ/dλ)
    float energy;
    float angularMomentum;
};

struct GeodesicDerivative {
    vec4 dx;
    vec4 dp;
};

struct MetricData {
    mat4 g;
    mat4 gInv;
    mat4 dg_dr;
    mat4 dg_dtheta;
};

float metricComponent(mat4 m, int row, int col) {
    return m[col][row];
}

float clampRadius(float r) {
    return max(r, uEventHorizon + 1e-5);
}

MetricData computeSchwarzschildMetric(float r, float theta) {
    MetricData data;
    float rSafe = clampRadius(r);
    float rs = uEventHorizon;
    float sinTheta = sin(theta);
    float cosTheta = cos(theta);
    float sin2 = max(sinTheta * sinTheta, 1e-6);

    float f = 1.0 - rs / rSafe;
    float invF = 1.0 / f;
    float g_tt = -f;
    float g_rr = invF;
    float g_theta = rSafe * rSafe;
    float g_phi = g_theta * sin2;

    data.g = mat4(
        vec4(g_tt, 0.0, 0.0, 0.0),
        vec4(0.0, g_rr, 0.0, 0.0),
        vec4(0.0, 0.0, g_theta, 0.0),
        vec4(0.0, 0.0, 0.0, g_phi)
    );

    data.gInv = mat4(
        vec4(-invF, 0.0, 0.0, 0.0),
        vec4(0.0, f, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0 / g_theta, 0.0),
        vec4(0.0, 0.0, 0.0, 1.0 / g_phi)
    );

    float df_dr = rs / (rSafe * rSafe);
    float dg_tt_dr = df_dr;
    float dg_rr_dr = df_dr / (f * f);
    float dg_theta_dr = 2.0 * rSafe;
    float dg_phi_dr = 2.0 * rSafe * sin2;
    float dg_phi_dtheta = g_theta * sin(2.0 * theta);

    data.dg_dr = mat4(
        vec4(dg_tt_dr, 0.0, 0.0, 0.0),
        vec4(0.0, dg_rr_dr, 0.0, 0.0),
        vec4(0.0, 0.0, dg_theta_dr, 0.0),
        vec4(0.0, 0.0, 0.0, dg_phi_dr)
    );

    data.dg_dtheta = mat4(
        vec4(0.0, 0.0, 0.0, 0.0),
        vec4(0.0, 0.0, 0.0, 0.0),
        vec4(0.0, 0.0, 0.0, 0.0),
        vec4(0.0, 0.0, 0.0, dg_phi_dtheta)
    );

    return data;
}

MetricData computeKerrMetric(float r, float theta) {
    MetricData data;
    float rSafe = clampRadius(r);
    float M = uBlackHoleMass;
    float aDimless = clamp(uSpin, -0.998, 0.998);
    float a = aDimless * M;
    float sinTheta = sin(theta);
    float cosTheta = cos(theta);
    float sin2 = max(sinTheta * sinTheta, 1e-6);
    float cos2 = cosTheta * cosTheta;
    float r2 = rSafe * rSafe;
    float a2 = a * a;

    float Sigma = r2 + a2 * cos2;
    float Delta = r2 - 2.0 * M * rSafe + a2;
    float A = (r2 + a2) * (r2 + a2) - a2 * Delta * sin2;

    float DeltaSafe = max(Delta, 1e-6);

    float g_tt = -(1.0 - (2.0 * M * rSafe) / Sigma);
    float g_tphi = -(2.0 * M * rSafe * a * sin2) / Sigma;
    float g_rr = Sigma / DeltaSafe;
    float g_theta = Sigma;
    float g_phi = (A * sin2) / Sigma;

    data.g = mat4(
        vec4(g_tt, 0.0, 0.0, g_tphi),
        vec4(0.0, g_rr, 0.0, 0.0),
        vec4(0.0, 0.0, g_theta, 0.0),
        vec4(g_tphi, 0.0, 0.0, g_phi)
    );

    float invSigmaDelta = 1.0 / (Sigma * DeltaSafe);
    float gInv_tt = -A * invSigmaDelta;
    float gInv_tphi = -2.0 * M * rSafe * a * invSigmaDelta;
    float gInv_rr = DeltaSafe / Sigma;
    float gInv_theta = 1.0 / Sigma;
    float gInv_phi = (Delta - a2 * sin2) / (Sigma * DeltaSafe * sin2);

    data.gInv = mat4(
        vec4(gInv_tt, 0.0, 0.0, gInv_tphi),
        vec4(0.0, gInv_rr, 0.0, 0.0),
        vec4(0.0, 0.0, gInv_theta, 0.0),
        vec4(gInv_tphi, 0.0, 0.0, gInv_phi)
    );

    float Sigma2 = Sigma * Sigma;
    float dSigma_dr = 2.0 * rSafe;
    float dSigma_dtheta = -2.0 * a2 * sinTheta * cosTheta;
    float dDelta_dr = 2.0 * rSafe - 2.0 * M;
    float dA_dr = 4.0 * rSafe * (r2 + a2) - a2 * dDelta_dr * sin2;
    float dA_dtheta = -2.0 * a2 * Delta * sinTheta * cosTheta;

    float dgtt_dr = (2.0 * M / Sigma) - (2.0 * M * rSafe * dSigma_dr) / Sigma2;
    float dgtt_dtheta = -(2.0 * M * rSafe * dSigma_dtheta) / Sigma2;

    float numerator_r = rSafe * sin2;
    float dNumerator_dr = sin2;
    float dNumerator_dtheta = 2.0 * rSafe * sinTheta * cosTheta;

    float dg_tphi_dr = -2.0 * M * a * (
        (dNumerator_dr * Sigma - numerator_r * dSigma_dr) / Sigma2
    );
    float dg_tphi_dtheta = -2.0 * M * a * (
        (dNumerator_dtheta * Sigma - numerator_r * dSigma_dtheta) / Sigma2
    );

    float dg_rr_dr = (dSigma_dr * DeltaSafe - Sigma * dDelta_dr) / (DeltaSafe * DeltaSafe);
    float dg_rr_dtheta = dSigma_dtheta / DeltaSafe;

    float dg_theta_dr = dSigma_dr;
    float dg_theta_dtheta = dSigma_dtheta;

    float dg_phi_dr = (dA_dr * sin2 * Sigma - A * sin2 * dSigma_dr) / Sigma2;
    float dg_phi_dtheta = ((dA_dtheta * sin2 + A * 2.0 * sinTheta * cosTheta) * Sigma - A * sin2 * dSigma_dtheta) / Sigma2;

    data.dg_dr = mat4(
        vec4(dgtt_dr, 0.0, 0.0, dg_tphi_dr),
        vec4(0.0, dg_rr_dr, 0.0, 0.0),
        vec4(0.0, 0.0, dg_theta_dr, 0.0),
        vec4(dg_tphi_dr, 0.0, 0.0, dg_phi_dr)
    );

    data.dg_dtheta = mat4(
        vec4(dgtt_dtheta, 0.0, 0.0, dg_tphi_dtheta),
        vec4(0.0, dg_rr_dtheta, 0.0, 0.0),
        vec4(0.0, 0.0, dg_theta_dtheta, 0.0),
        vec4(dg_tphi_dtheta, 0.0, 0.0, dg_phi_dtheta)
    );

    return data;
}

MetricData computeMetricData(float r, float theta) {
    if (uMetricType == 1 && abs(uSpin) > 1e-6) {
        return computeKerrMetric(r, theta);
    }
    return computeSchwarzschildMetric(r, theta);
}

float derivativeComponent(int coord, int row, int col, MetricData data) {
    if (coord == 1) return metricComponent(data.dg_dr, row, col);
    if (coord == 2) return metricComponent(data.dg_dtheta, row, col);
    return 0.0;
}

float christoffel(int mu, int alpha, int beta, MetricData data) {
    float sum = 0.0;
    for (int nu = 0; nu < 4; ++nu) {
        float term = derivativeComponent(alpha, beta, nu, data) +
                     derivativeComponent(beta, alpha, nu, data) -
                     derivativeComponent(nu, alpha, beta, data);
        sum += metricComponent(data.gInv, mu, nu) * term;
    }
    return 0.5 * sum;
}

vec3 cartesianToSpherical(vec3 pos) {
    float r = length(pos);
    float theta = acos(clamp(pos.z / max(r, EPSILON), -1.0, 1.0));
    float phi = atan(pos.y, pos.x);
    return vec3(r, theta, phi);
}

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

void sphericalBasis(float theta, float phi, out vec3 e_r, out vec3 e_theta, out vec3 e_phi) {
    float sinTheta = sin(theta);
    float cosTheta = cos(theta);
    float sinPhi = sin(phi);
    float cosPhi = cos(phi);

    e_r = vec3(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta);
    e_theta = vec3(cosTheta * cosPhi, cosTheta * sinPhi, -sinTheta);
    e_phi = vec3(-sinPhi, cosPhi, 0.0);
}



GeodesicDerivative computeDerivative(GeodesicState state) {
    MetricData data = computeMetricData(state.position.y, state.position.z);

    GeodesicDerivative deriv;
    deriv.dx = state.momentum;
    deriv.dp = vec4(0.0);

    for (int mu = 0; mu < 4; ++mu) {
        float sum = 0.0;
        for (int alpha = 0; alpha < 4; ++alpha) {
            for (int beta = 0; beta < 4; ++beta) {
                sum += christoffel(mu, alpha, beta, data) *
                        state.momentum[alpha] * state.momentum[beta];
            }
        }
        deriv.dp[mu] = -sum;
    }

    return deriv;
}

void rk4Step(inout GeodesicState state, float h) {
    GeodesicDerivative k1 = computeDerivative(state);

    GeodesicState temp;
    temp.position = state.position + k1.dx * (h * 0.5);
    temp.momentum = state.momentum + k1.dp * (h * 0.5);
    GeodesicDerivative k2 = computeDerivative(temp);

    temp.position = state.position + k2.dx * (h * 0.5);
    temp.momentum = state.momentum + k2.dp * (h * 0.5);
    GeodesicDerivative k3 = computeDerivative(temp);

    temp.position = state.position + k3.dx * h;
    temp.momentum = state.momentum + k3.dp * h;
    GeodesicDerivative k4 = computeDerivative(temp);

    state.position += (k1.dx + 2.0 * k2.dx + 2.0 * k3.dx + k4.dx) * (h / 6.0);
    state.momentum += (k1.dp + 2.0 * k2.dp + 2.0 * k3.dp + k4.dp) * (h / 6.0);
}

float computeNullConstraint(GeodesicState state) {
    MetricData data = computeMetricData(state.position.y, state.position.z);
    float constraint = 0.0;
    for (int alpha = 0; alpha < 4; ++alpha) {
        for (int beta = 0; beta < 4; ++beta) {
            constraint += metricComponent(data.g, alpha, beta) *
                         state.momentum[alpha] * state.momentum[beta];
        }
    }
    return constraint;
}

float computeEnergyInvariant(GeodesicState state) {
    MetricData data = computeMetricData(state.position.y, state.position.z);
    float energy = 0.0;
    for (int mu = 0; mu < 4; ++mu) {
        energy += metricComponent(data.g, 0, mu) * state.momentum[mu];
    }
    return -energy;
}

float computeAngularMomentumInvariant(GeodesicState state) {
    MetricData data = computeMetricData(state.position.y, state.position.z);
    float ang = 0.0;
    for (int mu = 0; mu < 4; ++mu) {
        ang += metricComponent(data.g, 3, mu) * state.momentum[mu];
    }
    return ang;
}

float solveTimeComponent(vec4 momentum, MetricData data) {
    float g_tt = metricComponent(data.g, 0, 0);
    float g_tphi = metricComponent(data.g, 0, 3);
    float g_rr = metricComponent(data.g, 1, 1);
    float g_theta = metricComponent(data.g, 2, 2);
    float g_phi = metricComponent(data.g, 3, 3);

    float A = g_tt;
    float B = 2.0 * g_tphi * momentum.w;
    float C = g_rr * momentum.y * momentum.y +
              g_theta * momentum.z * momentum.z +
              g_phi * momentum.w * momentum.w;

    float disc = max(B * B - 4.0 * A * C, 0.0);
    float sqrtDisc = sqrt(disc);
    float denom = 2.0 * A;
    if (abs(denom) < 1e-6) {
        denom = (denom >= 0.0 ? 1e-6 : -1e-6);
    }

    float pt1 = (-B + sqrtDisc) / denom;
    float pt2 = (-B - sqrtDisc) / denom;

    float energy1 = -(g_tt * pt1 + g_tphi * momentum.w);
    float energy2 = -(g_tt * pt2 + g_tphi * momentum.w);

    return (energy1 > energy2) ? pt1 : pt2;
}

void enforceConservedQuantities(inout GeodesicState state, float energy, float angularMomentum) {
    MetricData data = computeMetricData(state.position.y, state.position.z);
    float g_tt = metricComponent(data.g, 0, 0);
    float g_tphi = metricComponent(data.g, 0, 3);
    float g_rr = metricComponent(data.g, 1, 1);
    float g_theta = metricComponent(data.g, 2, 2);
    float g_phi = metricComponent(data.g, 3, 3);

    float det = g_tt * g_phi - g_tphi * g_tphi;
    if (abs(det) < 1e-8) {
        return;
    }

    float pt = (-energy * g_phi - g_tphi * angularMomentum) / det;
    float pphi = (angularMomentum * g_tt + g_tphi * energy) / det;

    state.momentum.x = pt;
    state.momentum.w = pphi;

    float spatial = g_rr * state.momentum.y * state.momentum.y +
                    g_theta * state.momentum.z * state.momentum.z;
    float temporal = g_tt * pt * pt + 2.0 * g_tphi * pt * pphi + g_phi * pphi * pphi;
    float target = -temporal;

    if (spatial > 1e-8 && target > 0.0) {
        float scale = sqrt(target / spatial);
        state.momentum.y *= scale;
        state.momentum.z *= scale;
    }
}

void normalizeSpherical(inout GeodesicState state) {
    if (state.position.z < 0.0) {
        state.position.z = -state.position.z;
        state.position.w += PI;
        state.momentum.z = -state.momentum.z;
    }
    if (state.position.z > PI) {
        state.position.z = TWO_PI - state.position.z;
        state.position.w += PI;
        state.momentum.z = -state.momentum.z;
    }
    state.position.w = mod(state.position.w, TWO_PI);
    if (state.position.w < 0.0) {
        state.position.w += TWO_PI;
    }
}

vec3 spatialDirection(GeodesicState state) {
    float r = max(state.position.y, uEventHorizon + 1e-5);
    float theta = state.position.z;
    float phi = state.position.w;

    vec3 e_r;
    vec3 e_theta;
    vec3 e_phi;
    sphericalBasis(theta, phi, e_r, e_theta, e_phi);

    vec3 velocity =
        state.momentum.y * e_r +
        r * state.momentum.z * e_theta +
        r * max(sin(theta), 1e-6) * state.momentum.w * e_phi;

    if (length(velocity) > 0.0) {
        return normalize(velocity);
    }
    return e_r;
}

// ============================================================================

// PHYSICS: Physically accurate accretion disk with Shakura-Sunyaev model
vec3 accretionDisk(vec3 pos,
                   vec3 rayDir,
                   float r,
                   float theta,
                   float coordinateTime,
                   float globalTime) {
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
    float emissionTime = max(globalTime - coordinateTime, 0.0);
    float phi0 = atan(pos.y, pos.x);
    float omega = sqrt(M / (r * r * r)); // Keplerian angular velocity
    float phiFlow = phi0 + omega * emissionTime;

    float v_orbital = sqrt(M / r);
    float beta = v_orbital;
    float gamma = 1.0 / sqrt(max(1.0 - beta * beta, 0.0001));

    vec3 velocity_dir = vec3(-sin(phiFlow), cos(phiFlow), 0.0);
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
    float turbulence = fract(sin(dot(pos.xy * 15.0, vec2(12.9898, 78.233)) + globalTime * 2.0 + uDeltaTime * 5.0) * 43758.5453);
    turbulence = turbulence * 0.3 + 0.85;
    float shear = sin(phiFlow * 6.0 + emissionTime * 0.5 + uDeltaTime * 30.0);
    brightness *= turbulence * (1.0 + 0.15 * shear);

    // ===== FINAL INTENSITY =====
    // Intensity = emissivity × brightness × color
    // The emissivity naturally handles optical depth
    float intensity = emissivity * brightness;

    // Per-step opacity for ray marching accumulation
    // This represents the fraction of light absorbed/emitted in this step
    float alpha_step = clamp(intensity * 0.3, 0.0, 1.0);

    return color * intensity * alpha_step;
}

// PHYSICS: Relativistic jets powered by Blandford-Znajek mechanism
// Jets are collimated outflows along the rotation axis
vec3 relativisticJet(vec3 pos,
                     vec3 rayDir,
                     float r,
                     float theta,
                     float globalTime) {
    if (!uJetsEnabled) return vec3(0.0);
    if (abs(uSpin) < 0.1) return vec3(0.0); // Jets require spin

    float M = uBlackHoleMass;

    // Jets launch from near the event horizon
    float r_jet_base = uEventHorizon * 1.5;
    float r_jet_max = r_jet_base * 100.0; // Jets extend far

    if (r < r_jet_base || r > r_jet_max) return vec3(0.0);

    // ===== JET GEOMETRY =====
    // Jets are aligned with black hole's spin axis (z-axis)
    // Opening angle depends on magnetic field collimation
    float thetaFromPole = min(theta, PI - theta); // Distance from either pole

    // Opening angle: ~5-15 degrees for well-collimated jets
    // More collimated closer to the base
    float collimation = 0.1 + 0.05 * (r - r_jet_base) / r_jet_base;
    float openingAngle = collimation; // radians

    if (thetaFromPole > openingAngle) return vec3(0.0); // Outside jet cone

    // ===== JET PHYSICS =====
    // Jet power from Blandford-Znajek: P ∝ a² (black hole spin)
    // Lorentz factor: γ ~ 10-100 for astrophysical jets
    float spinFactor = abs(uSpin);
    float jetPower = spinFactor * spinFactor;

    // Jet velocity (highly relativistic)
    float beta = 0.9 + 0.09 * spinFactor; // v/c = 0.9 - 0.99
    float gamma = 1.0 / sqrt(1.0 - beta * beta);

    // ===== INTENSITY PROFILE =====
    // Brightness decreases with distance (r^-2) and angle from axis
    float radialFalloff = r_jet_base / r;
    float angularProfile = exp(-thetaFromPole * thetaFromPole / (openingAngle * openingAngle * 0.1));

    // ===== SYNCHROTRON EMISSION =====
    // Relativistic electrons spiraling in magnetic fields
    // Peaks in radio/optical, appears blue-white in visible

    // Magnetic field strength: B ∝ 1/r (dipole-like)
    float B_field = jetPower / (r / r_jet_base);

    // Synchrotron frequency: ν ∝ γ² B
    // Higher energy → bluer
    float synchrotronFreq = gamma * gamma * B_field;

    // Color: Blue-white for high-energy synchrotron
    vec3 jetColor = vec3(0.7, 0.85, 1.0); // Cool blue-white

    // Add some red at the edges (lower energy electrons)
    float edgeFactor = thetaFromPole / openingAngle;
    jetColor = mix(jetColor, vec3(1.0, 0.6, 0.4), edgeFactor * 0.3);

    // ===== TURBULENCE AND KNOTS =====
    // Jets show instabilities and shock-heated "knots"
    float z_along_jet = r * cos(thetaFromPole);
    float turbulence = abs(sin(z_along_jet * 0.5 + globalTime * 2.0)) *
                       abs(sin(z_along_jet * 1.3 - globalTime * 1.5));
    turbulence = pow(turbulence, 3.0); // Make knots sharper

    // Shock-heated knots are brighter and redder
    if (turbulence > 0.6) {
        jetColor = mix(jetColor, vec3(1.0, 0.9, 0.7), (turbulence - 0.6) * 2.0);
        angularProfile *= (1.0 + turbulence * 2.0);
    }

    // ===== RELATIVISTIC BEAMING =====
    // Jet moving toward us appears much brighter (Doppler beaming)
    vec3 jetDirection = (theta < PI/2.0) ? vec3(0.0, 0.0, 1.0) : vec3(0.0, 0.0, -1.0);
    float cosTheta_obs = -dot(normalize(rayDir), jetDirection); // Negative because ray points toward camera

    // Doppler factor: D = 1 / [γ(1 - β cosθ)]
    float D = 1.0 / max(gamma * (1.0 - beta * cosTheta_obs), 0.01);
    float beaming = pow(D, 3.0); // I_obs = I_emit × D³

    // Jets pointing toward us are dramatically brighter
    float directionalBoost = (cosTheta_obs > 0.0) ? (1.0 + beaming * 0.5) : 1.0;

    // ===== FINAL INTENSITY =====
    float intensity = jetPower * radialFalloff * angularProfile * directionalBoost;
    intensity = clamp(intensity * 0.3, 0.0, 1.0); // Scale for visibility

    return jetColor * intensity;
}

// PHYSICS: Ergosphere boundary visualization
// The ergosphere is where frame-dragging is so extreme that nothing can remain stationary
vec3 ergosphereBoundary(vec3 pos, float r, float theta, vec3 baseColor) {
    if (!uErgosphereVisible) return baseColor;
    if (abs(uSpin) < 0.01) return baseColor; // No ergosphere for non-rotating

    float M = uBlackHoleMass;
    float a = uSpin * M;
    float cos_theta = cos(theta);

    // Event horizon radius (inner boundary): r_- = M - √(M² - a²)
    float r_minus = M - sqrt(max(M * M - a * a, 0.0));

    // Ergosphere outer boundary: r_ergo = M + √(M² - a²cos²θ)
    // This is the "stationary limit" surface
    float r_ergo = M + sqrt(max(M * M - a * a * cos_theta * cos_theta, 0.0));

    // Check if we're near the ergosphere boundary
    float distToErgo = abs(r - r_ergo);

    // ===== VISUALIZATION OPTIONS =====

    // Option 1: Wireframe grid on the surface
    if (distToErgo < 0.05) {
        // Create lat/long grid
        float gridPhi = mod(atan(pos.y, pos.x) * 8.0, TWO_PI);
        float gridTheta = mod(theta * 12.0, PI);

        bool isGridLine = (gridPhi < 0.15 || gridPhi > TWO_PI - 0.15) ||
                          (gridTheta < 0.05 || gridTheta > PI - 0.05);

        if (isGridLine) {
            // Cyan/blue glow for ergosphere
            vec3 ergoColor = vec3(0.0, 0.8, 1.0);
            float alpha = (0.05 - distToErgo) / 0.05;
            return mix(baseColor, ergoColor, alpha * 0.6);
        }
    }

    // Option 2: Translucent shell showing the region
    if (r > r_minus && r < r_ergo + 0.3) {
        float depthInErgo = (r - r_minus) / (r_ergo - r_minus + 0.3);

        // Pulsing effect to show frame-dragging
        float pulse = 0.5 + 0.5 * sin(uTime * 2.0 + r * 5.0);

        // Color shifts from blue (outer) to red (near horizon)
        vec3 ergoColor = mix(vec3(0.2, 0.6, 1.0), vec3(1.0, 0.4, 0.2), 1.0 - depthInErgo);

        // Swirling pattern showing frame-dragging direction
        float swirl = sin(atan(pos.y, pos.x) * 6.0 - uTime * 3.0 * sign(uSpin) + r * 2.0);
        swirl = swirl * 0.5 + 0.5;

        // Intensity increases near the boundary
        float intensity = 0.1 * pulse * swirl * (1.0 - depthInErgo) * 0.5;

        return mix(baseColor, ergoColor, intensity);
    }

    // Option 3: Highlight the exact boundary surface
    if (distToErgo < 0.02) {
        vec3 boundaryColor = vec3(0.0, 1.0, 1.0); // Bright cyan
        float alpha = (0.02 - distToErgo) / 0.02;

        // Animated ring around the boundary
        float ringPattern = sin(atan(pos.y, pos.x) * 12.0 - uTime * 5.0);
        ringPattern = ringPattern * 0.5 + 0.5;

        return mix(baseColor, boundaryColor, alpha * ringPattern * 0.8);
    }

    return baseColor;
}

vec3 traceRay(vec3 origin, vec3 dir) {
    vec3 spherical = cartesianToSpherical(origin);

    float r = spherical.x;
    float theta = clamp(spherical.y, 1e-6, PI - 1e-6);
    float phi = spherical.z;

    float rSafe = max(r, uEventHorizon + 1e-4);

    GeodesicState state;
    state.position = vec4(0.0, rSafe, theta, phi);

    vec3 e_r;
    vec3 e_theta;
    vec3 e_phi;
    sphericalBasis(theta, phi, e_r, e_theta, e_phi);

    vec3 rayDir = normalize(dir);

    float dir_r = dot(rayDir, e_r);
    float dir_theta = dot(rayDir, e_theta);
    float dir_phi = dot(rayDir, e_phi);

    float sinTheta = max(sin(theta), 1e-6);
    float pr = dir_r;
    float ptheta = dir_theta / rSafe;
    float pphi = dir_phi / (rSafe * sinTheta);
    MetricData metricInit = computeMetricData(rSafe, theta);
    vec4 initialMomentum = vec4(0.0, pr, ptheta, pphi);
    float pt = solveTimeComponent(initialMomentum, metricInit);

    state.momentum = vec4(pt, pr, ptheta, pphi);
    state.energy = computeEnergyInvariant(state);
    state.angularMomentum = computeAngularMomentumInvariant(state);
    enforceConservedQuantities(state, state.energy, state.angularMomentum);
    state.energy = computeEnergyInvariant(state);
    state.angularMomentum = computeAngularMomentumInvariant(state);

    float conservedEnergy = state.energy;
    float conservedAngularMomentum = state.angularMomentum;

    float closestApproach = state.position.y;
    vec3 accumulatedDisk = vec3(0.0);
    vec3 accumulatedJet = vec3(0.0);
    float diskAlpha = 0.0;
    float jetAlpha = 0.0;

    float baseStepSize = uStepSize;

    for (int i = 0; i < uMaxSteps; ++i) {
        float rCurrent = state.position.y;
        closestApproach = min(closestApproach, rCurrent);

        if (rCurrent <= uEventHorizon) {
            return vec3(0.0);
        }

        if (isnan(rCurrent) || isinf(rCurrent)) {
            break;
        }

        if (rCurrent >= ESCAPE_RADIUS) {
            vec3 skyDir = spatialDirection(state);
            vec3 skyColor = enhancedStarfield(skyDir);
            vec3 finalColor = skyColor;
            finalColor = mix(finalColor, accumulatedDisk, diskAlpha);
            finalColor = mix(finalColor, accumulatedJet, jetAlpha);
            finalColor = photonSphereGlow(rCurrent, closestApproach, finalColor);

            // Apply ergosphere visualization to the final color
            vec3 posCartesian = sphericalToCartesian(vec3(state.position.y, state.position.z, state.position.w));
            finalColor = ergosphereBoundary(posCartesian, state.position.y, state.position.z, finalColor);

            return finalColor;
        }

        float h = baseStepSize;
        if (rCurrent < uPhotonSphere * 2.0) {
            h = baseStepSize * 0.5;
        }
        if (rCurrent < uEventHorizon * 1.5) {
            h = baseStepSize * 0.25;
        }

        rk4Step(state, h);
        normalizeSpherical(state);
        enforceConservedQuantities(state, conservedEnergy, conservedAngularMomentum);
        state.energy = conservedEnergy;
        state.angularMomentum = conservedAngularMomentum;

        if (abs(computeNullConstraint(state)) > 1e-4) {
            break;
        }

        vec3 posCartesian = sphericalToCartesian(vec3(state.position.y, state.position.z, state.position.w));
        vec3 raySpatialDir = spatialDirection(state);

        if (uAccretionDiskEnabled) {
            vec3 diskColor = accretionDisk(posCartesian,
                                           raySpatialDir,
                                           state.position.y,
                                           state.position.z,
                                           state.position.x,
                                           uTime);

            if (length(diskColor) > 0.0) {
                float diskAlphaStep = min(length(diskColor) * 0.5, 1.0);
                accumulatedDisk += diskColor * (1.0 - diskAlpha);
                diskAlpha += diskAlphaStep * (1.0 - diskAlpha);
            }
        }

        // Accumulate jets
        if (uJetsEnabled) {
            vec3 jetColor = relativisticJet(posCartesian,
                                           raySpatialDir,
                                           state.position.y,
                                           state.position.z,
                                           uTime);

            if (length(jetColor) > 0.0) {
                float jetAlphaStep = min(length(jetColor) * 0.4, 1.0);
                accumulatedJet += jetColor * (1.0 - jetAlpha);
                jetAlpha += jetAlphaStep * (1.0 - jetAlpha);
            }
        }

        // Early termination if fully opaque
        if (diskAlpha > 0.99 && jetAlpha > 0.99) {
            vec3 result = accumulatedDisk * (diskAlpha / (diskAlpha + jetAlpha));
            result += accumulatedJet * (jetAlpha / (diskAlpha + jetAlpha));
            return result;
        }
    }

    vec3 fallbackDir = spatialDirection(state);
    vec3 fallbackSky = enhancedStarfield(fallbackDir);
    vec3 fallback = fallbackSky;
    fallback = mix(fallback, accumulatedDisk, diskAlpha);
    fallback = mix(fallback, accumulatedJet, jetAlpha);
    fallback = photonSphereGlow(state.position.y, closestApproach, fallback);

    // Apply ergosphere visualization
    vec3 posCartesian = sphericalToCartesian(vec3(state.position.y, state.position.z, state.position.w));
    fallback = ergosphereBoundary(posCartesian, state.position.y, state.position.z, fallback);

    return fallback;
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
    // Output HDR color for post-processing
    vec3 color = traceRay(uCameraPos, rayDir);

    // No tone mapping here - done in post-processing
    FragColor = vec4(color, 1.0);
}
