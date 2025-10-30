#include "physics/Geodesic.hpp"
#include <cmath>
#include <limits>

namespace cosmic {
namespace physics {

GeodesicIntegrator::GeodesicIntegrator(const Metric* metric, double stepSize)
    : metric_(metric),
      stepSize_(stepSize),
      escapeRadius_(constants::ESCAPE_RADIUS) {
    if (!metric_) {
        throw std::invalid_argument("Metric pointer cannot be null");
    }
}

GeodesicIntegrator::Result GeodesicIntegrator::integrate(
    State& state, int maxSteps) {

    for (int step = 0; step < maxSteps; ++step) {
        state.stepCount = step;

        // Check validity
        if (!isValidState(state)) {
            return Result::NumericalError;
        }

        double r = state.position[0];

        // Check termination conditions
        if (r < metric_->eventHorizonRadius()) {
            return Result::HitEventHorizon;
        }

        if (r > escapeRadius_) {
            return Result::Escaped;
        }

        // Perform integration step
        rk4Step(state, stepSize_);
    }

    return Result::MaxStepsReached;
}

void GeodesicIntegrator::rk4Step(State& state, double h) {
    // Runge-Kutta 4th order integration
    // k1 = f(t, y)
    // k2 = f(t + h/2, y + k1*h/2)
    // k3 = f(t + h/2, y + k2*h/2)
    // k4 = f(t + h, y + k3*h)
    // y_next = y + (k1 + 2*k2 + 2*k3 + k4) * h/6

    auto k1 = computeDerivative(state);

    State state2 = state;
    state2.position += k1.velocity * (h / 2.0);
    state2.velocity += k1.acceleration * (h / 2.0);
    auto k2 = computeDerivative(state2);

    State state3 = state;
    state3.position += k2.velocity * (h / 2.0);
    state3.velocity += k2.acceleration * (h / 2.0);
    auto k3 = computeDerivative(state3);

    State state4 = state;
    state4.position += k3.velocity * h;
    state4.velocity += k3.acceleration * h;
    auto k4 = computeDerivative(state4);

    // Update state
    state.position += (k1.velocity +
                       k2.velocity * 2.0 +
                       k3.velocity * 2.0 +
                       k4.velocity) * (h / 6.0);

    state.velocity += (k1.acceleration +
                       k2.acceleration * 2.0 +
                       k3.acceleration * 2.0 +
                       k4.acceleration) * (h / 6.0);

    state.affineParameter += h;

    // Clamp radius to prevent numerical issues
    state.position[0] = clampRadius(state.position[0]);

    // Normalize theta to [0, π]
    if (state.position[1] < 0.0) {
        state.position[1] = -state.position[1];
        state.position[2] += constants::PI;
    }
    if (state.position[1] > constants::PI) {
        state.position[1] = 2.0 * constants::PI - state.position[1];
        state.position[2] += constants::PI;
    }

    // Normalize phi to [0, 2π)
    state.position[2] = std::fmod(state.position[2], constants::TWO_PI);
    if (state.position[2] < 0.0) {
        state.position[2] += constants::TWO_PI;
    }
}

GeodesicIntegrator::StateDerivative GeodesicIntegrator::computeDerivative(
    const State& state) const {

    StateDerivative deriv;
    deriv.velocity = state.velocity;
    deriv.acceleration = metric_->geodesicAcceleration(state.position, state.velocity);
    return deriv;
}

bool GeodesicIntegrator::isValidState(const State& state) const {
    // Check for NaN or Inf
    for (int i = 0; i < 3; ++i) {
        if (!std::isfinite(state.position[i]) || !std::isfinite(state.velocity[i])) {
            return false;
        }
    }

    // Check that radius is positive
    if (state.position[0] <= 0.0) {
        return false;
    }

    return true;
}

double GeodesicIntegrator::clampRadius(double r) const {
    // Prevent radius from going too close to zero or too far
    const double minRadius = 0.1 * metric_->eventHorizonRadius();
    const double maxRadius = 2.0 * escapeRadius_;
    return math::clamp(r, minRadius, maxRadius);
}

void GeodesicIntegrator::setStepSize(double stepSize) {
    stepSize_ = math::clamp(stepSize,
                           constants::MIN_STEP_SIZE,
                           constants::MAX_STEP_SIZE);
}

} // namespace physics
} // namespace cosmic
