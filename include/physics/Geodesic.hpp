#pragma once

#include "Metric.hpp"
#include "../utils/Constants.hpp"
#include <Eigen/Dense>

namespace cosmic {
namespace physics {

/**
 * @brief Geodesic integrator for light ray tracing in curved spacetime
 *
 * Uses RK4 (Runge-Kutta 4th order) for numerical integration of geodesic equations
 */
class GeodesicIntegrator {
public:
    /**
     * @brief State of a light ray during integration
     */
    struct State {
        Eigen::Vector3d position;      // (r, θ, φ) in spherical coordinates
        Eigen::Vector3d velocity;      // (dr/dλ, dθ/dλ, dφ/dλ)
        double affineParameter;        // Affine parameter λ
        int stepCount;                 // Number of integration steps

        State() : position(Eigen::Vector3d::Zero()),
                  velocity(Eigen::Vector3d::Zero()),
                  affineParameter(0.0),
                  stepCount(0) {}
    };

    /**
     * @brief Result of geodesic integration
     */
    enum class Result {
        Escaped,           // Ray escaped to infinity (r > escape radius)
        HitEventHorizon,   // Ray crossed event horizon
        MaxStepsReached,   // Maximum integration steps exceeded
        NumericalError     // Numerical instability detected (NaN, Inf)
    };

    /**
     * @brief Construct geodesic integrator
     * @param metric Pointer to spacetime metric (not owned)
     * @param stepSize Integration step size (default: 0.1)
     */
    explicit GeodesicIntegrator(const Metric* metric,
                               double stepSize = constants::DEFAULT_STEP_SIZE);

    /**
     * @brief Integrate geodesic equations from initial state
     *
     * Performs backward ray tracing from camera through pixel
     *
     * @param state Initial state (position and velocity), modified during integration
     * @param maxSteps Maximum number of integration steps
     * @return Result of integration
     */
    Result integrate(State& state, int maxSteps = constants::MAX_INTEGRATION_STEPS);

    /**
     * @brief Perform single RK4 integration step
     * @param state Current state, will be updated
     * @param h Step size
     */
    void rk4Step(State& state, double h);

    /**
     * @brief Set integration step size
     * @param stepSize New step size
     */
    void setStepSize(double stepSize);

    /**
     * @brief Get current step size
     * @return Current step size
     */
    double getStepSize() const { return stepSize_; }

    /**
     * @brief Set escape radius (ray escapes if r > this)
     * @param radius Escape radius
     */
    void setEscapeRadius(double radius) { escapeRadius_ = radius; }

    /**
     * @brief Get escape radius
     * @return Current escape radius
     */
    double getEscapeRadius() const { return escapeRadius_; }

private:
    const Metric* metric_;      // Spacetime metric (not owned)
    double stepSize_;           // Integration step size
    double escapeRadius_;       // Radius at which ray is considered escaped

    /**
     * @brief Compute time derivative of state
     * @param state Current state
     * @return Time derivatives (velocity, acceleration)
     */
    struct StateDerivative {
        Eigen::Vector3d velocity;
        Eigen::Vector3d acceleration;
    };

    StateDerivative computeDerivative(const State& state) const;

    /**
     * @brief Check if state contains valid numbers (no NaN or Inf)
     * @param state State to check
     * @return True if state is valid
     */
    bool isValidState(const State& state) const;

    /**
     * @brief Clamp radial coordinate to prevent numerical issues
     * @param r Radial coordinate
     * @return Clamped value
     */
    double clampRadius(double r) const;
};

} // namespace physics
} // namespace cosmic
