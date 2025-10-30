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
    float theta = acos(clamp(pos.z / (r + EPSILON), -1.0, 1.0));
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

// Properly convert Cartesian velocity to spherical velocity
vec3 cartesianToSphericalVelocity(vec3 pos, vec3 vel) {
    float r = length(pos);
    if (r < EPSILON) return vec3(0.0);

    float x = pos.x, y = pos.y, z = pos.z;
    float vx = vel.x, vy = vel.y, vz = vel.z;

    float rxy = sqrt(x*x + y*y);

    // dr/dt
    float dr = (x*vx + y*vy + z*vz) / r;

    // dθ/dt
    float dtheta = 0.0;
    if (rxy > EPSILON) {
        dtheta = (z * (x*vx + y*vy) - rxy*rxy * vz) / (r*r * rxy);
    }

    // dφ/dt
    float dphi = 0.0;
    if (rxy > EPSILON) {
        dphi = (x*vy - y*vx) / (rxy * rxy);
    }

    return vec3(dr, dtheta, dphi);
}

// Compute Schwarzschild geodesic acceleration
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

    // Precompute common terms
    float sin_theta = sin(theta);
    float cos_theta = cos(theta);
    float sin2_theta = sin_theta * sin_theta;

    // Christoffel symbols
    float gamma_r_rr = rs / (2.0 * r * (r - rs));
    float gamma_r_theta_theta = -(r - rs);
    float gamma_r_phi_phi = -(r - rs) * sin2_theta;

    float gamma_theta_r_theta = 1.0 / r;
    float gamma_theta_phi_phi = -sin_theta * cos_theta;

    float gamma_phi_r_phi = 1.0 / r;
    float gamma_phi_theta_phi = (abs(sin_theta) > EPSILON) ? (cos_theta / sin_theta) : 0.0;

    // Geodesic equation
    float d2r = -gamma_r_rr * dr * dr
                -gamma_r_theta_theta * dtheta * dtheta
                -gamma_r_phi_phi * dphi * dphi;

    float d2theta = -2.0 * gamma_theta_r_theta * dr * dtheta
                    -gamma_theta_phi_phi * dphi * dphi;

    float d2phi = -2.0 * gamma_phi_r_phi * dr * dphi
                  -2.0 * gamma_phi_theta_phi * dtheta * dphi;

    return vec3(d2r, d2theta, d2phi);
}

// RK4 integration step
struct RayState {
    vec3 position;
    vec3 velocity;
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

    // Update
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

// Improved starfield with better stars
vec3 starfieldColor(vec3 direction) {
    vec3 dir = normalize(direction);

    // Multiple layers of stars with different sizes
    vec3 color = vec3(0.01, 0.01, 0.02); // Dark blue background

    // Large bright stars
    vec2 p1 = vec2(atan(dir.y, dir.x), acos(dir.z));
    float star1 = fract(sin(dot(p1 * 50.0, vec2(12.9898, 78.233))) * 43758.5453);
    if (star1 > 0.998) {
        float brightness = (star1 - 0.998) / 0.002;
        color += vec3(1.0, 0.95, 0.9) * brightness * 3.0;
    }

    // Medium stars
    vec2 p2 = vec2(atan(dir.y, dir.x), acos(dir.z));
    float star2 = fract(sin(dot(p2 * 100.0, vec2(15.9898, 82.233))) * 43758.5453);
    if (star2 > 0.996) {
        float brightness = (star2 - 0.996) / 0.004;
        color += vec3(0.9, 0.9, 1.0) * brightness * 2.0;
    }

    // Small stars
    vec2 p3 = vec2(atan(dir.y, dir.x), acos(dir.z));
    float star3 = fract(sin(dot(p3 * 200.0, vec2(18.9898, 85.233))) * 43758.5453);
    if (star3 > 0.993) {
        float brightness = (star3 - 0.993) / 0.007;
        color += vec3(0.8, 0.8, 0.9) * brightness;
    }

    // Milky way-like gradient
    float gradient = abs(sin(acos(dir.z) * 2.0));
    color += vec3(0.03, 0.04, 0.06) * gradient * 0.5;

    return color;
}

// Blackbody radiation color (simplified Planck's law)
vec3 blackbodyColor(float temperature) {
    // Temperature in Kelvin, simplified color mapping
    // Red dominates at low T, blue at high T

    float t = temperature / 10000.0; // Normalize

    vec3 color;
    if (t < 0.5) {
        // Red to orange
        color = vec3(1.0, t * 1.2, t * 0.3);
    } else {
        // Orange to white to blue
        color = vec3(1.0, 0.8 + t * 0.2, 0.5 + t * 0.5);
    }

    return clamp(color, 0.0, 1.0);
}

// Accretion disk intersection and rendering
vec4 accretionDiskColor(vec3 hitPos, vec3 velocity, float r) {
    if (!uAccretionDiskEnabled) return vec4(0.0);

    // Disk extends from ISCO to outer radius
    float r_inner = uISCO;
    float r_outer = uISCO * 10.0;

    if (r < r_inner || r > r_outer) {
        return vec4(0.0);
    }

    // Temperature profile (Shakura-Sunyaev)
    float T_max = 10000.0; // Kelvin
    float temp = T_max * pow(r_inner / r, 0.75) * pow(max(1.0 - sqrt(r_inner / r), 0.0), 0.25);

    // Blackbody color
    vec3 color = blackbodyColor(temp);

    // Orbital velocity (Keplerian)
    float v_orbital = sqrt(uBlackHoleMass / r);
    float beta = v_orbital; // In geometric units (c=1)

    // Doppler shift (simplified)
    // Approaching side is blue-shifted, receding is red-shifted
    float phi = atan(hitPos.y, hitPos.x);
    float doppler_factor = 1.0 + beta * sin(phi) * 0.5;

    color *= doppler_factor;

    // Relativistic beaming (simplified)
    float gamma = 1.0 / sqrt(1.0 - beta * beta);
    float beaming = pow(doppler_factor, 3.0);

    // Brightness falloff with distance
    float brightness = 1.0 / (r / r_inner);
    brightness *= beaming;

    // Add some turbulence/noise
    float noise = fract(sin(dot(hitPos.xy * 10.0, vec2(12.9898, 78.233))) * 43758.5453);
    brightness *= (0.8 + noise * 0.4);

    color *= brightness;

    // Alpha based on brightness
    float alpha = clamp(brightness * 0.5, 0.0, 1.0);

    return vec4(color, alpha);
}

// Check disk intersection
bool intersectDisk(vec3 pos, vec3 dir, out float t, out vec3 hitPos) {
    // Disk is in equatorial plane (z=0 initially, but rotates with coordinates)
    // In spherical coords, disk is at theta = PI/2

    // Simple intersection with z=0 plane in Cartesian
    if (abs(dir.z) < EPSILON) return false;

    t = -pos.z / dir.z;
    if (t < 0.0) return false;

    hitPos = pos + dir * t;
    float r = length(hitPos.xy);

    return (r > uISCO && r < uISCO * 10.0);
}

// Main ray tracing function
vec3 traceRay(vec3 origin, vec3 direction) {
    // Convert to spherical coordinates
    vec3 sphPos = cartesianToSpherical(origin);

    // Convert velocity properly
    vec3 sphVel = cartesianToSphericalVelocity(origin, direction);

    RayState state;
    state.position = sphPos;
    state.velocity = sphVel;

    vec3 accumulated_color = vec3(0.0);
    float accumulated_alpha = 0.0;

    vec3 lastCartPos = origin;

    // Integrate geodesic
    for (int step = 0; step < uMaxSteps; ++step) {
        float r = state.position.x;

        // Convert current position to Cartesian for disk check
        vec3 cartPos = sphericalToCartesian(state.position);

        // Check if ray hit event horizon
        if (r < uEventHorizon) {
            return mix(accumulated_color, vec3(0.0), 1.0 - accumulated_alpha);
        }

        // Check if ray escaped
        if (r > ESCAPE_RADIUS) {
            vec3 finalDir = sphericalToCartesian(vec3(1.0, state.position.y, state.position.z));
            vec3 skyColor = starfieldColor(finalDir);
            return mix(skyColor, accumulated_color, accumulated_alpha);
        }

        // Check accretion disk intersection (in Cartesian for simplicity)
        if (uAccretionDiskEnabled && accumulated_alpha < 0.99) {
            // Check if we crossed the equatorial plane
            if (lastCartPos.z * cartPos.z < 0.0) {
                float diskR = length(cartPos.xy);
                if (diskR > uISCO && diskR < uISCO * 10.0) {
                    vec3 diskVel = normalize(cartPos - lastCartPos);
                    vec4 diskColor = accretionDiskColor(cartPos, diskVel, diskR);

                    // Alpha blend
                    accumulated_color = mix(accumulated_color, diskColor.rgb, diskColor.a * (1.0 - accumulated_alpha));
                    accumulated_alpha += diskColor.a * (1.0 - accumulated_alpha);
                }
            }
        }

        lastCartPos = cartPos;

        // Integrate one step
        state = rk4Step(state, uStepSize);
    }

    // Max steps reached
    return mix(vec3(0.1, 0.0, 0.0), accumulated_color, accumulated_alpha);
}

void main() {
    // Generate ray direction
    float tanHalfFov = tan(radians(uFov * 0.5));

    vec3 rayDir = normalize(
        uCameraFront +
        uCameraRight * (vScreenPos.x * uAspect * tanHalfFov) +
        uCameraUp * (vScreenPos.y * tanHalfFov)
    );

    // Trace ray
    vec3 color = traceRay(uCameraPos, rayDir);

    // Tone mapping and gamma correction
    color = color / (color + vec3(1.0)); // Reinhard tone mapping
    color = pow(color, vec3(1.0/2.2)); // Gamma correction

    FragColor = vec4(color, 1.0);
}
