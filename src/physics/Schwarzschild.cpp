#include "physics/Schwarzschild.hpp"
#include <cmath>
#include <stdexcept>

namespace cosmic {
namespace physics {

Schwarzschild::Schwarzschild(double mass) : mass_(mass) {
    if (mass <= 0.0) {
        throw std::invalid_argument("Black hole mass must be positive");
    }
}

Eigen::Vector3d Schwarzschild::geodesicAcceleration(
    const Eigen::Vector3d& pos,
    const Eigen::Vector3d& vel) const {

    double r = pos[0];
    double theta = pos[1];
    // phi = pos[2]  // Not needed for acceleration calculation

    double dr = vel[0];
    double dtheta = vel[1];
    double dphi = vel[2];

    // Schwarzschild radius
    double rs = schwarzschildRadius();

    // Prevent division by zero at event horizon
    if (r <= rs * 1.001) {
        return Eigen::Vector3d::Zero();
    }

    // Compute Christoffel symbols
    auto gamma = computeChristoffel(r, theta);

    // Compute geodesic acceleration: d²x^μ/dλ² = -Γ^μ_αβ (dx^α/dλ)(dx^β/dλ)

    // d²r/dλ²
    double d2r = -gamma.gamma_r_rr * dr * dr
                 -gamma.gamma_r_theta_theta * dtheta * dtheta
                 -gamma.gamma_r_phi_phi * dphi * dphi;

    // d²θ/dλ²
    double d2theta = -2.0 * gamma.gamma_theta_r_theta * dr * dtheta
                     -gamma.gamma_theta_phi_phi * dphi * dphi;

    // d²φ/dλ²
    double d2phi = -2.0 * gamma.gamma_phi_r_phi * dr * dphi
                   -2.0 * gamma.gamma_phi_theta_phi * dtheta * dphi;

    return Eigen::Vector3d(d2r, d2theta, d2phi);
}

Schwarzschild::ChristoffelSymbols Schwarzschild::computeChristoffel(
    double r, double theta) const {

    ChristoffelSymbols gamma;

    double rs = schwarzschildRadius();

    // Avoid singularities
    if (r <= rs * 1.001) {
        return gamma;  // Return zero symbols near horizon
    }

    // Precompute common terms
    double r2 = r * r;
    double sin_theta = std::sin(theta);
    double cos_theta = std::cos(theta);
    double sin2_theta = sin_theta * sin_theta;

    // Γ^r_rr = rs / (2r(r - rs))
    gamma.gamma_r_rr = rs / (2.0 * r * (r - rs));

    // Γ^r_θθ = -(r - rs)
    gamma.gamma_r_theta_theta = -(r - rs);

    // Γ^r_φφ = -(r - rs) * sin²θ
    gamma.gamma_r_phi_phi = -(r - rs) * sin2_theta;

    // Γ^θ_rθ = Γ^θ_θr = 1/r
    gamma.gamma_theta_r_theta = 1.0 / r;

    // Γ^θ_φφ = -sinθ * cosθ
    gamma.gamma_theta_phi_phi = -sin_theta * cos_theta;

    // Γ^φ_rφ = Γ^φ_φr = 1/r
    gamma.gamma_phi_r_phi = 1.0 / r;

    // Γ^φ_θφ = Γ^φ_φθ = cotθ = cosθ/sinθ
    if (std::abs(sin_theta) > constants::EPSILON) {
        gamma.gamma_phi_theta_phi = cos_theta / sin_theta;
    } else {
        gamma.gamma_phi_theta_phi = 0.0;
    }

    return gamma;
}

} // namespace physics
} // namespace cosmic
