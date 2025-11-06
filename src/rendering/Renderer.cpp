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
      sceneFBO_(0),
      sceneTexture_(0),
      sceneDepth_(0),
      brightFBO_(0),
      brightTexture_(0),
      maxRaySteps_(1000),
      stepSize_(0.1f),
      accretionDiskEnabled_(false),
      jetsEnabled_(false),         // Jets disabled by default (enable with UI)
      ergosphereVisible_(false),   // Ergosphere hidden by default (toggle in UI)
      backgroundColor_(0.0f, 0.0f, 0.0f),
      bloomEnabled_(true),         // Enable bloom by default
      bloomIntensity_(0.6f),       // Moderate bloom
      bloomThreshold_(1.0f),       // Brightness threshold
      bloomSpread_(1.0f),          // Blur spread
      exposure_(1.0f),             // Default exposure
      gamma_(2.2f),                // Standard gamma
      accretionRate_(0.1f),        // Moderate accretion rate
      alphaViscosity_(0.1f),       // Standard Shakura-Sunyaev alpha
      diskInclination_(1.0f),      // ~60 degrees viewing angle
      scaleHeightRatio_(0.05f),    // Thin disk: H/r ~ 0.05
      metricType_(0),
      spin_(0.0f) {
    pingpongFBO_[0] = pingpongFBO_[1] = 0;
    pingpongTextures_[0] = pingpongTextures_[1] = 0;
}

Renderer::~Renderer() {
    cleanup();
}

bool Renderer::initialize() {
    // Load main ray tracer shader
    if (!rayTracerShader_.load("shaders/raytracer.vert", "shaders/raytracer.frag")) {
        std::cerr << "ERROR::RENDERER::RAYTRACER_SHADER_LOAD_FAILED\n";
        return false;
    }

    // Load post-processing shaders
    if (!brightPassShader_.load("shaders/post_process.vert", "shaders/bright_pass.frag")) {
        std::cerr << "ERROR::RENDERER::BRIGHT_PASS_SHADER_LOAD_FAILED\n";
        return false;
    }

    if (!blurShader_.load("shaders/post_process.vert", "shaders/gaussian_blur.frag")) {
        std::cerr << "ERROR::RENDERER::BLUR_SHADER_LOAD_FAILED\n";
        return false;
    }

    if (!compositeShader_.load("shaders/post_process.vert", "shaders/composite.frag")) {
        std::cerr << "ERROR::RENDERER::COMPOSITE_SHADER_LOAD_FAILED\n";
        return false;
    }

    // Setup fullscreen quad
    setupQuad();

    // Setup framebuffers for post-processing
    setupFramebuffers();

    // Set initial OpenGL state
    glClearColor(backgroundColor_.r, backgroundColor_.g, backgroundColor_.b, 1.0f);

    std::cout << "Renderer initialized successfully\n";
    std::cout << "  Resolution: " << width_ << "x" << height_ << "\n";
    std::cout << "  Max ray steps: " << maxRaySteps_ << "\n";
    std::cout << "  Step size: " << stepSize_ << "\n";
    std::cout << "  Bloom enabled: " << (bloomEnabled_ ? "Yes" : "No") << "\n";

    return true;
}

void Renderer::render(const Camera& camera,
                      const physics::Metric* metric,
                      float simulationTime,
                      float deltaTime) {
    if (!metric) {
        std::cerr << "ERROR::RENDERER::NULL_METRIC\n";
        return;
    }

    // ===== PASS 1: Render scene to texture =====
    if (bloomEnabled_) {
        glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO_);
    }

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
    rayTracerShader_.setFloat("uTime", simulationTime);
    rayTracerShader_.setFloat("uDeltaTime", deltaTime);
    rayTracerShader_.setInt("uMetricType", metricType_);
    rayTracerShader_.setFloat("uSpin", spin_);

    // Black hole parameters
    rayTracerShader_.setFloat("uBlackHoleMass", static_cast<float>(metric->getMass()));
    rayTracerShader_.setFloat("uEventHorizon", static_cast<float>(metric->eventHorizonRadius()));
    rayTracerShader_.setFloat("uPhotonSphere", static_cast<float>(metric->photonSphereRadius()));
    rayTracerShader_.setFloat("uISCO", static_cast<float>(metric->iscoRadius()));

    // Ray tracing parameters
    rayTracerShader_.setInt("uMaxSteps", maxRaySteps_);
    rayTracerShader_.setFloat("uStepSize", stepSize_);
    rayTracerShader_.setBool("uAccretionDiskEnabled", accretionDiskEnabled_);
    rayTracerShader_.setBool("uJetsEnabled", jetsEnabled_);
    rayTracerShader_.setBool("uErgosphereVisible", ergosphereVisible_);

    // Accretion disk physics parameters
    rayTracerShader_.setFloat("uAccretionRate", accretionRate_);
    rayTracerShader_.setFloat("uAlphaViscosity", alphaViscosity_);
    rayTracerShader_.setFloat("uDiskInclination", diskInclination_);
    rayTracerShader_.setFloat("uScaleHeightRatio", scaleHeightRatio_);

    // Render fullscreen quad
    glBindVertexArray(quadVAO_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    if (!bloomEnabled_) {
        return; // No post-processing
    }

    // ===== PASS 2-N: Apply bloom post-processing =====
    applyBloom();
}

void Renderer::setResolution(int width, int height) {
    width_ = width;
    height_ = height;
    glViewport(0, 0, width, height);
    resizeFramebuffers();
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

void Renderer::setupFramebuffers() {
    // Create scene framebuffer
    glGenFramebuffers(1, &sceneFBO_);
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO_);

    // Scene color texture (HDR format for bloom)
    glGenTextures(1, &sceneTexture_);
    glBindTexture(GL_TEXTURE_2D, sceneTexture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width_, height_, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneTexture_, 0);

    // Scene depth renderbuffer
    glGenRenderbuffers(1, &sceneDepth_);
    glBindRenderbuffer(GL_RENDERBUFFER, sceneDepth_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width_, height_);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, sceneDepth_);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR::FRAMEBUFFER::Scene framebuffer not complete!\n";
    }

    // Create bright pass framebuffer
    glGenFramebuffers(1, &brightFBO_);
    glBindFramebuffer(GL_FRAMEBUFFER, brightFBO_);

    glGenTextures(1, &brightTexture_);
    glBindTexture(GL_TEXTURE_2D, brightTexture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width_, height_, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brightTexture_, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR::FRAMEBUFFER::Bright pass framebuffer not complete!\n";
    }

    // Create ping-pong framebuffers for blur
    glGenFramebuffers(2, pingpongFBO_);
    glGenTextures(2, pingpongTextures_);

    for (int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO_[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongTextures_[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width_, height_, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongTextures_[i], 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "ERROR::FRAMEBUFFER::Ping-pong framebuffer " << i << " not complete!\n";
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    std::cout << "Framebuffers setup complete\n";
}

void Renderer::resizeFramebuffers() {
    if (sceneTexture_ == 0) return; // Not initialized yet

    // Resize scene texture
    glBindTexture(GL_TEXTURE_2D, sceneTexture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width_, height_, 0, GL_RGBA, GL_FLOAT, nullptr);

    // Resize scene depth
    glBindRenderbuffer(GL_RENDERBUFFER, sceneDepth_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width_, height_);

    // Resize bright pass texture
    glBindTexture(GL_TEXTURE_2D, brightTexture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width_, height_, 0, GL_RGBA, GL_FLOAT, nullptr);

    // Resize ping-pong textures
    for (int i = 0; i < 2; i++) {
        glBindTexture(GL_TEXTURE_2D, pingpongTextures_[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width_, height_, 0, GL_RGBA, GL_FLOAT, nullptr);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::applyBloom() {
    glBindVertexArray(quadVAO_);

    // ===== PASS 2: Extract bright regions =====
    glBindFramebuffer(GL_FRAMEBUFFER, brightFBO_);
    glClear(GL_COLOR_BUFFER_BIT);

    brightPassShader_.use();
    brightPassShader_.setInt("uSceneTexture", 0);
    brightPassShader_.setFloat("uThreshold", bloomThreshold_);
    brightPassShader_.setFloat("uSoftThreshold", 0.5f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneTexture_);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    // ===== PASS 3-N: Gaussian blur (ping-pong) =====
    bool horizontal = true;
    bool firstIteration = true;
    int blurPasses = 10; // Number of blur iterations

    blurShader_.use();
    blurShader_.setFloat("uBloomSpread", bloomSpread_);

    for (int i = 0; i < blurPasses; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO_[horizontal]);
        blurShader_.setBool("uHorizontal", horizontal);

        // First iteration uses bright pass texture, rest use ping-pong
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, firstIteration ? brightTexture_ : pingpongTextures_[!horizontal]);
        blurShader_.setInt("uTexture", 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        horizontal = !horizontal;
        if (firstIteration) firstIteration = false;
    }

    // ===== FINAL PASS: Composite scene + bloom =====
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Back to default framebuffer
    glClear(GL_COLOR_BUFFER_BIT);

    compositeShader_.use();
    compositeShader_.setInt("uSceneTexture", 0);
    compositeShader_.setInt("uBloomTexture", 1);
    compositeShader_.setFloat("uBloomIntensity", bloomIntensity_);
    compositeShader_.setFloat("uExposure", exposure_);
    compositeShader_.setFloat("uGamma", gamma_);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneTexture_);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pingpongTextures_[!horizontal]); // Final blurred texture

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
}

void Renderer::cleanup() {
    // Cleanup framebuffers
    if (sceneFBO_ != 0) {
        glDeleteFramebuffers(1, &sceneFBO_);
        sceneFBO_ = 0;
    }
    if (sceneTexture_ != 0) {
        glDeleteTextures(1, &sceneTexture_);
        sceneTexture_ = 0;
    }
    if (sceneDepth_ != 0) {
        glDeleteRenderbuffers(1, &sceneDepth_);
        sceneDepth_ = 0;
    }
    if (brightFBO_ != 0) {
        glDeleteFramebuffers(1, &brightFBO_);
        brightFBO_ = 0;
    }
    if (brightTexture_ != 0) {
        glDeleteTextures(1, &brightTexture_);
        brightTexture_ = 0;
    }
    if (pingpongFBO_[0] != 0) {
        glDeleteFramebuffers(2, pingpongFBO_);
        pingpongFBO_[0] = pingpongFBO_[1] = 0;
    }
    if (pingpongTextures_[0] != 0) {
        glDeleteTextures(2, pingpongTextures_);
        pingpongTextures_[0] = pingpongTextures_[1] = 0;
    }

    // Cleanup quad
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
