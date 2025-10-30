#include "rendering/Renderer.hpp"
#include <glad/glad.h>
#include <iostream>

namespace cosmic {
namespace rendering {

Renderer::Renderer(int width, int height)
    : width_(width),
      height_(height),
      quadVAO_(0),
      quadVBO_(0),
      maxRaySteps_(1000),
      stepSize_(0.1f),
      accretionDiskEnabled_(false),
      backgroundColor_(0.0f, 0.0f, 0.0f),
      accretionRate_(0.1f),        // Moderate accretion rate
      alphaViscosity_(0.1f),       // Standard Shakura-Sunyaev alpha
      diskInclination_(1.0f),      // ~60 degrees viewing angle
      scaleHeightRatio_(0.05f) {   // Thin disk: H/r ~ 0.05
}

Renderer::~Renderer() {
    cleanup();
}

bool Renderer::initialize() {
    // Load shaders
    if (!rayTracerShader_.load("shaders/raytracer.vert", "shaders/raytracer.frag")) {
        std::cerr << "ERROR::RENDERER::SHADER_LOAD_FAILED\n";
        return false;
    }

    // Setup fullscreen quad
    setupQuad();

    // Set initial OpenGL state
    glClearColor(backgroundColor_.r, backgroundColor_.g, backgroundColor_.b, 1.0f);

    std::cout << "Renderer initialized successfully\n";
    std::cout << "  Resolution: " << width_ << "x" << height_ << "\n";
    std::cout << "  Max ray steps: " << maxRaySteps_ << "\n";
    std::cout << "  Step size: " << stepSize_ << "\n";

    return true;
}

void Renderer::render(const Camera& camera, const physics::Metric* metric) {
    if (!metric) {
        std::cerr << "ERROR::RENDERER::NULL_METRIC\n";
        return;
    }

    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use ray tracing shader
    rayTracerShader_.use();

    // Set uniforms
    rayTracerShader_.setVec3("uCameraPos", camera.getPosition());
    rayTracerShader_.setVec3("uCameraFront", camera.getFront());
    rayTracerShader_.setVec3("uCameraUp", camera.getUp());
    rayTracerShader_.setVec3("uCameraRight", camera.getRight());
    rayTracerShader_.setFloat("uFov", camera.getFov());
    rayTracerShader_.setFloat("uAspect", static_cast<float>(width_) / static_cast<float>(height_));

    // Black hole parameters
    rayTracerShader_.setFloat("uBlackHoleMass", static_cast<float>(metric->getMass()));
    rayTracerShader_.setFloat("uEventHorizon", static_cast<float>(metric->eventHorizonRadius()));
    rayTracerShader_.setFloat("uPhotonSphere", static_cast<float>(metric->photonSphereRadius()));
    rayTracerShader_.setFloat("uISCO", static_cast<float>(metric->iscoRadius()));

    // Ray tracing parameters
    rayTracerShader_.setInt("uMaxSteps", maxRaySteps_);
    rayTracerShader_.setFloat("uStepSize", stepSize_);
    rayTracerShader_.setBool("uAccretionDiskEnabled", accretionDiskEnabled_);

    // Accretion disk physics parameters
    rayTracerShader_.setFloat("uAccretionRate", accretionRate_);
    rayTracerShader_.setFloat("uAlphaViscosity", alphaViscosity_);
    rayTracerShader_.setFloat("uDiskInclination", diskInclination_);
    rayTracerShader_.setFloat("uScaleHeightRatio", scaleHeightRatio_);

    // Render fullscreen quad
    glBindVertexArray(quadVAO_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void Renderer::setResolution(int width, int height) {
    width_ = width;
    height_ = height;
    glViewport(0, 0, width, height);
}

void Renderer::setBackgroundColor(float r, float g, float b) {
    backgroundColor_ = glm::vec3(r, g, b);
    glClearColor(r, g, b, 1.0f);
}

void Renderer::setupQuad() {
    // Fullscreen quad vertices (NDC coordinates)
    // Two triangles forming a quad covering the screen
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    // Generate and bind VAO/VBO
    glGenVertexArrays(1, &quadVAO_);
    glGenBuffers(1, &quadVBO_);

    glBindVertexArray(quadVAO_);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // Texture coordinate attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);

    std::cout << "Fullscreen quad setup complete\n";
}

void Renderer::cleanup() {
    if (quadVAO_ != 0) {
        glDeleteVertexArrays(1, &quadVAO_);
        quadVAO_ = 0;
    }
    if (quadVBO_ != 0) {
        glDeleteBuffers(1, &quadVBO_);
        quadVBO_ = 0;
    }
}

} // namespace rendering
} // namespace cosmic
