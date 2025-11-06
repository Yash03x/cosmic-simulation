#version 410 core

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uSceneTexture;
uniform float uThreshold;      // Brightness threshold for bloom
uniform float uSoftThreshold;  // Soft knee for smooth transition

// Extract bright regions for bloom effect
void main() {
    vec3 color = texture(uSceneTexture, vTexCoord).rgb;

    // Calculate luminance (perceived brightness)
    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));

    // Soft threshold curve
    // This creates a smooth transition rather than hard cutoff
    float knee = uThreshold * uSoftThreshold;
    float soft = luminance - uThreshold + knee;
    soft = clamp(soft, 0.0, 2.0 * knee);
    soft = soft * soft / (4.0 * knee + 0.00001);

    float contribution = max(soft, luminance - uThreshold) / max(luminance, 0.00001);

    FragColor = vec4(color * contribution, 1.0);
}
