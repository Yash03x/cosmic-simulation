#include "physics/Polarization.hpp"
#include "physics/Kerr.hpp"
#include <cmath>
#include <stdexcept>

namespace cosmic {
namespace physics {

PolarizationTransport::PolarizationTransport(const Metric* metric)
    : metric_(metric) {
    if (!metric_) {
        throw std::invalid_argument("Metric pointer cannot be null");
    }
}

void PolarizationTransport::transportStep(PolarizationState& state,
                                         double stepSize) {
    // RK4 integration of parallel transport equation
    // dπ^μ/dλ = -Γ^μ_αβ k^α π^β

    auto computeDerivative = [this](const Metric::FourVector& pos,
                                   const Metric::FourVector& k,
                                   const Metric::FourVector& pi) {
        return computePolarizationDerivative(pos, k, pi);
    };

    // Current values
    Metric::FourVector pos = state.position;
    Metric::FourVector k = state.photonMomentum;
    Metric::FourVector pi = state.polarization;

    // RK4 stages for polarization
    Metric::FourVector k1_pi = computeDerivative(pos, k, pi);

    Metric::FourVector pi_temp = pi + k1_pi * (stepSize / 2.0);
    Metric::FourVector k2_pi = computeDerivative(pos, k, pi_temp);

    pi_temp = pi + k2_pi * (stepSize / 2.0);
    Metric::FourVector k3_pi = computeDerivative(pos, k, pi_temp);

    pi_temp = pi + k3_pi * stepSize;
    Metric::FourVector k4_pi = computeDerivative(pos, k, pi_temp);

    // Update polarization
    state.polarization += (k1_pi + k2_pi * 2.0 + k3_pi * 2.0 + k4_pi) * (stepSize / 6.0);

    // Enforce orthogonality constraints: π · k = 0
    orthogonalize(state.polarization, state.photonMomentum, state.position);

    // Update affine parameter
    state.affineParameter += stepSize;
}

void PolarizationTransport::computeStokesParameters(
    const Metric::FourVector& polarization,
    const Metric::FourVector& photonMomentum,
    const Metric::FourVector& observerVelocity,
    double& I, double& Q, double& U, double& V) const {

    // Construct observer tetrad
    std::array<Metric::FourVector, 4> tetrad =
        constructTetrad(Metric::FourVector::Zero(), observerVelocity);

    // Project polarization to observer frame
    Eigen::Vector3d pi_obs = projectToObserverFrame(polarization, tetrad);

    // Compute Stokes parameters
    // For linear polarization in x-y plane:
    double px = pi_obs[0];
    double py = pi_obs[1];

    I = px * px + py * py;  // Total intensity
    Q = px * px - py * py;  // Linear polarization (0°-90°)
    U = 2.0 * px * py;      // Linear polarization (45°-135°)
    V = 0.0;                // Circular (requires full treatment)

    // Normalize to I
    if (I > constants::EPSILON) {
        Q /= I;
        U /= I;
        V /= I;
    }
}

std::array<Metric::FourVector, 4> PolarizationTransport::constructTetrad(
    const Metric::FourVector& position,
    const Metric::FourVector& velocity) const {

    std::array<Metric::FourVector, 4> tetrad;

    // e_0 = four-velocity (timelike)
    tetrad[0] = velocity;

    // e_1 = radial direction
    tetrad[1] = Metric::FourVector::Zero();
    tetrad[1][1] = 1.0;
    orthogonalize(tetrad[1], tetrad[0], position);

    // e_2 = theta direction
    tetrad[2] = Metric::FourVector::Zero();
    tetrad[2][2] = 1.0;
    orthogonalize(tetrad[2], tetrad[0], position);
    orthogonalize(tetrad[2], tetrad[1], position);

    // e_3 = phi direction
    tetrad[3] = Metric::FourVector::Zero();
    tetrad[3][3] = 1.0;
    orthogonalize(tetrad[3], tetrad[0], position);
    orthogonalize(tetrad[3], tetrad[1], position);
    orthogonalize(tetrad[3], tetrad[2], position);

    // Normalize
    for (int i = 0; i < 4; ++i) {
        double norm = innerProduct(tetrad[i], tetrad[i], position);
        if (std::abs(norm) > constants::EPSILON) {
            double sign = (i == 0) ? -1.0 : 1.0; // Timelike is negative norm
            tetrad[i] /= std::sqrt(sign * norm);
        }
    }

    return tetrad;
}

Eigen::Vector3d PolarizationTransport::projectToObserverFrame(
    const Metric::FourVector& polarization,
    const std::array<Metric::FourVector, 4>& tetrad) const {

    Eigen::Vector3d result;

    // Project onto spatial tetrad vectors
    Metric::FourVector pos = Metric::FourVector::Zero();
    result[0] = innerProduct(polarization, tetrad[1], pos);
    result[1] = innerProduct(polarization, tetrad[2], pos);
    result[2] = innerProduct(polarization, tetrad[3], pos);

    return result;
}

double PolarizationTransport::polarizationRotationAngle(
    const PolarizationState& initialState,
    const PolarizationState& finalState) const {

    // Construct observer tetrads at initial and final positions
    Metric::FourVector u_obs = Metric::FourVector::Zero();
    u_obs[0] = 1.0;

    auto tetrad_init = constructTetrad(initialState.position, u_obs);
    auto tetrad_final = constructTetrad(finalState.position, u_obs);

    // Project polarizations
    Eigen::Vector3d pi_init = projectToObserverFrame(initialState.polarization,
                                                     tetrad_init);
    Eigen::Vector3d pi_final = projectToObserverFrame(finalState.polarization,
                                                      tetrad_final);

    // Compute angle between projections
    double angle_init = std::atan2(pi_init[1], pi_init[0]);
    double angle_final = std::atan2(pi_final[1], pi_final[0]);

    return angle_final - angle_init;
}

PolarizationTransport::WalkerPenroseConstant
    PolarizationTransport::computeWalkerPenrose(
        const PolarizationState& state) const {

    WalkerPenroseConstant wpc;

    // Check if this is Kerr metric
    const Kerr* kerr = dynamic_cast<const Kerr*>(metric_);
    if (!kerr) {
        wpc.isConserved = false;
        wpc.conservationError = 0.0;
        return wpc;
    }

    // Walker-Penrose constant: κ = π_μ ξ^μ + i π_μ η^μ
    // where ξ^μ is timelike Killing vector, η^μ is rotational Killing vector

    // For now, return placeholder
    wpc.kappa = std::complex<double>(0.0, 0.0);
    wpc.isConserved = true;
    wpc.conservationError = 0.0;

    return wpc;
}

Metric::FourVector PolarizationTransport::initializePolarization(
    const Metric::FourVector& photonMomentum,
    const Metric::FourVector& position,
    double polarizationAngle) const {

    // Construct polarization vector orthogonal to photon momentum
    Metric::FourVector polarization = Metric::FourVector::Zero();

    // Start with a spatial direction
    polarization[1] = std::cos(polarizationAngle);
    polarization[2] = std::sin(polarizationAngle);

    // Orthogonalize against photon momentum
    orthogonalize(polarization, photonMomentum, position);

    // Normalize to null (π · π = 0 for photon)
    double norm_sq = innerProduct(polarization, polarization, position);
    if (norm_sq > constants::EPSILON) {
        polarization /= std::sqrt(std::abs(norm_sq));
    }

    return polarization;
}

double PolarizationTransport::faradayRotation(double electronDensity,
                                             double magneticField,
                                             double pathLength,
                                             double wavelength) const {
    // Rotation measure: RM = (e^3 / 2πm_e^2 c^4) ∫ n_e B·dl
    // In cgs units: RM ≈ 2.6e-13 * ∫ n_e B·dl [rad/cm^2]

    constexpr double RM_CONSTANT = 2.6e-13; // rad cm^2 / (cm^-3 Gauss cm)

    double RM = RM_CONSTANT * electronDensity * magneticField * pathLength;

    // Rotation angle = RM * λ²
    double lambda_sq = wavelength * wavelength;
    return RM * lambda_sq;
}

double PolarizationTransport::diskPolarizationFraction(
    double magneticFieldStrength,
    double opticalDepth,
    double temperature) const {

    // Synchrotron intrinsic polarization: ~70% for power-law electrons
    double intrinsic = 0.7;

    // Depolarization from Faraday rotation in optically thick regions
    // P ≈ P_0 / (1 + τ_F)  where τ_F ∝ τ
    double faraday_suppression = 1.0 / (1.0 + 0.1 * opticalDepth);

    // Scattering depolarization (Thomson scattering is unpolarized)
    // P ≈ P_0 * exp(-τ)
    double scattering_suppression = std::exp(-opticalDepth);

    return intrinsic * faraday_suppression * scattering_suppression;
}

Metric::FourVector PolarizationTransport::computePolarizationDerivative(
    const Metric::FourVector& position,
    const Metric::FourVector& photonMomentum,
    const Metric::FourVector& polarization) const {

    // Parallel transport equation: Dπ^μ/dλ = -Γ^μ_αβ k^α π^β

    Metric::ChristoffelTensor gamma;
    metric_->christoffelSymbols(position, gamma);

    Metric::FourVector derivative = Metric::FourVector::Zero();

    for (int mu = 0; mu < 4; ++mu) {
        double sum = 0.0;
        for (int alpha = 0; alpha < 4; ++alpha) {
            for (int beta = 0; beta < 4; ++beta) {
                sum += gamma[mu](alpha, beta) *
                       photonMomentum[alpha] *
                       polarization[beta];
            }
        }
        derivative[mu] = -sum;
    }

    return derivative;
}

void PolarizationTransport::orthogonalize(Metric::FourVector& vec,
                                         const Metric::FourVector& against,
                                         const Metric::FourVector& position) const {
    // Gram-Schmidt: v' = v - (v·u / u·u) u

    double v_dot_u = innerProduct(vec, against, position);
    double u_dot_u = innerProduct(against, against, position);

    if (std::abs(u_dot_u) > constants::EPSILON) {
        vec -= against * (v_dot_u / u_dot_u);
    }
}

double PolarizationTransport::innerProduct(const Metric::FourVector& a,
                                          const Metric::FourVector& b,
                                          const Metric::FourVector& position) const {
    Metric::MetricTensor g = metric_->metricTensor(position);

    double result = 0.0;
    for (int mu = 0; mu < 4; ++mu) {
        for (int nu = 0; nu < 4; ++nu) {
            result += g(mu, nu) * a[mu] * b[nu];
        }
    }

    return result;
}

// SynchrotronPolarization implementation

SynchrotronPolarization::SynchrotronPolarization(const Metric* metric)
    : metric_(metric) {
    if (!metric_) {
        throw std::invalid_argument("Metric pointer cannot be null");
    }
}

SynchrotronPolarization::EmissionProperties
    SynchrotronPolarization::computeEmission(
        const Metric::FourVector& position,
        const Eigen::Vector3d& magneticField,
        double electronEnergy,
        double viewingAngle,
        double frequency) const {

    EmissionProperties props;

    // Synchrotron critical frequency: ν_c = (3/2) γ² (eB/2πm_e c) sin(α)
    // where α is pitch angle
    constexpr double GYRO_FREQ_CONSTANT = 2.8e6; // Hz/Gauss for electron

    double B_mag = magneticField.norm();
    double gamma = electronEnergy;
    double sin_alpha = std::sin(viewingAngle);

    double nu_c = 1.5 * gamma * gamma * GYRO_FREQ_CONSTANT * B_mag * sin_alpha;

    // Synchrotron spectral shape
    double x = frequency / (nu_c + constants::EPSILON);

    // F(x) ≈ x^(1/3) for x << 1, exp(-x) for x >> 1
    double spectrum;
    if (x < 1.0) {
        spectrum = std::pow(x, 1.0/3.0);
    } else {
        spectrum = std::sqrt(x) * std::exp(-x);
    }

    props.totalIntensity = spectrum;

    // Synchrotron polarization: ~70% perpendicular to B field
    props.polarizationFraction = 0.7 * (1.0 - 1.0 / (3.0 * gamma * gamma));

    // Polarization angle perpendicular to projected B field
    props.polarizationAngle = std::atan2(magneticField[1], magneticField[0]) +
                              constants::HALF_PI;

    // Spectral index: α ≈ 0.5 to 1.0 for synchrotron
    props.spectralIndex = 0.7;

    props.magneticField = magneticField;

    return props;
}

std::vector<std::vector<std::array<double, 4>>>
    SynchrotronPolarization::polarizationMap(double inclination,
                                            int imageWidth,
                                            int imageHeight) const {

    std::vector<std::vector<std::array<double, 4>>> map;
    map.resize(imageHeight);
    for (int i = 0; i < imageHeight; ++i) {
        map[i].resize(imageWidth);
        for (int j = 0; j < imageWidth; ++j) {
            // Placeholder: would require full ray tracing
            map[i][j] = {1.0, 0.0, 0.0, 0.0}; // (I, Q, U, V)
        }
    }

    return map;
}

} // namespace physics
} // namespace cosmic
