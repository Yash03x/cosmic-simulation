#include "rendering/MagneticField.hpp"
#include "physics/Metric.hpp"
#include <glm/gtc/constants.hpp>
#include <cmath>

namespace cosmic {
namespace rendering {

MagneticField::MagneticField(const physics::Metric* metric, double fieldStrength)
    : metric_(metric),
      fieldStrength_(fieldStrength),
      visible_(false),
      magneticAxis_(0.0f, 0.0f, 1.0f),  // Default: aligned with z-axis
      colorScheme_(0) {
}

void MagneticField::generateFieldLines(int numLines, float maxRadius) {
    fieldLines_.clear();

    // Generate field lines starting from different points around the poles
    const int linesPerPole = numLines / 2;
    const float startRadius = 3.0f;  // Start near event horizon

    for (int i = 0; i < linesPerPole; i++) {
        float angle = (2.0f * glm::pi<float>() * i) / linesPerPole;

        // North pole field lines
        glm::vec3 northStart = glm::vec3(
            startRadius * std::cos(angle),
            startRadius * std::sin(angle),
            startRadius * 0.5f  // Slightly above equator
        );

        // Rotate by magnetic axis
        northStart = northStart;  // TODO: Apply magnetic axis rotation if needed

        fieldLines_.push_back(traceFieldLine(northStart, maxRadius));

        // South pole field lines
        glm::vec3 southStart = glm::vec3(
            startRadius * std::cos(angle),
            startRadius * std::sin(angle),
            -startRadius * 0.5f
        );

        fieldLines_.push_back(traceFieldLine(southStart, maxRadius));
    }
}

FieldLine MagneticField::traceFieldLine(const glm::vec3& startPoint, float maxRadius, int maxSteps) {
    FieldLine line;
    line.points.reserve(maxSteps);

    glm::vec3 currentPoint = startPoint;
    float stepSize = 0.5f;

    for (int step = 0; step < maxSteps; step++) {
        // Add current point
        line.points.push_back(currentPoint);

        // Get field vector at current position
        glm::vec3 fieldDir = getFieldVector(currentPoint);

        if (glm::length(fieldDir) < 0.001f) {
            break;  // Field too weak, stop tracing
        }

        fieldDir = glm::normalize(fieldDir);

        // Move along field line
        currentPoint += fieldDir * stepSize;

        // Check if we've gone too far or too close
        float r = glm::length(currentPoint);
        if (r > maxRadius || r < 2.0f) {
            break;
        }

        // Check if we've looped back to the other pole
        if (line.points.size() > 10) {
            float startZ = startPoint.z;
            float currentZ = currentPoint.z;
            if (startZ * currentZ < 0 && std::abs(currentZ) > std::abs(startZ)) {
                // Crossed equator going away, likely closed field line
                break;
            }
        }
    }

    // Calculate average field strength along line
    float avgStrength = 0.0f;
    for (const auto& point : line.points) {
        avgStrength += getFieldStrengthAt(point);
    }
    if (!line.points.empty()) {
        avgStrength /= line.points.size();
    }

    line.intensity = std::min(avgStrength, 1.0f);
    line.color = getFieldLineColor(line.intensity, startPoint.z > 0);

    return line;
}

glm::vec3 MagneticField::getFieldVector(const glm::vec3& position) const {
    // Calculate basic dipole field
    glm::vec3 field = calculateDipoleField(position);

    // Apply frame-dragging correction if we have a Kerr metric
    if (metric_) {
        field = applyFrameDragging(position, field);
    }

    return field;
}

glm::vec3 MagneticField::calculateDipoleField(const glm::vec3& position) const {
    float r = glm::length(position);
    if (r < 0.1f) return glm::vec3(0.0f);

    glm::vec3 r_hat = position / r;

    // Magnetic moment vector (aligned with magnetic axis)
    glm::vec3 m = magneticAxis_ * static_cast<float>(fieldStrength_);

    // Dipole field: B = (3(m·r̂)r̂ - m) / r³
    float m_dot_r = glm::dot(m, r_hat);
    glm::vec3 field = (3.0f * m_dot_r * r_hat - m) / (r * r * r);

    return field;
}

glm::vec3 MagneticField::applyFrameDragging(const glm::vec3& position,
                                             const glm::vec3& field) const {
    // Frame-dragging effect: magnetic field lines are twisted by rotation
    // This is a simplified model - full treatment requires GRMHD

    float r = glm::length(position);
    if (r < 2.0f) return field;  // Inside event horizon

    // Get black hole spin parameter
    double spin = metric_->spin();  // This might need to be added to Metric interface

    // Frame-dragging angular velocity (simplified)
    // ω = 2Ma / (r³ + a²r + 2Ma²)
    float a = static_cast<float>(spin);
    float omega = (2.0f * a) / (r * r * r + a * a * r + 2.0f * a * a);

    // Twist field lines in azimuthal direction
    glm::vec3 azimuthal = glm::normalize(glm::cross(glm::vec3(0, 0, 1), position));
    if (glm::length(azimuthal) < 0.01f) {
        azimuthal = glm::vec3(1, 0, 0);  // Fallback for poles
    }

    // Add azimuthal component proportional to frame-dragging
    glm::vec3 twistedField = field + omega * 10.0f * azimuthal;

    return twistedField;
}

float MagneticField::getFieldStrengthAt(const glm::vec3& position) const {
    glm::vec3 field = getFieldVector(position);
    return glm::length(field);
}

glm::vec3 MagneticField::getFieldLineColor(float intensity, bool northPole) const {
    switch (colorScheme_) {
        case 0:  // Blue-white (standard)
            return glm::mix(glm::vec3(0.3f, 0.5f, 1.0f), glm::vec3(1.0f), intensity);

        case 1:  // Heat map (strength-based)
            if (intensity < 0.5f) {
                // Blue to cyan
                return glm::mix(glm::vec3(0.0f, 0.0f, 1.0f),
                              glm::vec3(0.0f, 1.0f, 1.0f),
                              intensity * 2.0f);
            } else {
                // Cyan to yellow to red
                return glm::mix(glm::vec3(0.0f, 1.0f, 1.0f),
                              glm::vec3(1.0f, 0.0f, 0.0f),
                              (intensity - 0.5f) * 2.0f);
            }

        case 2:  // Cyan-magenta (polarity)
            if (northPole) {
                return glm::vec3(0.0f, 1.0f, 1.0f) * intensity;  // Cyan for north
            } else {
                return glm::vec3(1.0f, 0.0f, 1.0f) * intensity;  // Magenta for south
            }

        default:
            return glm::vec3(1.0f);
    }
}

} // namespace rendering
} // namespace cosmic
