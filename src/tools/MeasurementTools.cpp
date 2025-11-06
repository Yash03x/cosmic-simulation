#include "tools/MeasurementTools.hpp"
#include "physics/Metric.hpp"
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <sstream>
#include <iomanip>

namespace cosmic {
namespace tools {

MeasurementTools::MeasurementTools()
    : active_(false),
      mode_(Mode::Distance) {
}

void MeasurementTools::addPoint(const glm::vec3& position, const std::string& label) {
    MeasurementPoint point;
    point.position = position;
    point.label = label.empty() ? "Point " + std::to_string(points_.size() + 1) : label;
    point.timestamp = 0.0f;  // Could use actual time if needed

    points_.push_back(point);
}

void MeasurementTools::clear() {
    points_.clear();
}

void MeasurementTools::removeLastPoint() {
    if (!points_.empty()) {
        points_.pop_back();
    }
}

std::optional<DistanceMeasurement> MeasurementTools::measureDistance(
    const physics::Metric* metric) const {

    if (points_.size() < 2) {
        return std::nullopt;
    }

    const auto& p1 = points_[points_.size() - 2];
    const auto& p2 = points_[points_.size() - 1];

    DistanceMeasurement result;
    result.point1 = p1;
    result.point2 = p2;

    // Calculate Euclidean distance
    glm::vec3 diff = p2.position - p1.position;
    result.euclideanDistance = glm::length(diff);

    // Calculate proper distance (accounting for spacetime curvature)
    result.properDistance = calculateProperDistance(p1.position, p2.position, metric);

    // Create description
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3);
    oss << "Distance: " << result.euclideanDistance << " M (flat)\n";
    oss << "Proper distance: " << result.properDistance << " M (curved)";
    result.description = oss.str();

    return result;
}

std::optional<AngleMeasurement> MeasurementTools::measureAngle() const {
    if (points_.size() < 3) {
        return std::nullopt;
    }

    const auto& vertex = points_[points_.size() - 3];
    const auto& p1 = points_[points_.size() - 2];
    const auto& p2 = points_[points_.size() - 1];

    AngleMeasurement result;
    result.vertex = vertex;
    result.point1 = p1;
    result.point2 = p2;

    // Calculate vectors from vertex to each point
    glm::vec3 v1 = glm::normalize(p1.position - vertex.position);
    glm::vec3 v2 = glm::normalize(p2.position - vertex.position);

    // Calculate angle using dot product
    float cosAngle = glm::clamp(glm::dot(v1, v2), -1.0f, 1.0f);
    result.angleRadians = std::acos(cosAngle);
    result.angleDegrees = glm::degrees(result.angleRadians);

    // Create description
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "Angle: " << result.angleDegrees << "° (" << result.angleRadians << " rad)";
    result.description = oss.str();

    return result;
}

std::optional<RedshiftMeasurement> MeasurementTools::measureRedshift(
    const physics::Metric* metric) const {

    if (points_.size() < 2 || !metric) {
        return std::nullopt;
    }

    const auto& observer = points_[points_.size() - 2];
    const auto& emitter = points_[points_.size() - 1];

    RedshiftMeasurement result;
    result.observerPoint = observer;
    result.emitterPoint = emitter;

    // Calculate time dilation at each point
    float timeDialationObserver = calculateTimeDilation(observer.position, metric);
    float timeDialationEmitter = calculateTimeDilation(emitter.position, metric);

    // Gravitational redshift formula: z = sqrt(g_tt_obs / g_tt_em) - 1
    result.timeDialation = timeDialationEmitter / timeDialationObserver;
    result.gravitationalRedshift = result.timeDialation - 1.0f;

    // Calculate escape velocity at emission point
    float r = glm::length(emitter.position);
    double eventHorizon = metric->eventHorizonRadius();

    if (r > eventHorizon) {
        result.escapeVelocity = std::sqrt(2.0 * eventHorizon / r);
    } else {
        result.escapeVelocity = 1.0f;  // Inside horizon, escape velocity > c
    }

    // Create description
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(4);
    oss << "Gravitational redshift z: " << result.gravitationalRedshift << "\n";
    oss << "Time dilation: " << result.timeDialation << "x\n";
    oss << std::setprecision(3);
    oss << "Escape velocity: " << (result.escapeVelocity * 100.0f) << "% c";
    result.description = oss.str();

    return result;
}

void MeasurementTools::completeMeasurement(const physics::Metric* metric) {
    switch (mode_) {
        case Mode::Distance: {
            auto result = measureDistance(metric);
            if (result) {
                distanceMeasurements_.push_back(*result);
                // Keep last point for next measurement
                if (points_.size() > 1) {
                    MeasurementPoint lastPoint = points_.back();
                    points_.clear();
                    points_.push_back(lastPoint);
                }
            }
            break;
        }
        case Mode::Angle: {
            auto result = measureAngle();
            if (result) {
                angleMeasurements_.push_back(*result);
                points_.clear();
            }
            break;
        }
        case Mode::Redshift: {
            auto result = measureRedshift(metric);
            if (result) {
                redshiftMeasurements_.push_back(*result);
                // Keep last point for next measurement
                if (points_.size() > 1) {
                    MeasurementPoint lastPoint = points_.back();
                    points_.clear();
                    points_.push_back(lastPoint);
                }
            }
            break;
        }
    }
}

float MeasurementTools::calculateProperDistance(const glm::vec3& p1, const glm::vec3& p2,
                                                  const physics::Metric* metric) const {
    // For a simple approximation, integrate along straight line in coordinate space
    // More accurate would be to find geodesic, but this is sufficient for measurements

    const int steps = 100;
    float properDistance = 0.0f;

    for (int i = 0; i < steps; i++) {
        float t = static_cast<float>(i) / steps;
        glm::vec3 pos = p1 + t * (p2 - p1);

        // Convert to spherical coordinates
        float r = glm::length(pos);
        if (r < 0.001f) r = 0.001f;  // Avoid singularity

        // Get metric component g_rr at this point
        // For Schwarzschild: g_rr = (1 - r_s/r)^(-1)
        double rs = 2.0 * metric->eventHorizonRadius();
        float grr = (r > rs / 2.0) ? 1.0f / (1.0f - rs / r) : 1.0f;

        // Differential proper distance: ds^2 = g_rr dr^2 + r^2 dΩ^2
        float dr = glm::length(p2 - p1) / steps;
        properDistance += std::sqrt(std::abs(grr)) * dr;
    }

    return properDistance;
}

float MeasurementTools::calculateTimeDilation(const glm::vec3& position,
                                                const physics::Metric* metric) const {
    float r = glm::length(position);
    if (r < 0.001f) r = 0.001f;

    double eventHorizon = metric->eventHorizonRadius();

    // Time dilation factor: dt_proper / dt_infinity = sqrt(g_tt)
    // For Schwarzschild: g_tt = -(1 - 2M/r)
    if (r > eventHorizon) {
        float gtt = 1.0f - 2.0f * static_cast<float>(eventHorizon) / r;
        return 1.0f / std::sqrt(gtt);  // Inverse because we want dt_proper / dt_inf
    } else {
        return 1e10f;  // Extreme time dilation inside horizon
    }
}

} // namespace tools
} // namespace cosmic
