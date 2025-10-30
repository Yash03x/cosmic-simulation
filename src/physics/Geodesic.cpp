#include "physics/Geodesic.hpp"
#include <cmath>
#include <limits>
#include <stdexcept>

namespace cosmic {
namespace physics {

GeodesicIntegrator::GeodesicIntegrator(const Metric* metric, double stepSize)
    : metric_(metric),
      stepSize_(stepSize),
      escapeRadius_(constants::ESCAPE_RADIUS),
      constraintTolerance_(constants::NULL_CONSTRAINT_TOLERANCE) {
    if (!metric_) {
        throw std::invalid_argument("Metric pointer cannot be null");
    }
    if (stepSize_ <= 0.0) {
        throw std::invalid_argument("Integration step size must be positive");
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

void GeodesicIntegrator::rk4Step(State& state, double h) {
    StateDerivative k1 = computeDerivative(state);

    State temp = state;
    temp.position += k1.dx * (h * 0.5);
    temp.momentum += k1.dp * (h * 0.5);
    StateDerivative k2 = computeDerivative(temp);

    temp = state;
    temp.position += k2.dx * (h * 0.5);
    temp.momentum += k2.dp * (h * 0.5);
    StateDerivative k3 = computeDerivative(temp);

    temp = state;
    temp.position += k3.dx * h;
    temp.momentum += k3.dp * h;
    StateDerivative k4 = computeDerivative(temp);

    state.position += (k1.dx +
                       2.0 * k2.dx +
                       2.0 * k3.dx +
                       k4.dx) * (h / 6.0);

    state.momentum += (k1.dp +
                       2.0 * k2.dp +
                       2.0 * k3.dp +
                       k4.dp) * (h / 6.0);

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
}

} // namespace physics
} // namespace cosmic
