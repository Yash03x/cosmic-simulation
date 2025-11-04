#pragma once

#include "Metric.hpp"
#include "../utils/Constants.hpp"
#include "../core/AdaptiveStepController.hpp"
#include <Eigen/Dense>

namespace cosmic {
namespace physics {

class GeodesicIntegrator {
public:
    struct State {
        Metric::FourVector position;
        Metric::FourVector momentum;
        double affineParameter;
        int stepCount;
        double energy;
        double angularMomentum;
        double constraintError;

        State()
            : position(Metric::FourVector::Zero()),
              momentum(Metric::FourVector::Zero()),
              affineParameter(0.0),
              stepCount(0),
              energy(0.0),
              angularMomentum(0.0),
              constraintError(0.0) {}
    };

    enum class Result {
        Escaped,
        HitEventHorizon,
        MaxStepsReached,
        NumericalError,
        ConstraintViolation
    };

    explicit GeodesicIntegrator(const Metric* metric,
                               double initialStepSize = constants::DEFAULT_STEP_SIZE);

    Result integrate(State& state, int maxSteps = constants::MAX_INTEGRATION_STEPS);

    void rkdp5Step(State& state, double& h);

    void rk4Step(State& state, double& h);

    void setStepSize(double stepSize);
    double getStepSize() const { return stepSize_; }

    void setEscapeRadius(double radius) { escapeRadius_ = radius; }
    double getEscapeRadius() const { return escapeRadius_; }

private:
    const Metric* metric_;
    double stepSize_;
    core::AdaptiveStepController adaptiveController_;
    double escapeRadius_;

    struct StateDerivative {
        Metric::FourVector dx;
        Metric::FourVector dp;
    };

    StateDerivative computeDerivative(const State& state) const;
    bool isValidState(const State& state) const;
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
