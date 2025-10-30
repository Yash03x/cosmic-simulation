#include "physics/Schwarzschild.hpp"
#include "physics/Geodesic.hpp"
#include "utils/Constants.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>

using cosmic::physics::GeodesicIntegrator;
using cosmic::physics::Schwarzschild;

namespace {

struct TestResult {
    bool passed;
    std::string message;
};

GeodesicIntegrator::State makePhotonSphereState(const Schwarzschild& metric) {
    GeodesicIntegrator::State state;
    const double mass = metric.getMass();
    const double r = 3.0 * mass;

    state.position[0] = 0.0;                            // t
    state.position[1] = r;                              // r
    state.position[2] = cosmic::constants::HALF_PI;     // θ = π/2
    state.position[3] = 0.0;                            // φ

    const double energy = 1.0;
    const double lapse = 1.0 - (2.0 * mass / r);
    state.momentum[0] = energy / lapse;                 // dt/dλ
    state.momentum[1] = 0.0;                            // dr/dλ
    state.momentum[2] = 0.0;                            // dθ/dλ
    const double angularMomentum = std::sqrt(27.0) * mass * energy;
    state.momentum[3] = angularMomentum / (r * r);      // dφ/dλ

    return state;
}

GeodesicIntegrator::State makeAsymptoticState(const Schwarzschild& metric,
                                              double radius,
                                              double impactParameter,
                                              double signDr) {
    GeodesicIntegrator::State state;

    state.position[0] = 0.0;
    state.position[1] = radius;
    state.position[2] = cosmic::constants::HALF_PI;
    state.position[3] = 0.0;

    const double mass = metric.getMass();
    const double energy = 1.0;
    const double lapse = 1.0 - (2.0 * mass / radius);

    state.momentum[0] = energy / lapse;
    state.momentum[2] = 0.0;

    const double L = impactParameter * energy;
    state.momentum[3] = L / (radius * radius * std::sin(state.position[2]) * std::sin(state.position[2]));

    cosmic::physics::Metric::MetricTensor g = metric.metricTensor(state.position);
    const double g_tt = g(0, 0);
    const double g_rr = g(1, 1);
    const double g_phiphi = g(3, 3);

    const double radialSquared =
        -(g_tt * state.momentum[0] * state.momentum[0] +
          g_phiphi * state.momentum[3] * state.momentum[3]) / g_rr;

    state.momentum[1] = signDr * std::sqrt(std::max(radialSquared, 0.0));

    return state;
}

TestResult photonSphereStability() {
    Schwarzschild metric(1.0);
    GeodesicIntegrator integrator(&metric, 0.0025);
    integrator.setEscapeRadius(200.0);

    auto state = makePhotonSphereState(metric);
    const auto result = integrator.integrate(state, 4000);

    const double expectedRadius = 3.0 * metric.getMass();
    const double radiusError = std::abs(state.position[1] - expectedRadius);

    if (result != GeodesicIntegrator::Result::MaxStepsReached) {
        return {false, "Photon sphere integration did not exhaust steps"};
    }
    if (radiusError > 1e-4) {
        return {false, "Photon sphere radius drift exceeded tolerance"};
    }
    if (state.constraintError > cosmic::constants::NULL_CONSTRAINT_TOLERANCE) {
        return {false, "Null constraint violated during photon sphere test"};
    }
    return {true, "Photon sphere remains stable at r = 3M"};
}

TestResult impactParameterThreshold() {
    Schwarzschild metric(1.0);
    GeodesicIntegrator integrator(&metric, 0.002);
    integrator.setEscapeRadius(400.0);

    const double bCritical = std::sqrt(27.0) * metric.getMass();
    const double radius = 200.0;

    auto inboundState = makeAsymptoticState(metric, radius, 0.99 * bCritical, -1.0);
    auto resultNear = integrator.integrate(inboundState, 200000);
    if (resultNear != GeodesicIntegrator::Result::HitEventHorizon) {
        return {false, "Impact parameter just below critical should hit horizon (result = " +
                       std::to_string(static_cast<int>(resultNear)) +
                       ", r_final = " + std::to_string(inboundState.position[1]) + ")"};
    }

    GeodesicIntegrator integratorEscape(&metric, 0.002);
    integratorEscape.setEscapeRadius(400.0);
    auto outboundState = makeAsymptoticState(metric, radius, 1.01 * bCritical, -1.0);
    auto resultFar = integratorEscape.integrate(outboundState, 200000);
    if (resultFar != GeodesicIntegrator::Result::Escaped &&
        resultFar != GeodesicIntegrator::Result::MaxStepsReached) {
        return {false, "Impact parameter above critical should avoid horizon (result = " +
                       std::to_string(static_cast<int>(resultFar)) + ")"};
    }

    return {true, "Impact parameter threshold matches analytical value"};
}

} // namespace

int main() {
    auto photonSphere = photonSphereStability();
    auto impactCheck = impactParameterThreshold();

    if (!photonSphere.passed) {
        std::cerr << "Photon sphere test failed: " << photonSphere.message << "\n";
    }
    if (!impactCheck.passed) {
        std::cerr << "Impact parameter test failed: " << impactCheck.message << "\n";
    }

    if (photonSphere.passed && impactCheck.passed) {
        std::cout << "All geodesic validation tests passed.\n";
        return 0;
    }

    return 1;
}
