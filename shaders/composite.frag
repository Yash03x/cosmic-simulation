#version 410 core

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uSceneTexture;      // Original scene
uniform sampler2D uBloomTexture;      // Blurred bright regions
uniform float uBloomIntensity;        // Bloom strength (0-1+)
uniform float uExposure;              // HDR exposure
uniform float uGamma;                 // Gamma correction

// ACES tone mapping (film-like response)
vec3 acesToneMapping(vec3 color) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

// Reinhard tone mapping (simpler, preserves hue better)
vec3 reinhardToneMapping(vec3 color) {
    return color / (color + vec3(1.0));
}

void main() {
    // Sample scene and bloom
    vec3 sceneColor = texture(uSceneTexture, vTexCoord).rgb;
    vec3 bloomColor = texture(uBloomTexture, vTexCoord).rgb;

    // Combine scene with bloom
    vec3 color = sceneColor + bloomColor * uBloomIntensity;

    // Apply exposure
    color *= uExposure;

    // Tone mapping (use ACES for cinematic look)
    color = acesToneMapping(color);

    // Gamma correction
    color = pow(color, vec3(1.0 / uGamma));

    // Subtle vignette for focus
    float vignette = 1.0 - length(vTexCoord - 0.5) * 0.4;
    vignette = smoothstep(0.4, 1.0, vignette);
    color *= vignette;

    FragColor = vec4(color, 1.0);
}
