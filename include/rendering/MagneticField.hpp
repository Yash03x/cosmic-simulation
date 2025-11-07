#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace cosmic {

// Forward declaration
namespace physics {
    class Metric;
}

namespace rendering {

/**
 * @brief Represents a single magnetic field line
 */
struct FieldLine {
    std::vector<glm::vec3> points;  // Points along the field line
    glm::vec3 color;                // Color of the field line
    float intensity;                // Field strength (0-1)
};

/**
 * @brief Magnetic field configuration around a black hole
 */
class MagneticField {
public:
    /**
     * @brief Construct magnetic field visualization
     * @param metric Black hole metric (for frame-dragging effects)
     * @param fieldStrength Overall field strength (Tesla)
     */
    explicit MagneticField(const physics::Metric* metric = nullptr,
                          double fieldStrength = 1e8);

    /**
     * @brief Generate field lines
     * @param numLines Number of field lines to generate
     * @param maxRadius Maximum radius to trace field lines
     */
    void generateFieldLines(int numLines = 16, float maxRadius = 100.0f);

    /**
     * @brief Get all field lines
     */
    const std::vector<FieldLine>& getFieldLines() const { return fieldLines_; }

    /**
     * @brief Set field strength
     */
    void setFieldStrength(double strength) { fieldStrength_ = strength; }

    /**
     * @brief Get field strength
     */
    double getFieldStrength() const { return fieldStrength_; }

    /**
     * @brief Set visibility
     */
    void setVisible(bool visible) { visible_ = visible; }

    /**
     * @brief Check if visible
     */
    bool isVisible() const { return visible_; }

    /**
     * @brief Set magnetic axis (rotation axis of field)
     */
    void setMagneticAxis(const glm::vec3& axis) { magneticAxis_ = glm::normalize(axis); }

    /**
     * @brief Get magnetic axis
     */
    glm::vec3 getMagneticAxis() const { return magneticAxis_; }

    /**
     * @brief Set field line color scheme
     * 0 = Blue-white (standard)
     * 1 = Heat map (strength-based)
     * 2 = Cyan-magenta (polarity)
     */
    void setColorScheme(int scheme) { colorScheme_ = scheme; }

    /**
     * @brief Get color scheme
     */
    int getColorScheme() const { return colorScheme_; }

    /**
     * @brief Calculate magnetic field vector at position
     * Uses dipole field with frame-dragging corrections
     */
    glm::vec3 getFieldVector(const glm::vec3& position) const;

    /**
     * @brief Calculate field strength at position
     */
    float getFieldStrengthAt(const glm::vec3& position) const;

private:
    const physics::Metric* metric_;
    double fieldStrength_;
    bool visible_;
    glm::vec3 magneticAxis_;
    int colorScheme_;

    std::vector<FieldLine> fieldLines_;

    /**
     * @brief Trace a single field line from starting point
     */
    FieldLine traceFieldLine(const glm::vec3& startPoint, float maxRadius, int maxSteps = 200);

    /**
     * @brief Get color for field line based on scheme and intensity
     */
    glm::vec3 getFieldLineColor(float intensity, bool northPole) const;

    /**
     * @brief Calculate magnetic dipole field
     * B = (3(m·r̂)r̂ - m) / r³
     */
    glm::vec3 calculateDipoleField(const glm::vec3& position) const;

    /**
     * @brief Apply frame-dragging correction to field direction
     */
    glm::vec3 applyFrameDragging(const glm::vec3& position, const glm::vec3& field) const;
};

} // namespace rendering
} // namespace cosmic
