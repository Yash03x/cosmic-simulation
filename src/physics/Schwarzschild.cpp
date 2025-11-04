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

Eigen::Vector3d Schwarzschild::geodesicAcceleration(const Eigen::Vector3d& pos, const Eigen::Vector3d& vel) const {
    double r = pos[0];
    double theta = pos[1];
    double phi = pos[2];

    if (r <= eventHorizonRadius()) {
        throw std::domain_error("Schwarzschild coordinates undefined at/inside event horizon");
    }

    // Four-position and four-velocity are needed for the geodesic equation.
    // The time component of the position is not needed for Christoffel symbols, so we can use a dummy value.
    FourVector fourPos(0.0, r, theta, phi);

    // The time component of the velocity (dt/dλ) is not provided.
    // We need to calculate it from the metric and the fact that for a photon, the four-velocity squared is zero.
    // g_μν (dx^μ/dλ)(dx^ν/dλ) = 0
    MetricTensor g = metricTensor(fourPos);
    double g_tt = g(0, 0);
    double g_rr = g(1, 1);
    double g_thetatheta = g(2, 2);
    double g_phiphi = g(3, 3);

    double dr = vel[0];
    double dtheta = vel[1];
    double dphi = vel[2];

    // Solve for dt/dλ
    double dt_dlambda_sq = -(g_rr * dr * dr + g_thetatheta * dtheta * dtheta + g_phiphi * dphi * dphi) / g_tt;
    if (dt_dlambda_sq < 0) {
        // This can happen due to numerical precision issues close to the horizon
        dt_dlambda_sq = 0;
    }
    double dt_dlambda = std::sqrt(dt_dlambda_sq);

    FourVector fourVel(dt_dlambda, dr, dtheta, dphi);

    ChristoffelTensor gamma;
    christoffelSymbols(fourPos, gamma);

    Eigen::Vector3d acceleration = Eigen::Vector3d::Zero();
    for (int mu = 1; mu < 4; ++mu) { // Iterate over spatial components r, θ, φ
        double sum = 0.0;
        for (int alpha = 0; alpha < 4; ++alpha) {
            for (int beta = 0; beta < 4; ++beta) {
                sum += gamma[mu](alpha, beta) * fourVel[alpha] * fourVel[beta];
            }
        }
        acceleration[mu - 1] = -sum;
    }

    return acceleration;
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
