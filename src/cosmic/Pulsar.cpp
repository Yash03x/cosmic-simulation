#include "cosmic/Pulsar.hpp"
#include <glm/gtc/constants.hpp>
#include <cmath>

namespace cosmic {

// ==================== PulsarPresets ====================

PulsarBeamProperties PulsarPresets::getDefaultBeamProperties() {
    PulsarBeamProperties props;
    props.beamWidth = 15.0f;  // degrees
    props.beamLength = 150.0f;  // M
    props.beamColor = glm::vec3(0.3f, 0.8f, 1.0f);  // Cyan (radio emission)
    props.intensity = 1.0f;
    props.pulseFrequency = 1.0f;  // 1 Hz
    props.pattern = PulsarBeamProperties::Pattern::Cone;
    return props;
}

PulsarBeamProperties PulsarPresets::getCrabBeamProperties() {
    PulsarBeamProperties props;
    props.beamWidth = 20.0f;
    props.beamLength = 200.0f;
    props.beamColor = glm::vec3(0.8f, 0.9f, 1.0f);  // Bright blue-white
    props.intensity = 1.5f;
    props.pulseFrequency = 30.0f;  // 30 Hz
    props.pattern = PulsarBeamProperties::Pattern::Cone;
    return props;
}

PulsarBeamProperties PulsarPresets::getVelaBeamProperties() {
    PulsarBeamProperties props;
    props.beamWidth = 18.0f;
    props.beamLength = 180.0f;
    props.beamColor = glm::vec3(0.5f, 0.9f, 0.8f);  // Greenish-cyan
    props.intensity = 1.2f;
    props.pulseFrequency = 11.2f;  // 11.2 Hz
    props.pattern = PulsarBeamProperties::Pattern::Cone;
    return props;
}

PulsarBeamProperties PulsarPresets::getMillisecondBeamProperties() {
    PulsarBeamProperties props;
    props.beamWidth = 10.0f;  // Narrow beams
    props.beamLength = 120.0f;
    props.beamColor = glm::vec3(0.6f, 0.7f, 1.0f);  // Light blue
    props.intensity = 0.8f;
    props.pulseFrequency = 333.0f;  // 333 Hz (3ms period)
    props.pattern = PulsarBeamProperties::Pattern::PencilBeam;
    return props;
}

PulsarBeamProperties PulsarPresets::getMagnetarBeamProperties() {
    PulsarBeamProperties props;
    props.beamWidth = 25.0f;  // Wide beams
    props.beamLength = 250.0f;
    props.beamColor = glm::vec3(1.0f, 0.5f, 0.3f);  // Orange-red (X-rays!)
    props.intensity = 2.0f;  // Very bright
    props.pulseFrequency = 0.13f;  // Slow rotation
    props.pattern = PulsarBeamProperties::Pattern::FanBeam;
    return props;
}

// ==================== Pulsar ====================

Pulsar::Pulsar(const NeutronStarProperties& nsProps)
    : neutronStar_(nsProps),
      beamsVisible_(true) {

    // Set beam properties based on neutron star type
    if (nsProps.name.find("Crab") != std::string::npos) {
        beamProps_ = PulsarPresets::getCrabBeamProperties();
    } else if (nsProps.name.find("Vela") != std::string::npos) {
        beamProps_ = PulsarPresets::getVelaBeamProperties();
    } else if (nsProps.name.find("Millisecond") != std::string::npos) {
        beamProps_ = PulsarPresets::getMillisecondBeamProperties();
    } else if (nsProps.name.find("Magnetar") != std::string::npos) {
        beamProps_ = PulsarPresets::getMagnetarBeamProperties();
    } else {
        beamProps_ = PulsarPresets::getDefaultBeamProperties();
    }

    // Sync pulse frequency with rotation
    beamProps_.pulseFrequency = 1.0f / nsProps.rotationPeriod;
}

void Pulsar::update(float deltaTime) {
    // Update underlying neutron star (handles rotation)
    neutronStar_.update(deltaTime);
}

std::vector<BeamSegment> Pulsar::getBeamSegments() const {
    std::vector<BeamSegment> segments;

    if (!beamsVisible_ || !neutronStar_.isVisible()) {
        return segments;
    }

    generateBeamSegments(segments);
    return segments;
}

void Pulsar::generateBeamSegments(std::vector<BeamSegment>& segments) const {
    const glm::vec3 nsPos = neutronStar_.getPosition();
    const float beamLength = beamProps_.beamLength;
    const float beamWidth = glm::radians(beamProps_.beamWidth);

    // Get beam directions from both magnetic poles
    glm::vec3 northBeamDir = getBeamDirection(true);
    glm::vec3 southBeamDir = getBeamDirection(false);

    // Number of segments along beam
    const int numSegments = 20;
    const float segmentLength = beamLength / numSegments;

    // Generate north pole beam
    for (int i = 0; i < numSegments; i++) {
        float t1 = static_cast<float>(i) / numSegments;
        float t2 = static_cast<float>(i + 1) / numSegments;

        BeamSegment seg;
        seg.startPos = nsPos + northBeamDir * (t1 * beamLength);
        seg.endPos = nsPos + northBeamDir * (t2 * beamLength);

        // Fade intensity along beam
        float fadeStart = 1.0f - (t1 * 0.7f);
        float fadeEnd = 1.0f - (t2 * 0.7f);
        seg.intensity = (fadeStart + fadeEnd) * 0.5f * beamProps_.intensity;
        seg.color = beamProps_.beamColor;

        segments.push_back(seg);
    }

    // Generate south pole beam
    for (int i = 0; i < numSegments; i++) {
        float t1 = static_cast<float>(i) / numSegments;
        float t2 = static_cast<float>(i + 1) / numSegments;

        BeamSegment seg;
        seg.startPos = nsPos + southBeamDir * (t1 * beamLength);
        seg.endPos = nsPos + southBeamDir * (t2 * beamLength);

        float fadeStart = 1.0f - (t1 * 0.7f);
        float fadeEnd = 1.0f - (t2 * 0.7f);
        seg.intensity = (fadeStart + fadeEnd) * 0.5f * beamProps_.intensity;
        seg.color = beamProps_.beamColor;

        segments.push_back(seg);
    }

    // Add cone pattern segments if applicable
    if (beamProps_.pattern == PulsarBeamProperties::Pattern::Cone) {
        const int numConeSegments = 8;
        for (int j = 0; j < numConeSegments; j++) {
            float angle = (2.0f * glm::pi<float>() * j) / numConeSegments;

            // Perpendicular vector to beam direction
            glm::vec3 perp = glm::normalize(glm::cross(northBeamDir, glm::vec3(0.0f, 1.0f, 0.0f)));
            if (glm::length(perp) < 0.1f) {
                perp = glm::normalize(glm::cross(northBeamDir, glm::vec3(1.0f, 0.0f, 0.0f)));
            }

            glm::vec3 perp2 = glm::normalize(glm::cross(northBeamDir, perp));

            glm::vec3 offset = (perp * std::cos(angle) + perp2 * std::sin(angle)) * std::tan(beamWidth);

            for (int i = 5; i < numSegments; i++) {  // Start partway along beam
                float t1 = static_cast<float>(i) / numSegments;
                float t2 = static_cast<float>(i + 1) / numSegments;

                BeamSegment seg;
                seg.startPos = nsPos + northBeamDir * (t1 * beamLength) + offset * (t1 * beamLength);
                seg.endPos = nsPos + northBeamDir * (t2 * beamLength) + offset * (t2 * beamLength);
                seg.intensity = 0.3f * beamProps_.intensity * (1.0f - t1 * 0.5f);
                seg.color = beamProps_.beamColor;

                segments.push_back(seg);
            }
        }
    }
}

glm::vec3 Pulsar::getBeamDirection(bool northPole) const {
    // Get current magnetic axis (rotates with neutron star)
    glm::vec3 magneticAxis = neutronStar_.getMagneticAxis();

    // Current rotation
    float rotationAngle = neutronStar_.getRotationAngle();
    glm::vec3 rotationAxis = neutronStar_.getRotationAxis();

    // Rotate magnetic axis by current rotation angle
    float c = std::cos(rotationAngle);
    float s = std::sin(rotationAngle);

    // Rodrigues' rotation formula
    glm::vec3 k = rotationAxis;
    glm::vec3 v = magneticAxis;
    glm::vec3 rotated = v * c + glm::cross(k, v) * s + k * glm::dot(k, v) * (1.0f - c);

    // Normalize
    rotated = glm::normalize(rotated);

    // Return direction (north or south pole)
    return northPole ? rotated : -rotated;
}

float Pulsar::getRotationPhase() const {
    float angle = neutronStar_.getRotationAngle();
    return angle / (2.0f * glm::pi<float>());
}

bool Pulsar::isBeamPointingAtObserver(const glm::vec3& observerPos) const {
    glm::vec3 toObserver = glm::normalize(observerPos - neutronStar_.getPosition());

    // Check if either beam is pointing near observer
    glm::vec3 northBeam = getBeamDirection(true);
    glm::vec3 southBeam = getBeamDirection(false);

    float northDot = glm::dot(northBeam, toObserver);
    float southDot = glm::dot(southBeam, toObserver);

    float beamWidthCos = std::cos(glm::radians(beamProps_.beamWidth));

    return (northDot > beamWidthCos) || (southDot > beamWidthCos);
}

float Pulsar::getPulseIntensity() const {
    if (!beamsVisible_) {
        return 0.0f;
    }

    // Simplified pulse profile: bright when beam points at us
    // Use observer at camera position (would need to pass this in)
    // For now, use a simple periodic function

    float phase = getRotationPhase();

    // Double pulse per rotation (two poles)
    float pulse1 = std::exp(-50.0f * std::pow(phase - 0.25f, 2.0f));
    float pulse2 = std::exp(-50.0f * std::pow(phase - 0.75f, 2.0f));

    return (pulse1 + pulse2) * beamProps_.intensity;
}

} // namespace cosmic
