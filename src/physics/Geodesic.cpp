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
            rk4Step(state, stepSize_);
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

void GeodesicIntegrator::rk4Step(State& state, double& h) {
    State originalState = state;
    StateDerivative k1, k2, k3, k4;

    // Full step
    k1 = computeDerivative(state);
    State temp = state;
    temp.position += k1.dx * (h * 0.5);
    temp.momentum += k1.dp * (h * 0.5);
    k2 = computeDerivative(temp);
    temp = state;
    temp.position += k2.dx * (h * 0.5);
    temp.momentum += k2.dp * (h * 0.5);
    k3 = computeDerivative(temp);
    temp = state;
    temp.position += k3.dx * h;
    temp.momentum += k3.dp * h;
    k4 = computeDerivative(temp);

    Metric::FourVector pos_h = state.position + (k1.dx + 2.0 * k2.dx + 2.0 * k3.dx + k4.dx) * (h / 6.0);
    Metric::FourVector mom_h = state.momentum + (k1.dp + 2.0 * k2.dp + 2.0 * k3.dp + k4.dp) * (h / 6.0);

    // Two half steps
    double h_half = h * 0.5;
    State state_half = originalState;

    k1 = computeDerivative(state_half);
    temp = state_half;
    temp.position += k1.dx * (h_half * 0.5);
    temp.momentum += k1.dp * (h_half * 0.5);
    k2 = computeDerivative(temp);
    temp = state_half;
    temp.position += k2.dx * (h_half * 0.5);
    temp.momentum += k2.dp * (h_half * 0.5);
    k3 = computeDerivative(temp);
    temp = state_half;
    temp.position += k3.dx * h_half;
    temp.momentum += k3.dp * h_half;
    k4 = computeDerivative(temp);
    state_half.position += (k1.dx + 2.0 * k2.dx + 2.0 * k3.dx + k4.dx) * (h_half / 6.0);
    state_half.momentum += (k1.dp + 2.0 * k2.dp + 2.0 * k3.dp + k4.dp) * (h_half / 6.0);

    k1 = computeDerivative(state_half);
    temp = state_half;
    temp.position += k1.dx * (h_half * 0.5);
    temp.momentum += k1.dp * (h_half * 0.5);
    k2 = computeDerivative(temp);
    temp = state_half;
    temp.position += k2.dx * (h_half * 0.5);
    temp.momentum += k2.dp * (h_half * 0.5);
    k3 = computeDerivative(temp);
    temp = state_half;
    temp.position += k3.dx * h_half;
    temp.momentum += k3.dp * h_half;
    k4 = computeDerivative(temp);

    Metric::FourVector pos_2h_half = state_half.position + (k1.dx + 2.0 * k2.dx + 2.0 * k3.dx + k4.dx) * (h_half / 6.0);
    Metric::FourVector mom_2h_half = state_half.momentum + (k1.dp + 2.0 * k2.dp + 2.0 * k3.dp + k4.dp) * (h_half / 6.0);

    // Error estimation (difference between one full step and two half steps)
    double error = (pos_h - pos_2h_half).norm() + (mom_h - mom_2h_half).norm();

    // Update step size
    h = adaptiveController_.computeNextStep(h, error);

    // Update state using the more accurate two-half-steps result
    state.position = pos_2h_half;
    state.momentum = mom_2h_half;
    state.affineParameter += h;
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
