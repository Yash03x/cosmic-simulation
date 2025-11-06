#version 410 core

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform bool uHorizontal;      // True for horizontal pass, false for vertical
uniform float uBloomSpread;    // Controls bloom size

// 9-tap Gaussian blur weights
// σ = 2.0 gives nice smooth bloom
const float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {
    vec2 texelSize = 1.0 / textureSize(uTexture, 0);
    vec3 result = texture(uTexture, vTexCoord).rgb * weights[0];

    // Sample along either horizontal or vertical axis
    if (uHorizontal) {
        for (int i = 1; i < 5; ++i) {
            float offset = float(i) * uBloomSpread;
            result += texture(uTexture, vTexCoord + vec2(texelSize.x * offset, 0.0)).rgb * weights[i];
            result += texture(uTexture, vTexCoord - vec2(texelSize.x * offset, 0.0)).rgb * weights[i];
        }
    } else {
        for (int i = 1; i < 5; ++i) {
            float offset = float(i) * uBloomSpread;
            result += texture(uTexture, vTexCoord + vec2(0.0, texelSize.y * offset)).rgb * weights[i];
            result += texture(uTexture, vTexCoord - vec2(0.0, texelSize.y * offset)).rgb * weights[i];
        }
    }

    FragColor = vec4(result, 1.0);
}
