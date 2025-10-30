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

#define PI 3.14159265359
#define ESCAPE_RADIUS 50.0

// Beautiful multi-layer starfield
vec3 starfield(vec3 dir) {
    vec3 d = normalize(dir);
    vec3 color = vec3(0.01, 0.01, 0.03); // Dark blue background

    // Large bright stars
    vec2 p1 = vec2(atan(d.y, d.x), acos(d.z)) * 30.0;
    float s1 = fract(sin(dot(p1, vec2(12.9898, 78.233))) * 43758.5453);
    if (s1 > 0.998) {
        float b = (s1 - 0.998) / 0.002;
        color += vec3(1.0, 0.95, 0.85) * b * 4.0;
    }

    // Medium stars
    vec2 p2 = vec2(atan(d.y, d.x), acos(d.z)) * 80.0;
    float s2 = fract(sin(dot(p2, vec2(15.234, 82.123))) * 43758.5453);
    if (s2 > 0.996) {
        float b = (s2 - 0.996) / 0.004;
        color += vec3(0.9, 0.9, 1.0) * b * 2.0;
    }

    // Small stars (dust)
    vec2 p3 = vec2(atan(d.y, d.x), acos(d.z)) * 150.0;
    float s3 = fract(sin(dot(p3, vec2(18.456, 85.789))) * 43758.5453);
    if (s3 > 0.994) {
        float b = (s3 - 0.994) / 0.006;
        color += vec3(0.7, 0.8, 0.9) * b * 0.8;
    }

    // Milky way gradient
    float gradient = abs(sin(acos(d.z) * 2.0));
    color += vec3(0.03, 0.04, 0.06) * gradient * 0.4;

    return color;
}

// Simple but effective ray marching with gravitational deflection
vec3 traceRay(vec3 origin, vec3 dir) {
    vec3 pos = origin;
    vec3 rayDir = normalize(dir);

    float dt = 0.2; // Step size

    for (int i = 0; i < uMaxSteps; i++) {
        float r = length(pos);

        // Hit event horizon - return black
        if (r < uEventHorizon) {
            return vec3(0.0);
        }

        // Escaped - show stars
        if (r > ESCAPE_RADIUS) {
            return starfield(rayDir);
        }

        // Gravitational deflection (simplified but effective)
        vec3 toCenter = -normalize(pos);
        float dist = length(pos);

        // Schwarzschild-like deflection
        float rs = uEventHorizon;
        float deflection = (rs * rs * rs) / (dist * dist * dist);
        deflection = clamp(deflection, 0.0, 0.3);

        // Bend ray toward black hole
        rayDir = normalize(rayDir + toCenter * deflection * dt);

        // Step forward
        pos += rayDir * dt;
    }

    // Max iterations - return dim red
    return vec3(0.05, 0.0, 0.0);
}

void main() {
    // Generate ray direction
    float tanHalfFov = tan(radians(uFov * 0.5));

    vec3 rayDir = normalize(
        uCameraFront +
        uCameraRight * (vScreenPos.x * uAspect * tanHalfFov) +
        uCameraUp * (vScreenPos.y * tanHalfFov)
    );

    // Trace ray with gravitational lensing
    vec3 color = traceRay(uCameraPos, rayDir);

    // Gamma correction
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}
