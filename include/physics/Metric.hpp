#pragma once

#include <Eigen/Dense>
#include <array>

namespace cosmic {
namespace physics {

/**
 * @brief Base class for spacetime metrics
 *
 * Defines the interface for different black hole metrics (Schwarzschild, Kerr, etc.)
 * Uses geometric units where G = c = 1
 */
class Metric {
public:
    using FourVector = Eigen::Matrix<double, 4, 1>;
    using MetricTensor = Eigen::Matrix<double, 4, 4>;
    using ChristoffelTensor = std::array<MetricTensor, 4>; // Γ^μ_{αβ} for μ = 0..3

    virtual ~Metric() = default;

    /**
     * @brief Get the event horizon radius
     * @return Radius of the event horizon in geometric units
     */
    virtual double eventHorizonRadius() const = 0;

    /**
     * @brief Get the photon sphere radius
     * @return Radius where light can orbit
     */
    virtual double photonSphereRadius() const = 0;

    /**
     * @brief Get the ISCO (Innermost Stable Circular Orbit) radius
     * @return Radius of the innermost stable circular orbit
     */
    virtual double iscoRadius() const = 0;

    /**
     * @brief Compute geodesic acceleration for a particle/photon
     * @param position Position in spherical coordinates (r, θ, φ)
     * @param velocity Velocity/momentum in spherical coordinates
     * @return Acceleration vector
     */
    virtual Eigen::Vector3d geodesicAcceleration(
        const Eigen::Vector3d& position,
        const Eigen::Vector3d& velocity) const = 0;

    /**
     * @brief Compute the covariant metric tensor g_{μν} at a point
     * @param position Four-position (t, r, θ, φ)
     * @return 4x4 metric tensor
     */
    virtual MetricTensor metricTensor(const FourVector& position) const = 0;

    /**
     * @brief Compute the inverse metric tensor g^{μν} at a point
     * @param position Four-position (t, r, θ, φ)
     * @return 4x4 inverse metric tensor
     */
    virtual MetricTensor inverseMetricTensor(const FourVector& position) const = 0;

    /**
     * @brief Compute Christoffel symbols Γ^μ_{αβ} at a point
     * @param position Four-position (t, r, θ, φ)
     * @param outGamma Christoffel tensor to populate (μ major index)
     */
    virtual void christoffelSymbols(const FourVector& position,
                                    ChristoffelTensor& outGamma) const = 0;

    /**
     * @brief Get black hole mass
     * @return Mass in geometric units (solar masses)
     */
    virtual double getMass() const = 0;

    /**
     * @brief Get black hole spin parameter (a/M)
     * @return Spin parameter (0 for Schwarzschild, 0-1 for Kerr)
     */
    virtual double spin() const { return 0.0; }

    /**
     * @brief Check if position is inside event horizon
     * @param r Radial coordinate
     * @return True if inside event horizon
     */
    virtual bool isInsideEventHorizon(double r) const {
        return r < eventHorizonRadius();
    }

    /**
     * @brief Check if position is in photon sphere region
     * @param r Radial coordinate
     * @return True if near photon sphere
     */
    virtual bool isNearPhotonSphere(double r) const {
        double rPhoton = photonSphereRadius();
        return std::abs(r - rPhoton) < 0.1 * rPhoton;
    }
};

} // namespace physics
} // namespace cosmic
