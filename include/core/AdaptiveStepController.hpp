#ifndef COSMIC_CORE_ADAPTIVESTEPCONTROLLER_HPP
#define COSMIC_CORE_ADAPTIVESTEPCONTROLLER_HPP

namespace cosmic {
namespace core {

class AdaptiveStepController {
public:
    AdaptiveStepController(double tolerance, double safetyFactor, double minStep, double maxStep);

    double computeNextStep(double currentStep, double error) const;

private:
    double tolerance_;
    double safetyFactor_;
    double minStep_;
    double maxStep_;
};

} // namespace core
} // namespace cosmic

#endif // COSMIC_CORE_ADAPTIVESTEPCONTROLLER_HPP
