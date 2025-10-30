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

// Constants
#define PI 3.14159265359
#define TWO_PI 6.28318530718
#define ESCAPE_RADIUS 100.0
#define EPSILON 1e-6

// Convert Cartesian to spherical coordinates
vec3 cartesianToSpherical(vec3 pos) {
    float r = length(pos);
    float theta = acos(clamp(pos.z / r, -1.0, 1.0));
    float phi = atan(pos.y, pos.x);
    return vec3(r, theta, phi);
}

// Convert spherical to Cartesian coordinates
vec3 sphericalToCartesian(vec3 sph) {
    float r = sph.x;
    float theta = sph.y;
    float phi = sph.z;
    return vec3(
        r * sin(theta) * cos(phi),
        r * sin(theta) * sin(phi),
        r * cos(theta)
    );
}

// Compute Schwarzschild geodesic acceleration
// Returns acceleration in spherical coordinates
vec3 schwarzschildAcceleration(vec3 pos, vec3 vel) {
    float r = pos.x;
    float theta = pos.y;

    float dr = vel.x;
    float dtheta = vel.y;
    float dphi = vel.z;

    float rs = 2.0 * uBlackHoleMass;

    // Avoid singularity at event horizon
    if (r <= rs * 1.001) {
        return vec3(0.0);
    }

    // Christoffel symbols for Schwarzschild metric
    float r2 = r * r;
    float sin_theta = sin(theta);
    float cos_theta = cos(theta);
    float sin2_theta = sin_theta * sin_theta;

    // Γ^r components
    float gamma_r_rr = rs / (2.0 * r * (r - rs));
    float gamma_r_theta_theta = -(r - rs);
    float gamma_r_phi_phi = -(r - rs) * sin2_theta;

    // Γ^θ components
    float gamma_theta_r_theta = 1.0 / r;
    float gamma_theta_phi_phi = -sin_theta * cos_theta;

    // Γ^φ components
    float gamma_phi_r_phi = 1.0 / r;
    float gamma_phi_theta_phi = (abs(sin_theta) > EPSILON) ? (cos_theta / sin_theta) : 0.0;

    // Geodesic equation: d²x^μ/dλ² = -Γ^μ_αβ (dx^α/dλ)(dx^β/dλ)
    float d2r = -gamma_r_rr * dr * dr
                -gamma_r_theta_theta * dtheta * dtheta
                -gamma_r_phi_phi * dphi * dphi;

    float d2theta = -2.0 * gamma_theta_r_theta * dr * dtheta
                    -gamma_theta_phi_phi * dphi * dphi;

    float d2phi = -2.0 * gamma_phi_r_phi * dr * dphi
                  -2.0 * gamma_phi_theta_phi * dtheta * dphi;

    return vec3(d2r, d2theta, d2phi);
}

// RK4 integration step for geodesic
struct RayState {
    vec3 position;  // Spherical coordinates (r, θ, φ)
    vec3 velocity;  // Derivatives (dr/dλ, dθ/dλ, dφ/dλ)
};

RayState rk4Step(RayState state, float h) {
    // k1
    vec3 k1_vel = state.velocity;
    vec3 k1_acc = schwarzschildAcceleration(state.position, state.velocity);

    // k2
    vec3 pos2 = state.position + k1_vel * (h / 2.0);
    vec3 vel2 = state.velocity + k1_acc * (h / 2.0);
    vec3 k2_vel = vel2;
    vec3 k2_acc = schwarzschildAcceleration(pos2, vel2);

    // k3
    vec3 pos3 = state.position + k2_vel * (h / 2.0);
    vec3 vel3 = state.velocity + k2_acc * (h / 2.0);
    vec3 k3_vel = vel3;
    vec3 k3_acc = schwarzschildAcceleration(pos3, vel3);

    // k4
    vec3 pos4 = state.position + k3_vel * h;
    vec3 vel4 = state.velocity + k3_acc * h;
    vec3 k4_vel = vel4;
    vec3 k4_acc = schwarzschildAcceleration(pos4, vel4);

    // Update state
    RayState newState;
    newState.position = state.position + (k1_vel + 2.0*k2_vel + 2.0*k3_vel + k4_vel) * (h / 6.0);
    newState.velocity = state.velocity + (k1_acc + 2.0*k2_acc + 2.0*k3_acc + k4_acc) * (h / 6.0);

    // Clamp radius
    newState.position.x = max(newState.position.x, uEventHorizon * 0.1);

    // Normalize theta to [0, π]
    if (newState.position.y < 0.0) {
        newState.position.y = -newState.position.y;
        newState.position.z += PI;
    }
    if (newState.position.y > PI) {
        newState.position.y = TWO_PI - newState.position.y;
        newState.position.z += PI;
    }

    // Normalize phi to [0, 2π]
    newState.position.z = mod(newState.position.z, TWO_PI);

    return newState;
}

// Simple procedural starfield
vec3 starfieldColor(vec3 direction) {
    // Create a simple star field pattern
    vec3 dir = normalize(direction);

    // Convert to spherical for consistent pattern
    float theta = acos(clamp(dir.z, -1.0, 1.0));
    float phi = atan(dir.y, dir.x);

    // Create star-like points using noise-like function
    float starPattern = fract(sin(dot(vec2(theta * 100.0, phi * 100.0), vec2(12.9898, 78.233))) * 43758.5453);

    if (starPattern > 0.998) {
        // Bright star
        return vec3(1.0, 0.95, 0.9) * (starPattern * 2.0 - 1.996);
    } else if (starPattern > 0.995) {
        // Dim star
        return vec3(0.8, 0.85, 1.0) * (starPattern * 3.0 - 2.985);
    }

    // Gradient background (milky way-like)
    float gradient = abs(sin(theta * 2.0)) * 0.15;
    return vec3(0.05, 0.05, 0.1) + vec3(gradient * 0.3, gradient * 0.4, gradient * 0.6);
}

// Main ray tracing function
vec3 traceRay(vec3 origin, vec3 direction) {
    // Convert to spherical coordinates
    vec3 cartPos = origin;
    vec3 sphPos = cartesianToSpherical(cartPos);

    // Initial velocity in Cartesian, convert to spherical derivatives
    vec3 vel = normalize(direction);

    // Simplified: use small initial velocity magnitude
    float velMag = 1.0;
    vec3 sphVel = vec3(
        dot(vel, normalize(cartPos)),  // radial component
        0.0,  // theta component (simplified)
        0.0   // phi component (simplified)
    ) * velMag;

    RayState state;
    state.position = sphPos;
    state.velocity = sphVel;

    // Integrate geodesic
    for (int step = 0; step < uMaxSteps; ++step) {
        float r = state.position.x;

        // Check if ray hit event horizon
        if (r < uEventHorizon) {
            return vec3(0.0);  // Black
        }

        // Check if ray escaped
        if (r > ESCAPE_RADIUS) {
            // Sample starfield
            vec3 finalDir = sphericalToCartesian(vec3(1.0, state.position.y, state.position.z));
            return starfieldColor(finalDir);
        }

        // TODO: Check accretion disk intersection
        // if (uAccretionDiskEnabled) { ... }

        // Integrate one step
        state = rk4Step(state, uStepSize);
    }

    // Max steps reached - return dark red to indicate
    return vec3(0.1, 0.0, 0.0);
}

void main() {
    // Generate ray direction from screen coordinates
    float tanHalfFov = tan(radians(uFov * 0.5));

    vec3 rayDir = normalize(
        uCameraFront +
        uCameraRight * (vScreenPos.x * uAspect * tanHalfFov) +
        uCameraUp * (vScreenPos.y * tanHalfFov)
    );

    // Trace ray through curved spacetime
    vec3 color = traceRay(uCameraPos, rayDir);

    // Output final color
    FragColor = vec4(color, 1.0);
}
