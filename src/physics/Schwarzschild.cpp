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

    double dr = vel[0];
    double dtheta = vel[1];
    double dphi = vel[2];

    // Schwarzschild radius
    double rs = schwarzschildRadius();

    if (r <= rs) {
        throw std::domain_error("Schwarzschild coordinates undefined at/inside event horizon");
    }

    double sinTheta = std::sin(theta);
    double cosTheta = std::cos(theta);

    double gamma_r_rr = rs / (2.0 * r * (r - rs)); // Γ^r_{rr}
    double gamma_r_theta_theta = -(r - rs);        // Γ^r_{θθ}
    double gamma_r_phi_phi = gamma_r_theta_theta * sinTheta * sinTheta; // Γ^r_{φφ}

    double gamma_theta_r_theta = 1.0 / r;                  // Γ^θ_{rθ}
    double gamma_theta_phi_phi = -sinTheta * cosTheta;     // Γ^θ_{φφ}

    double gamma_phi_r_phi = 1.0 / r;                      // Γ^φ_{rφ}
    double gamma_phi_theta_phi = 0.0;                      // Γ^φ_{θφ}
    if (std::abs(sinTheta) > constants::EPSILON) {
        gamma_phi_theta_phi = cosTheta / sinTheta;
    }

    // Compute geodesic acceleration: d²x^μ/dλ² = -Γ^μ_αβ (dx^α/dλ)(dx^β/dλ)

    // d²r/dλ²
    double d2r = -gamma_r_rr * dr * dr
                 -gamma_r_theta_theta * dtheta * dtheta
                 -gamma_r_phi_phi * dphi * dphi;

    // d²θ/dλ²
    double d2theta = -2.0 * gamma_theta_r_theta * dr * dtheta
                     -gamma_theta_phi_phi * dphi * dphi;

    // d²φ/dλ²
    double d2phi = -2.0 * gamma_phi_r_phi * dr * dphi
                   -2.0 * gamma_phi_theta_phi * dtheta * dphi;

    return Eigen::Vector3d(d2r, d2theta, d2phi);
}

Metric::MetricTensor Schwarzschild::metricTensor(const FourVector& position) const {
    double r = position[1];
    double theta = position[2];
    double rs = schwarzschildRadius();

    if (r <= 0.0) {
        throw std::domain_error("Radial coordinate must be positive in Schwarzschild metric");
    }
    if (r <= rs) {
        throw std::domain_error("Schwarzschild metric undefined at/inside event horizon (r <= 2M)");
    }

    double f = 1.0 - (rs / r);
    double sinTheta = std::sin(theta);

    MetricTensor g = MetricTensor::Zero();
    g(0, 0) = -f;
    g(1, 1) = 1.0 / f;
    g(2, 2) = r * r;
    g(3, 3) = r * r * sinTheta * sinTheta;
    return g;
}

Metric::MetricTensor Schwarzschild::inverseMetricTensor(const FourVector& position) const {
    double r = position[1];
    double theta = position[2];
    double rs = schwarzschildRadius();

    if (r <= 0.0) {
        throw std::domain_error("Radial coordinate must be positive in Schwarzschild metric");
    }
    if (r <= rs) {
        throw std::domain_error("Schwarzschild metric undefined at/inside event horizon (r <= 2M)");
    }

    double f = 1.0 - (rs / r);
    double sinTheta = std::sin(theta);

    MetricTensor gInv = MetricTensor::Zero();
    gInv(0, 0) = -1.0 / f;
    gInv(1, 1) = f;
    gInv(2, 2) = 1.0 / (r * r);
    gInv(3, 3) = 1.0 / (r * r * sinTheta * sinTheta);
    return gInv;
}

void Schwarzschild::christoffelSymbols(const FourVector& position,
                                       ChristoffelTensor& outGamma) const {
    double r = position[1];
    double theta = position[2];
    double rs = schwarzschildRadius();

    if (r <= 0.0) {
        throw std::domain_error("Radial coordinate must be positive in Schwarzschild metric");
    }
    if (r <= rs) {
        throw std::domain_error("Schwarzschild Christoffel symbols not defined at/inside event horizon (r <= 2M)");
    }

    for (auto& matrix : outGamma) {
        matrix.setZero();
    }

    double sinTheta = std::sin(theta);
    double cosTheta = std::cos(theta);
    double sinThetaSq = sinTheta * sinTheta;

    double denom = r - rs;
    double mass = mass_;

    // Γ^t_{tr} = Γ^t_{rt} = M / (r (r - 2M))
    double gamma_t_tr = mass / (r * denom);
    outGamma[0](0, 1) = gamma_t_tr;
    outGamma[0](1, 0) = gamma_t_tr;

    // Γ^r_{tt} = M (r - 2M) / r^3
    double gamma_r_tt = mass * denom / (r * r * r);
    outGamma[1](0, 0) = gamma_r_tt;

    // Γ^r_{rr} = -M / (r (r - 2M))
    double gamma_r_rr = -mass / (r * denom);
    outGamma[1](1, 1) = gamma_r_rr;

    // Γ^r_{θθ} = -(r - 2M)
    double gamma_r_theta_theta = -denom;
    outGamma[1](2, 2) = gamma_r_theta_theta;

    // Γ^r_{φφ} = -(r - 2M) sin^2 θ
    outGamma[1](3, 3) = gamma_r_theta_theta * sinThetaSq;

    // Γ^θ_{rθ} = Γ^θ_{θr} = 1/r
    double gamma_theta_r_theta = 1.0 / r;
    outGamma[2](1, 2) = gamma_theta_r_theta;
    outGamma[2](2, 1) = gamma_theta_r_theta;

    // Γ^θ_{φφ} = -sin θ cos θ
    outGamma[2](3, 3) = -sinTheta * cosTheta;

    // Γ^φ_{rφ} = Γ^φ_{φr} = 1/r
    double gamma_phi_r_phi = 1.0 / r;
    outGamma[3](1, 3) = gamma_phi_r_phi;
    outGamma[3](3, 1) = gamma_phi_r_phi;

    // Γ^φ_{θφ} = Γ^φ_{φθ} = cot θ
    if (std::abs(sinTheta) > constants::EPSILON) {
        double cotTheta = cosTheta / sinTheta;
        outGamma[3](2, 3) = cotTheta;
        outGamma[3](3, 2) = cotTheta;
    }
}

} // namespace physics
} // namespace cosmic
