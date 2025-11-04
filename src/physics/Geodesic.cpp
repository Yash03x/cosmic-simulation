#include "physics/Geodesic.hpp"
#include <cmath>
#include <limits>
#include <stdexcept>

namespace cosmic {
namespace physics {

GeodesicIntegrator::GeodesicIntegrator(const Metric* metric, double initialStepSize)
    : metric_(metric),
      stepSize_(initialStepSize),
      adaptiveController_(constants::ADAPTIVE_TOLERANCE,
                          constants::ADAPTIVE_SAFETY_FACTOR,
                          constants::MIN_STEP_SIZE,
                          constants::MAX_STEP_SIZE),
      escapeRadius_(constants::ESCAPE_RADIUS),
      constraintTolerance_(constants::NULL_CONSTRAINT_TOLERANCE) {
    if (!metric_) {
        throw std::invalid_argument("Metric pointer cannot be null");
    }
    if (stepSize_ <= 0.0) {
        throw std::invalid_argument("Initial step size must be positive");
    }
}

GeodesicIntegrator::Result GeodesicIntegrator::integrate(
    State& state, int maxSteps) {

    if (!isValidState(state)) {
        throw std::invalid_argument("Initial geodesic state is invalid");
    }

    state.energy = computeEnergy(state.position, state.momentum);
    state.angularMomentum = computeAngularMomentum(state.position, state.momentum);
    enforceConservedQuantities(state);
    state.constraintError = std::abs(computeNullConstraint(state.position, state.momentum));

    for (int step = 0; step < maxSteps; ++step) {
        state.stepCount = step;

        const double r = state.position[1];
        if (!std::isfinite(r) || r <= 0.0) {
            return Result::NumericalError;
        }

        if (r <= metric_->eventHorizonRadius()) {
            return Result::HitEventHorizon;
        }

        if (r >= escapeRadius_) {
            return Result::Escaped;
        }

        try {
            rkdp5Step(state, stepSize_);
        } catch (const std::domain_error&) {
            return Result::HitEventHorizon;
        }

        enforceConservedQuantities(state);

        state.constraintError = std::abs(computeNullConstraint(state.position, state.momentum));
        if (!std::isfinite(state.constraintError)) {
            return Result::NumericalError;
        }

        if (state.constraintError > constraintTolerance_) {
            return Result::ConstraintViolation;
        }
    }

    return Result::MaxStepsReached;
}

void GeodesicIntegrator::rkdp5Step(State& state, double& h) {
    // Dormand-Prince 5(4) method coefficients
    constexpr double c2 = 1.0/5.0, c3 = 3.0/10.0, c4 = 4.0/5.0, c5 = 8.0/9.0, c6 = 1.0, c7 = 1.0;

    constexpr double a21 = 1.0/5.0;
    constexpr double a31 = 3.0/40.0, a32 = 9.0/40.0;
    constexpr double a41 = 44.0/45.0, a42 = -56.0/15.0, a43 = 32.0/9.0;
    constexpr double a51 = 19372.0/6561.0, a52 = -25360.0/2187.0, a53 = 64448.0/6561.0, a54 = -212.0/729.0;
    constexpr double a61 = 9017.0/3168.0, a62 = -355.0/33.0, a63 = 46732.0/5247.0, a64 = 49.0/176.0, a65 = -5103.0/18656.0;
    constexpr double a71 = 35.0/384.0, a73 = 500.0/1113.0, a74 = 125.0/192.0, a75 = -2187.0/6784.0, a76 = 11.0/84.0;

    // 5th order solution coefficients
    constexpr double b1 = 35.0/384.0, b3 = 500.0/1113.0, b4 = 125.0/192.0, b5 = -2187.0/6784.0, b6 = 11.0/84.0;

    // Error estimation coefficients (difference between 5th and 4th order solutions)
    constexpr double e1 = 71.0/57600.0, e3 = -71.0/16695.0, e4 = 71.0/1920.0, e5 = -17253.0/339200.0, e6 = 22.0/525.0, e7 = -1.0/40.0;

    State temp = state;
    StateDerivative k1, k2, k3, k4, k5, k6, k7;

    k1 = computeDerivative(state);

    temp.position = state.position + k1.dx * h * a21;
    temp.momentum = state.momentum + k1.dp * h * a21;
    k2 = computeDerivative(temp);

    temp.position = state.position + (k1.dx * a31 + k2.dx * a32) * h;
    temp.momentum = state.momentum + (k1.dp * a31 + k2.dp * a32) * h;
    k3 = computeDerivative(temp);

    temp.position = state.position + (k1.dx * a41 + k2.dx * a42 + k3.dx * a43) * h;
    temp.momentum = state.momentum + (k1.dp * a41 + k2.dp * a42 + k3.dp * a43) * h;
    k4 = computeDerivative(temp);

    temp.position = state.position + (k1.dx * a51 + k2.dx * a52 + k3.dx * a53 + k4.dx * a54) * h;
    temp.momentum = state.momentum + (k1.dp * a51 + k2.dp * a52 + k3.dp * a53 + k4.dp * a54) * h;
    k5 = computeDerivative(temp);

    temp.position = state.position + (k1.dx * a61 + k2.dx * a62 + k3.dx * a63 + k4.dx * a64 + k5.dx * a65) * h;
    temp.momentum = state.momentum + (k1.dp * a61 + k2.dp * a62 + k3.dp * a63 + k4.dp * a64 + k5.dp * a65) * h;
    k6 = computeDerivative(temp);

    // The 7th stage calculation uses the 5th order coefficients (a7_i = b_i)
    temp.position = state.position + (k1.dx * a71 + k3.dx * a73 + k4.dx * a74 + k5.dx * a75 + k6.dx * a76) * h;
    temp.momentum = state.momentum + (k1.dp * a71 + k3.dp * a73 + k4.dp * a74 + k5.dp * a75 + k6.dp * a76) * h;
    k7 = computeDerivative(temp);

    // Update state with the 5th order solution
    state.position += (k1.dx * b1 + k3.dx * b3 + k4.dx * b4 + k5.dx * b5 + k6.dx * b6) * h;
    state.momentum += (k1.dp * b1 + k3.dp * b3 + k4.dp * b4 + k5.dp * b5 + k6.dp * b6) * h;

    // Estimate error
    Metric::FourVector posError = (k1.dx * e1 + k3.dx * e3 + k4.dx * e4 + k5.dx * e5 + k6.dx * e6 + k7.dx * e7) * h;
    Metric::FourVector momError = (k1.dp * e1 + k3.dp * e3 + k4.dp * e4 + k5.dp * e5 + k6.dp * e6 + k7.dp * e7) * h;
    double error = posError.norm() + momError.norm();

    // Store the previous step size
    double h_old = h;

    // Update step size
    h = adaptiveController_.computeNextStep(h, error);

    // Update affine parameter with the actual step size taken
    state.affineParameter += h_old;
}

void GeodesicIntegrator::rk4Step(State& state, double& h) {
    rkdp5Step(state, h);
}

void GeodesicIntegrator::setStepSize(double stepSize) {
    stepSize_ = math::clamp(stepSize,
                           constants::MIN_STEP_SIZE,
                           constants::MAX_STEP_SIZE);
}

GeodesicIntegrator::StateDerivative GeodesicIntegrator::computeDerivative(
    const State& state) const {

    StateDerivative deriv;
    deriv.dx = state.momentum;
    deriv.dp.setZero();

    Metric::ChristoffelTensor gamma;
    metric_->christoffelSymbols(state.position, gamma);

    for (int mu = 0; mu < 4; ++mu) {
        double sum = 0.0;
        for (int alpha = 0; alpha < 4; ++alpha) {
            for (int beta = 0; beta < 4; ++beta) {
                sum += gamma[mu](alpha, beta) *
                       state.momentum[alpha] *
                       state.momentum[beta];
            }
        }
        deriv.dp[mu] = -sum;
    }

    return deriv;
}

bool GeodesicIntegrator::isValidState(const State& state) const {
    for (int i = 0; i < 4; ++i) {
        if (!std::isfinite(state.position[i]) ||
            !std::isfinite(state.momentum[i])) {
            return false;
        }
    }

    if (!std::isfinite(state.affineParameter)) {
        return false;
    }

    if (state.position[1] <= 0.0) {
        return false;
    }

    return true;
}

double GeodesicIntegrator::computeNullConstraint(
    const Metric::FourVector& position,
    const Metric::FourVector& momentum) const {

    Metric::MetricTensor g = metric_->metricTensor(position);
    return momentum.transpose() * g * momentum;
}

double GeodesicIntegrator::computeEnergy(
    const Metric::FourVector& position,
    const Metric::FourVector& momentum) const {

    Metric::MetricTensor g = metric_->metricTensor(position);

    double energy = 0.0;
    for (int mu = 0; mu < 4; ++mu) {
        energy += g(0, mu) * momentum[mu];
    }
    return -energy;
}

double GeodesicIntegrator::computeAngularMomentum(
    const Metric::FourVector& position,
    const Metric::FourVector& momentum) const {

    Metric::MetricTensor g = metric_->metricTensor(position);

    double angularMomentum = 0.0;
    for (int mu = 0; mu < 4; ++mu) {
        angularMomentum += g(3, mu) * momentum[mu];
    }
    return angularMomentum;
}

void GeodesicIntegrator::enforceConservedQuantities(State& state) const {
    Metric::MetricTensor g = metric_->metricTensor(state.position);

    const double g_tt = g(0, 0);
    const double g_tphi = g(0, 3);
    const double g_rr = g(1, 1);
    const double g_thetatheta = g(2, 2);
    const double g_phiphi = g(3, 3);

    const double det = g_tt * g_phiphi - g_tphi * g_tphi;
    if (std::abs(det) < constants::EPSILON) {
        throw std::domain_error("Degenerate metric encountered while enforcing invariants");
    }

    const double E = state.energy;
    const double L = state.angularMomentum;

    const double pt = (-E * g_phiphi - g_tphi * L) / det;
    const double pphi = (L * g_tt + g_tphi * E) / det;

    state.momentum[0] = pt;
    state.momentum[3] = pphi;

    const double spatialTerm = g_rr * state.momentum[1] * state.momentum[1] +
                               g_thetatheta * state.momentum[2] * state.momentum[2];

    const double temporalTerm = g_tt * pt * pt +
                                2.0 * g_tphi * pt * pphi +
                                g_phiphi * pphi * pphi;

    const double target = -temporalTerm;

    if (spatialTerm > constants::EPSILON && target > constants::EPSILON) {
        const double scale = std::sqrt(target / spatialTerm);
        state.momentum[1] *= scale;
        state.momentum[2] *= scale;
    }

    state.energy = E;
    state.angularMomentum = L;
}

} // namespace physics
} // namespace cosmic
