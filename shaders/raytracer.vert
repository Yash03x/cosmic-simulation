#version 410 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 vTexCoord;
out vec2 vScreenPos;

void main() {
    vTexCoord = aTexCoord;
    vScreenPos = aPos;  // NDC coordinates [-1, 1]
    gl_Position = vec4(aPos, 0.0, 1.0);
}
