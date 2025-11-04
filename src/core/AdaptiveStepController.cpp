#include "core/AdaptiveStepController.hpp"
#include <cmath>
#include <algorithm>

namespace cosmic {
namespace core {

AdaptiveStepController::AdaptiveStepController(double tolerance, double safetyFactor, double minStep, double maxStep)
    : tolerance_(tolerance),
      safetyFactor_(safetyFactor),
      minStep_(minStep),
      maxStep_(maxStep) {}

double AdaptiveStepController::computeNextStep(double currentStep, double error) const {
    if (error == 0.0) {
        return std::min(maxStep_, currentStep * 2.0);
    }

    double stepRatio = safetyFactor_ * std::pow(tolerance_ / error, 0.2);
    double nextStep = currentStep * stepRatio;

    return std::max(minStep_, std::min(maxStep_, nextStep));
}

} // namespace core
} // namespace cosmic
