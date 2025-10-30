#pragma once

#include "Metric.hpp"
#include "../utils/Constants.hpp"

namespace cosmic {
namespace physics {

/**
 * @brief Schwarzschild metric for non-rotating black holes
 *
 * Metric: ds² = -(1 - rs/r)dt² + (1 - rs/r)⁻¹dr² + r²(dθ² + sin²θ dφ²)
 * where rs = 2M (Schwarzschild radius)
 *
 * In geometric units: G = c = 1
 */
class Schwarzschild : public Metric {
public:
    /**
     * @brief Construct Schwarzschild black hole
     * @param mass Black hole mass in geometric units (solar masses)
     */
    explicit Schwarzschild(double mass);

    // Metric interface implementation
    double eventHorizonRadius() const override {
        return 2.0 * mass_;  // rs = 2GM/c² = 2M (in geometric units)
    }

    double photonSphereRadius() const override {
        return 3.0 * mass_;  // r = 3GM/c² = 3M
    }

    double iscoRadius() const override {
        return 6.0 * mass_;  // r = 6GM/c² = 6M
    }

    double getMass() const override {
        return mass_;
    }

    /**
     * @brief Compute geodesic acceleration using Christoffel symbols
     *
     * Solves: d²x^μ/dλ² + Γ^μ_αβ (dx^α/dλ)(dx^β/dλ) = 0
     *
     * @param pos Position in spherical coordinates (r, θ, φ)
     * @param vel Velocity in spherical coordinates (dr/dλ, dθ/dλ, dφ/dλ)
     * @return Acceleration (d²r/dλ², d²θ/dλ², d²φ/dλ²)
     */
    Eigen::Vector3d geodesicAcceleration(
        const Eigen::Vector3d& pos,
        const Eigen::Vector3d& vel) const override;

    /**
     * @brief Get Schwarzschild radius
     * @return rs = 2M
     */
    double schwarzschildRadius() const {
        return eventHorizonRadius();
    }

private:
    double mass_;  // Black hole mass in geometric units

    /**
     * @brief Compute Christoffel symbol components at position
     * @param r Radial coordinate
     * @param theta Polar angle
     * @return Struct containing relevant Christoffel symbols
     */
    struct ChristoffelSymbols {
        // Γ^r components
        double gamma_r_rr;
        double gamma_r_theta_theta;
        double gamma_r_phi_phi;

        // Γ^θ components
        double gamma_theta_r_theta;
        double gamma_theta_phi_phi;

        // Γ^φ components
        double gamma_phi_r_phi;
        double gamma_phi_theta_phi;
    };

    ChristoffelSymbols computeChristoffel(double r, double theta) const;
};

} // namespace physics
} // namespace cosmic
