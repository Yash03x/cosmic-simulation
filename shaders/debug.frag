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

void main() {
    // Generate ray direction from screen coordinates
    float tanHalfFov = tan(radians(uFov * 0.5));

    vec3 rayDir = normalize(
        uCameraFront +
        uCameraRight * (vScreenPos.x * uAspect * tanHalfFov) +
        uCameraUp * (vScreenPos.y * tanHalfFov)
    );

    // Simple distance check - no physics, just geometry
    vec3 origin = uCameraPos;

    // Cast ray and find closest approach to origin
    float t = dot(-origin, rayDir);
    vec3 closestPoint = origin + rayDir * max(t, 0.0);
    float distToBlackHole = length(closestPoint);

    vec3 color;

    if (distToBlackHole < uEventHorizon) {
        // Should hit black hole - render black
        color = vec3(0.0);
    } else if (distToBlackHole < 5.0) {
        // Near black hole - render yellow/orange
        color = vec3(1.0, 0.7, 0.0);
    } else if (distToBlackHole < 10.0) {
        // Medium distance - render blue
        color = vec3(0.0, 0.5, 1.0);
    } else {
        // Far away - render stars
        // Simple procedural stars
        vec2 p = vec2(atan(rayDir.y, rayDir.x), acos(rayDir.z));
        float star = fract(sin(dot(p * 50.0, vec2(12.9898, 78.233))) * 43758.5453);

        if (star > 0.998) {
            color = vec3(1.0, 1.0, 1.0) * ((star - 0.998) / 0.002);
        } else {
            // Dark blue background
            color = vec3(0.01, 0.01, 0.05);
        }
    }

    FragColor = vec4(color, 1.0);
}
