#include "physics/Kerr.hpp"
#include <cmath>
#include <stdexcept>

namespace cosmic {
namespace physics {

namespace {

constexpr double MAX_SPIN = 0.998; // Thin-disk limit for astrophysical Kerr BHs

inline double clampSpin(double spin) {
    if (spin < -MAX_SPIN) return -MAX_SPIN;
    if (spin > MAX_SPIN) return MAX_SPIN;
    return spin;
}

inline double toSpinParameter(double mass, double dimensionlessSpin) {
    return clampSpin(dimensionlessSpin) * mass;
}

struct MetricCache {
    double Sigma;
    double Delta;
    double A;
    double sinTheta;
    double cosTheta;
    double sin2Theta;
    double cos2Theta;
};

MetricCache computeMetricCache(double mass, double a, double r, double theta) {
    MetricCache cache{};
    cache.sinTheta = std::sin(theta);
    cache.cosTheta = std::cos(theta);
    cache.sin2Theta = cache.sinTheta * cache.sinTheta;
    cache.cos2Theta = cache.cosTheta * cache.cosTheta;

    double r2 = r * r;
    double a2 = a * a;
    cache.Sigma = r2 + a2 * cache.cos2Theta;
    cache.Delta = r2 - 2.0 * mass * r + a2;
    cache.A = (r2 + a2) * (r2 + a2) - a2 * cache.Delta * cache.sin2Theta;
    return cache;
}

void computeMetricTensorComponents(double mass,
                                   double a,
                                   double r,
                                   double theta,
                                   const MetricCache& cache,
                                   Metric::MetricTensor& g) {
    g.setZero();

    const double Sigma = cache.Sigma;
    const double Delta = cache.Delta;
    const double A = cache.A;
    const double sin2 = cache.sin2Theta;

    // Metric components (Boyer-Lindquist coordinates, signature -+++)
    double g_tt = -(1.0 - (2.0 * mass * r) / Sigma);
    double g_tphi = - (2.0 * mass * r * a * sin2) / Sigma;
    double g_rr = Sigma / Delta;
    double g_thetatheta = Sigma;
    double g_phiphi = (A * sin2) / Sigma;

    g(0, 0) = g_tt;
    g(0, 3) = g_tphi;
    g(3, 0) = g_tphi;
    g(1, 1) = g_rr;
    g(2, 2) = g_thetatheta;
    g(3, 3) = g_phiphi;
}

void computeMetricDerivatives(double mass,
                              double a,
                              double r,
                              double theta,
                              const MetricCache& cache,
                              Metric::MetricTensor& dr,
                              Metric::MetricTensor& dtheta) {
    dr.setZero();
    dtheta.setZero();

    const double Sigma = cache.Sigma;
    const double Delta = cache.Delta;
    const double A = cache.A;
    const double sinTheta = cache.sinTheta;
    const double cosTheta = cache.cosTheta;
    const double sin2 = cache.sin2Theta;
    const double cos2 = cache.cos2Theta;

    const double r2 = r * r;
    const double a2 = a * a;

    const double dSigma_dr = 2.0 * r;
    const double dSigma_dtheta = -2.0 * a2 * sinTheta * cosTheta;

    const double dDelta_dr = 2.0 * r - 2.0 * mass;
    const double dDelta_dtheta = 0.0;

    const double dA_dr = 4.0 * r * (r2 + a2) - a2 * dDelta_dr * sin2;
    const double dA_dtheta = -2.0 * a2 * Delta * sinTheta * cosTheta;

    // g_tt derivatives
    double g_tt = -(1.0 - (2.0 * mass * r) / Sigma);
    double dgtt_dr = (2.0 * mass / Sigma) - (2.0 * mass * r * dSigma_dr) / (Sigma * Sigma);
    double dgtt_dtheta = -(2.0 * mass * r * dSigma_dtheta) / (Sigma * Sigma);

    // g_tphi derivatives
    double factor = -2.0 * mass * a;
    double numerator = r * sin2;

    double dnumerator_dr = sin2;
    double dnumerator_dtheta = r * 2.0 * sinTheta * cosTheta;

    double dg_tphi_dr =
        factor * ((dnumerator_dr * Sigma - numerator * dSigma_dr) / (Sigma * Sigma));
    double dg_tphi_dtheta =
        factor * ((dnumerator_dtheta * Sigma - numerator * dSigma_dtheta) / (Sigma * Sigma));

    // g_rr derivatives
    double g_rr = Sigma / Delta;
    double dgrr_dr = (dSigma_dr * Delta - Sigma * dDelta_dr) / (Delta * Delta);
    double dgrr_dtheta = dSigma_dtheta / Delta;

    // g_theta theta derivatives
    double dgtheta_dr = dSigma_dr;
    double dgtheta_dtheta = dSigma_dtheta;

    // g_phi phi derivatives
    double d_phiphi_dr =
        (dA_dr * sin2 * Sigma - A * sin2 * dSigma_dr) / (Sigma * Sigma);

    double sin_cos = 2.0 * sinTheta * cosTheta;
    double d_phiphi_dtheta =
        ((dA_dtheta * sin2 + A * sin_cos) * Sigma - A * sin2 * dSigma_dtheta) /
        (Sigma * Sigma);

    auto setSym = [](Metric::MetricTensor& m, int i, int j, double value) {
        m(i, j) = value;
        m(j, i) = value;
    };

    dr(0, 0) = dgtt_dr;
    setSym(dr, 0, 3, dg_tphi_dr);
    dr(1, 1) = dgrr_dr;
    dr(2, 2) = dgtheta_dr;
    dr(3, 3) = d_phiphi_dr;

    dtheta(0, 0) = dgtt_dtheta;
    setSym(dtheta, 0, 3, dg_tphi_dtheta);
    dtheta(1, 1) = dgrr_dtheta;
    dtheta(2, 2) = dgtheta_dtheta;
    dtheta(3, 3) = d_phiphi_dtheta;
}

double solveTimeComponent(double mass,
                          double a,
                          double r,
                          double theta,
                          double pr,
                          double ptheta,
                          double pphi) {
    MetricCache cache = computeMetricCache(mass, a, r, theta);
    Metric::MetricTensor g;
    computeMetricTensorComponents(mass, a, r, theta, cache, g);

    const double g_tt = g(0, 0);
    const double g_tphi = g(0, 3);
    const double g_rr = g(1, 1);
    const double g_thetatheta = g(2, 2);
    const double g_phiphi = g(3, 3);

    double A = g_tt;
    double B = 2.0 * g_tphi * pphi;
    double C = g_rr * pr * pr + g_thetatheta * ptheta * ptheta + g_phiphi * pphi * pphi;

    double discriminant = B * B - 4.0 * A * C;
    if (discriminant < 0.0) {
        discriminant = 0.0;
    }
    double sqrtDisc = std::sqrt(discriminant);

    // Choose root that yields positive energy (future-directed)
    double denom = 2.0 * A;
    if (std::abs(denom) < constants::EPSILON) {
        denom = (denom >= 0.0 ? constants::EPSILON : -constants::EPSILON);
    }

    double p_t_candidate1 = (-B + sqrtDisc) / denom;
    double p_t_candidate2 = (-B - sqrtDisc) / denom;

    double g_t0 = g_tt;
    double energy1 = -(g_t0 * p_t_candidate1 + g_tphi * pphi);
    double energy2 = -(g_t0 * p_t_candidate2 + g_tphi * pphi);

    return (energy1 > energy2) ? p_t_candidate1 : p_t_candidate2;
}

} // namespace

Kerr::Kerr(double mass, double spin)
    : mass_(mass),
      spin_(clampSpin(spin)) {
    if (mass_ <= 0.0) {
        throw std::invalid_argument("Black hole mass must be positive");
    }
}

double Kerr::eventHorizonRadius() const {
    double a = toSpinParameter(mass_, spin_);
    double discriminant = mass_ * mass_ - a * a;
    if (discriminant < 0.0) discriminant = 0.0;
    return mass_ + std::sqrt(discriminant);
}

double Kerr::photonSphereRadius() const {
    // Equatorial prograde photon orbit approximation
    double a = toSpinParameter(mass_, spin_);
    double absA = std::abs(a);
    double term = std::acos(-absA / mass_);
    return 2.0 * mass_ * (1.0 + std::cos((2.0 / 3.0) * term));
}

double Kerr::iscoRadius() const {
    double a = toSpinParameter(mass_, spin_);
    double z1 = 1.0 + std::cbrt(1.0 - (a / mass_) * (a / mass_)) *
                        (std::cbrt(1.0 + a / mass_) + std::cbrt(1.0 - a / mass_));
    double z2 = std::sqrt(3.0 * (a / mass_) * (a / mass_) + z1 * z1);
    double sign = (a >= 0.0) ? 1.0 : -1.0;
    return mass_ * (3.0 + z2 - sign * std::sqrt((3.0 - z1) * (3.0 + z1 + 2.0 * z2)));
}

Eigen::Vector3d Kerr::geodesicAcceleration(const Eigen::Vector3d& pos,
                                           const Eigen::Vector3d& vel) const {
    const double r = pos[0];
    const double theta = pos[1];
    const double phi = pos[2];

    const double a = toSpinParameter(mass_, spin_);
    if (r <= eventHorizonRadius()) {
        throw std::domain_error("Kerr coordinates undefined at/inside event horizon");
    }

    const double pr = vel[0];
    const double ptheta = vel[1];
    const double pphi = vel[2];

    double pt = solveTimeComponent(mass_, a, r, theta, pr, ptheta, pphi);

    Metric::ChristoffelTensor gamma;
    Metric::FourVector position;
    position << 0.0, r, theta, phi;
    christoffelSymbols(position, gamma);

    Metric::FourVector momentum;
    momentum << pt, pr, ptheta, pphi;

    Eigen::Vector3d acceleration = Eigen::Vector3d::Zero();
    for (int mu = 1; mu < 4; ++mu) {
        double sum = 0.0;
        for (int alpha = 0; alpha < 4; ++alpha) {
            for (int beta = 0; beta < 4; ++beta) {
                sum += gamma[mu](alpha, beta) * momentum[alpha] * momentum[beta];
            }
        }
        acceleration[mu - 1] = -sum;
    }
    return acceleration;
}

void Kerr::setSpin(double spin) {
    spin_ = clampSpin(spin);
}

double Kerr::ergosphereRadius(double theta) const {
    double a = toSpinParameter(mass_, spin_);
    double cosTheta = std::cos(theta);
    double discriminant = mass_ * mass_ - a * a * cosTheta * cosTheta;
    if (discriminant < 0.0) discriminant = 0.0;
    return mass_ + std::sqrt(discriminant);
}

bool Kerr::isInsideErgosphere(double r, double theta) const {
    return r < ergosphereRadius(theta) && r > eventHorizonRadius();
}

double Kerr::frameDraggingOmega(double r, double theta) const {
    double a = toSpinParameter(mass_, spin_);
    MetricCache cache = computeMetricCache(mass_, a, r, theta);
    Metric::MetricTensor g;
    computeMetricTensorComponents(mass_, a, r, theta, cache, g);
    double g_tphi = g(0, 3);
    double g_phiphi = g(3, 3);
    if (std::abs(g_phiphi) < constants::EPSILON) {
        return 0.0;
    }
    return -g_tphi / g_phiphi;
}

Kerr::MetricFunctions Kerr::computeMetricFunctions(double r, double theta) const {
    MetricCache cache = computeMetricCache(mass_, toSpinParameter(mass_, spin_), r, theta);
    MetricFunctions functions{};
    functions.Sigma = cache.Sigma;
    functions.Delta = cache.Delta;
    functions.A = cache.A;
    return functions;
}

Kerr::ChristoffelSymbols Kerr::computeChristoffel(double r, double theta) const {
    const double a = toSpinParameter(mass_, spin_);
    if (r <= eventHorizonRadius()) {
        throw std::domain_error("Kerr Christoffel undefined at/inside horizon");
    }

    MetricCache cache = computeMetricCache(mass_, a, r, theta);

    Metric::MetricTensor g;
    computeMetricTensorComponents(mass_, a, r, theta, cache, g);

    Metric::MetricTensor dg_dr;
    Metric::MetricTensor dg_dtheta;
    computeMetricDerivatives(mass_, a, r, theta, cache, dg_dr, dg_dtheta);

    Metric::MetricTensor gInv = g.inverse();

    auto derivative = [&](int coord, int row, int col) -> double {
        if (coord == 1) return dg_dr(row, col);
        if (coord == 2) return dg_dtheta(row, col);
        return 0.0;
    };

    ChristoffelSymbols result{};
    for (int mu = 0; mu < 4; ++mu) {
        for (int alpha = 0; alpha < 4; ++alpha) {
            for (int beta = 0; beta < 4; ++beta) {
                double sum = 0.0;
                for (int nu = 0; nu < 4; ++nu) {
                    double term = derivative(alpha, beta, nu) +
                                  derivative(beta, alpha, nu) -
                                  derivative(nu, alpha, beta);
                    sum += gInv(mu, nu) * term;
                }
                double gamma = 0.5 * sum;
                switch (mu) {
                    case 0: break; // not stored
                    case 1:
                        if (alpha == 1 && beta == 1) result.gamma_r_rr = gamma;
                        if ((alpha == 1 && beta == 2) || (alpha == 2 && beta == 1))
                            result.gamma_r_rtheta = gamma;
                        if (alpha == 2 && beta == 2) result.gamma_r_theta_theta = gamma;
                        if (alpha == 3 && beta == 3) result.gamma_r_phi_phi = gamma;
                        break;
                    case 2:
                        if (alpha == 1 && beta == 1) result.gamma_theta_rr = gamma;
                        if ((alpha == 1 && beta == 2) || (alpha == 2 && beta == 1))
                            result.gamma_theta_r_theta = gamma;
                        if (alpha == 2 && beta == 2)
                            result.gamma_theta_theta_theta = gamma;
                        if (alpha == 3 && beta == 3) result.gamma_theta_phi_phi = gamma;
                        break;
                    case 3:
                        if ((alpha == 1 && beta == 3) || (alpha == 3 && beta == 1))
                            result.gamma_phi_r_phi = gamma;
                        if ((alpha == 2 && beta == 3) || (alpha == 3 && beta == 2))
                            result.gamma_phi_theta_phi = gamma;
                        break;
                }
            }
        }
    }
    return result;
}

Metric::MetricTensor Kerr::metricTensor(const FourVector& position) const {
    Metric::MetricTensor g;
    double r = position[1];
    double theta = position[2];
    double a = toSpinParameter(mass_, spin_);
    if (r <= eventHorizonRadius()) {
        throw std::domain_error("Kerr metric undefined at/inside event horizon");
    }
    MetricCache cache = computeMetricCache(mass_, a, r, theta);
    computeMetricTensorComponents(mass_, a, r, theta, cache, g);
    return g;
}

Metric::MetricTensor Kerr::inverseMetricTensor(const FourVector& position) const {
    Metric::MetricTensor inv;
    double r = position[1];
    double theta = position[2];
    double a = toSpinParameter(mass_, spin_);
    if (r <= eventHorizonRadius()) {
        throw std::domain_error("Kerr metric undefined at/inside event horizon");
    }

    MetricCache cache = computeMetricCache(mass_, a, r, theta);
    const double Sigma = cache.Sigma;
    const double Delta = cache.Delta;
    const double sinTheta = cache.sinTheta;
    const double sin2 = cache.sin2Theta;

    double factor = 1.0 / (Sigma * Delta);
    double gttNumerator = (cache.A);
    double gtphiNumerator = 2.0 * mass_ * r * a;
    double gphiphiNumerator = Delta - a * a * sin2;

    inv.setZero();
    inv(0, 0) = -gttNumerator * factor;
    inv(0, 3) = -gtphiNumerator * factor;
    inv(3, 0) = inv(0, 3);
    inv(3, 3) = gphiphiNumerator / (Sigma * Delta * sin2);
    inv(1, 1) = Delta / Sigma;
    inv(2, 2) = 1.0 / Sigma;

    return inv;
}

void Kerr::christoffelSymbols(const FourVector& position,
                              ChristoffelTensor& outGamma) const {
    double r = position[1];
    double theta = position[2];
    if (r <= eventHorizonRadius()) {
        throw std::domain_error("Kerr Christoffel undefined at/inside event horizon");
    }

    const double a = toSpinParameter(mass_, spin_);
    MetricCache cache = computeMetricCache(mass_, a, r, theta);

    Metric::MetricTensor g;
    computeMetricTensorComponents(mass_, a, r, theta, cache, g);
    Metric::MetricTensor dg_dr;
    Metric::MetricTensor dg_dtheta;
    computeMetricDerivatives(mass_, a, r, theta, cache, dg_dr, dg_dtheta);
    Metric::MetricTensor gInv = g.inverse();

    auto derivative = [&](int coord, int row, int col) -> double {
        if (coord == 1) return dg_dr(row, col);
        if (coord == 2) return dg_dtheta(row, col);
        return 0.0;
    };

    for (auto& gammaMatrix : outGamma) {
        gammaMatrix.setZero();
    }

    for (int mu = 0; mu < 4; ++mu) {
        for (int alpha = 0; alpha < 4; ++alpha) {
            for (int beta = 0; beta < 4; ++beta) {
                double sum = 0.0;
                for (int nu = 0; nu < 4; ++nu) {
                    double term = derivative(alpha, beta, nu) +
                                  derivative(beta, alpha, nu) -
                                  derivative(nu, alpha, beta);
                    sum += gInv(mu, nu) * term;
                }
                outGamma[mu](alpha, beta) = 0.5 * sum;
            }
        }
    }
}

} // namespace physics
} // namespace cosmic
