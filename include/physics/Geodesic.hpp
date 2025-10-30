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
        Metric::FourVector position;   // (t, r, θ, φ)
        Metric::FourVector momentum;   // p^μ = dx^μ/dλ
        double affineParameter;        // Affine parameter λ
        int stepCount;                 // Number of integration steps
        double energy;                 // Conserved energy (-g_{tμ} p^μ)
        double angularMomentum;        // Conserved L_z = g_{φμ} p^μ
        double constraintError;        // |g_{μν} p^μ p^ν|

        State()
            : position(Metric::FourVector::Zero()),
              momentum(Metric::FourVector::Zero()),
              affineParameter(0.0),
              stepCount(0),
              energy(0.0),
              angularMomentum(0.0),
              constraintError(0.0) {}
    };

    /**
     * @brief Result of geodesic integration
     */
    enum class Result {
        Escaped,           // Ray escaped to infinity (r > escape radius)
        HitEventHorizon,   // Ray crossed event horizon
        MaxStepsReached,   // Maximum integration steps exceeded
        NumericalError,    // Numerical instability detected (NaN, Inf)
        ConstraintViolation // Null constraint drift beyond tolerance
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
        Metric::FourVector dx;
        Metric::FourVector dp;
    };

    StateDerivative computeDerivative(const State& state) const;

    /**
     * @brief Check if state contains valid numbers (no NaN or Inf)
     * @param state State to check
     * @return True if state is valid
     */
    bool isValidState(const State& state) const;

    /**
     * @brief Evaluate null constraint g_{μν} p^μ p^ν at a point
     */
    double computeNullConstraint(const Metric::FourVector& position,
                                 const Metric::FourVector& momentum) const;

    double computeEnergy(const Metric::FourVector& position,
                         const Metric::FourVector& momentum) const;

    double computeAngularMomentum(const Metric::FourVector& position,
                                  const Metric::FourVector& momentum) const;

    void enforceConservedQuantities(State& state) const;

    double constraintTolerance_;
};

} // namespace physics
} // namespace cosmic
