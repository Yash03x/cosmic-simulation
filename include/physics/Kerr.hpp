#pragma once

#include "Metric.hpp"
#include "../utils/Constants.hpp"
#include <Eigen/Dense>

namespace cosmic {
namespace physics {

/**
 * @brief Kerr metric for rotating black holes
 *
 * Implements the Kerr solution in Boyer-Lindquist coordinates
 * ds² = -(1 - 2Mr/Σ)dt² - (4Mra sin²θ/Σ)dtdφ + (Σ/Δ)dr² + Σdθ²
 *       + [(r²+a²)² - a²Δsin²θ]/Σ sin²θ dφ²
 *
 * where:
 * - a = J/M is the dimensionless spin parameter (0 ≤ a ≤ M)
 * - Σ = r² + a²cos²θ
 * - Δ = r² - 2Mr + a²
 */
class Kerr : public Metric {
public:
    /**
     * @brief Construct Kerr black hole
     * @param mass Black hole mass in solar masses
     * @param spin Dimensionless spin parameter a/M (0 to 0.998)
     */
    explicit Kerr(double mass, double spin = 0.0);

    /**
     * @brief Get black hole mass
     * @return Mass in solar masses
     */
    double getMass() const override { return mass_; }

    /**
     * @brief Get event horizon radius (depends on spin)
     * @return r+ = M + sqrt(M² - a²) in geometric units
     */
    double eventHorizonRadius() const override;

    /**
     * @brief Get photon sphere radius (depends on spin and theta)
     * @return Approximate photon orbit radius
     */
    double photonSphereRadius() const override;

    /**
     * @brief Get ISCO radius (depends on spin - prograde/retrograde)
     * @return r_ISCO (ranges from 1M to 9M depending on spin)
     */
    double iscoRadius() const override;

    /**
     * @brief Compute geodesic acceleration
     *
     * Solves: d²x^μ/dλ² + Γ^μ_αβ (dx^α/dλ)(dx^β/dλ) = 0
     *
     * @param pos Position in Boyer-Lindquist coordinates (r, θ, φ)
     * @param vel Velocity in Boyer-Lindquist coordinates (dr/dλ, dθ/dλ, dφ/dλ)
     * @return Acceleration (d²r/dλ², d²θ/dλ², d²φ/dλ²)
     */
    Eigen::Vector3d geodesicAcceleration(
        const Eigen::Vector3d& pos,
        const Eigen::Vector3d& vel) const override;

    MetricTensor metricTensor(const FourVector& position) const override;

    MetricTensor inverseMetricTensor(const FourVector& position) const override;

    void christoffelSymbols(const FourVector& position,
                            ChristoffelTensor& outGamma) const override;

    /**
     * @brief Get spin parameter
     * @return a/M (dimensionless, 0 to 0.998)
     */
    double getSpin() const { return spin_; }

    /**
     * @brief Set spin parameter
     * @param spin New spin value (will be clamped to [0, 0.998])
     */
    void setSpin(double spin);

    /**
     * @brief Get ergosphere outer radius at given theta
     * @param theta Polar angle
     * @return r_ergo = M + sqrt(M² - a²cos²θ)
     */
    double ergosphereRadius(double theta) const;

    /**
     * @brief Check if position is inside ergosphere
     * @param r Radial coordinate
     * @param theta Polar angle
     * @return True if inside ergosphere
     */
    bool isInsideErgosphere(double r, double theta) const;

    /**
     * @brief Get frame dragging angular velocity (ZAMO)
     * @param r Radial coordinate
     * @param theta Polar angle
     * @return ω = dφ/dt for Zero Angular Momentum Observers
     */
    double frameDraggingOmega(double r, double theta) const;

private:
    double mass_;  // Black hole mass
    double spin_;  // Dimensionless spin a/M

    /**
     * @brief Compute metric functions
     */
    struct MetricFunctions {
        double Sigma;  // Σ = r² + a²cos²θ
        double Delta;  // Δ = r² - 2Mr + a²
        double A;      // A = (r²+a²)² - a²Δsin²θ
    };

    MetricFunctions computeMetricFunctions(double r, double theta) const;

    /**
     * @brief Compute Christoffel symbol components
     */
    struct ChristoffelSymbols {
        // Γ^r components
        double gamma_r_rr;
        double gamma_r_rtheta;
        double gamma_r_theta_theta;
        double gamma_r_phi_phi;

        // Γ^θ components
        double gamma_theta_rr;
        double gamma_theta_r_theta;
        double gamma_theta_theta_theta;
        double gamma_theta_phi_phi;

        // Γ^φ components
        double gamma_phi_r_phi;
        double gamma_phi_theta_phi;
    };

    ChristoffelSymbols computeChristoffel(double r, double theta) const;
};

} // namespace physics
} // namespace cosmic
