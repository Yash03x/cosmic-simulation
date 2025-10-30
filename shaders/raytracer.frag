#version 410 core

in vec2 vTexCoord;
in vec2 vScreenPos;

out vec4 FragColor;

void main() {
    // Simple gradient test - should show red->green gradient
    vec3 color = vec3(
        (vScreenPos.x + 1.0) * 0.5,  // Red: 0 to 1 from left to right
        (vScreenPos.y + 1.0) * 0.5,  // Green: 0 to 1 from bottom to top
        0.3                           // Blue: constant
    );

    FragColor = vec4(color, 1.0);
}
