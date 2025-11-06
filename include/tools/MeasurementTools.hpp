#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <optional>

namespace cosmic {

// Forward declarations
namespace physics {
    class Metric;
}

namespace rendering {
    class Camera;
}

namespace tools {

/**
 * @brief Measurement point in 3D space
 */
struct MeasurementPoint {
    glm::vec3 position;
    std::string label;
    float timestamp;  // When it was created
};

/**
 * @brief Result of a distance measurement
 */
struct DistanceMeasurement {
    MeasurementPoint point1;
    MeasurementPoint point2;
    float euclideanDistance;      // Flat-space distance
    float properDistance;         // Distance accounting for spacetime curvature
    std::string description;
};

/**
 * @brief Result of an angle measurement
 */
struct AngleMeasurement {
    MeasurementPoint vertex;      // Vertex of the angle
    MeasurementPoint point1;      // First ray
    MeasurementPoint point2;      // Second ray
    float angleDegrees;
    float angleRadians;
    std::string description;
};

/**
 * @brief Result of a redshift measurement
 */
struct RedshiftMeasurement {
    MeasurementPoint observerPoint;
    MeasurementPoint emitterPoint;
    float gravitationalRedshift;  // z = (λ_obs - λ_em) / λ_em
    float escapeVelocity;         // Escape velocity at emission point (c units)
    float timeDialation;          // Time dilation factor
    std::string description;
};

/**
 * @brief Tools for measuring physical quantities in curved spacetime
 */
class MeasurementTools {
public:
    MeasurementTools();

    /**
     * @brief Add a measurement point at the given position
     * @param position World-space position
     * @param label Optional label for the point
     */
    void addPoint(const glm::vec3& position, const std::string& label = "");

    /**
     * @brief Clear all measurement points
     */
    void clear();

    /**
     * @brief Remove last added point
     */
    void removeLastPoint();

    /**
     * @brief Calculate distance between last two points
     * @param metric Black hole metric for proper distance calculation
     * @return Distance measurement or nullopt if insufficient points
     */
    std::optional<DistanceMeasurement> measureDistance(const physics::Metric* metric) const;

    /**
     * @brief Calculate angle formed by last three points
     * @return Angle measurement or nullopt if insufficient points
     */
    std::optional<AngleMeasurement> measureAngle() const;

    /**
     * @brief Calculate gravitational redshift between two points
     * @param metric Black hole metric
     * @return Redshift measurement or nullopt if insufficient points
     */
    std::optional<RedshiftMeasurement> measureRedshift(const physics::Metric* metric) const;

    /**
     * @brief Get all measurement points
     */
    const std::vector<MeasurementPoint>& getPoints() const { return points_; }

    /**
     * @brief Get number of points
     */
    size_t getPointCount() const { return points_.size(); }

    /**
     * @brief Check if measurement mode is active
     */
    bool isActive() const { return active_; }

    /**
     * @brief Set measurement mode active/inactive
     */
    void setActive(bool active) { active_ = active; }

    /**
     * @brief Get current measurement mode
     */
    enum class Mode {
        Distance,    // Measure distance between two points
        Angle,       // Measure angle formed by three points
        Redshift     // Measure gravitational redshift
    };

    void setMode(Mode mode) { mode_ = mode; }
    Mode getMode() const { return mode_; }

    /**
     * @brief Get all completed distance measurements
     */
    const std::vector<DistanceMeasurement>& getDistanceMeasurements() const {
        return distanceMeasurements_;
    }

    /**
     * @brief Get all completed angle measurements
     */
    const std::vector<AngleMeasurement>& getAngleMeasurements() const {
        return angleMeasurements_;
    }

    /**
     * @brief Get all completed redshift measurements
     */
    const std::vector<RedshiftMeasurement>& getRedshiftMeasurements() const {
        return redshiftMeasurements_;
    }

    /**
     * @brief Complete current measurement and start new one
     */
    void completeMeasurement(const physics::Metric* metric);

private:
    bool active_;
    Mode mode_;
    std::vector<MeasurementPoint> points_;

    // Completed measurements
    std::vector<DistanceMeasurement> distanceMeasurements_;
    std::vector<AngleMeasurement> angleMeasurements_;
    std::vector<RedshiftMeasurement> redshiftMeasurements_;

    /**
     * @brief Calculate proper distance in curved spacetime
     */
    float calculateProperDistance(const glm::vec3& p1, const glm::vec3& p2,
                                   const physics::Metric* metric) const;

    /**
     * @brief Calculate gravitational time dilation factor
     */
    float calculateTimeDilation(const glm::vec3& position,
                                 const physics::Metric* metric) const;
};

} // namespace tools
} // namespace cosmic
