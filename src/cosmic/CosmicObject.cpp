#include "cosmic/CosmicObject.hpp"
#include "physics/Schwarzschild.hpp"
#include "physics/Kerr.hpp"
#include "../utils/Constants.hpp"
#include <cmath>

namespace cosmic {

// ============================================================================
// BlackHole Implementation
// ============================================================================

BlackHole::BlackHole(std::unique_ptr<physics::Metric> metric, const std::string& name)
    : metric_(std::move(metric)), name_(name) {}

double BlackHole::getMass() const {
    return metric_->getMass();
}

double BlackHole::getRadius() const {
    return metric_->eventHorizonRadius();
}

bool BlackHole::isRotating() const {
    auto kerr = dynamic_cast<const physics::Kerr*>(metric_.get());
    if (kerr) {
        return std::abs(kerr->getSpin()) > 1e-6;
    }
    return false;
}

double BlackHole::rotationPeriod() const {
    auto kerr = dynamic_cast<const physics::Kerr*>(metric_.get());
    if (kerr && std::abs(kerr->getSpin()) > 1e-6) {
        // Approximate rotation period at horizon
        double M = getMass();
        double a = kerr->getSpin() * M;
        double r_plus = metric_->eventHorizonRadius();

        // Angular velocity at horizon: ω = a/(2Mr_+)
        double omega = a / (2.0 * M * r_plus);

        // Period = 2π/ω
        if (omega > 1e-10) {
            return constants::TWO_PI / omega;
        }
    }
    return std::numeric_limits<double>::infinity();
}

// ============================================================================
// NeutronStar Implementation
// ============================================================================

NeutronStar::NeutronStar(double mass, double radius, double temperature)
    : mass_(mass),
      radius_(radius),
      temperature_(temperature),
      period_(std::numeric_limits<double>::infinity()),
      B_surface_(1e12) {} // Typical value

double NeutronStar::surfaceTemperature(double theta, double phi) const {
    // For now, uniform temperature
    // Future: Add hot spots at magnetic poles
    return temperature_;
}

// ============================================================================
// Pulsar Implementation
// ============================================================================

Pulsar::Pulsar(double mass, double radius, double period)
    : NeutronStar(mass, radius, 1e6), // 1 million K
      tilt_(45.0) { // Typical 45° tilt
    setRotationPeriod(period);
    setMagneticField(1e12); // 10^12 Gauss
}

Eigen::Vector3d Pulsar::beamDirection(double time) const {
    // Rotation axis along z
    // Magnetic axis tilted by tilt_ degrees

    double omega = constants::TWO_PI / rotationPeriod();
    double phase = omega * time;

    double tilt_rad = tilt_ * constants::PI / 180.0;

    // Magnetic pole rotates around z-axis
    double x = std::sin(tilt_rad) * std::cos(phase);
    double y = std::sin(tilt_rad) * std::sin(phase);
    double z = std::cos(tilt_rad);

    return Eigen::Vector3d(x, y, z);
}

bool Pulsar::isBeamVisible(double time,
                           const Eigen::Vector3d& observerDir,
                           double coneAngle) const {
    Eigen::Vector3d beam = beamDirection(time);
    double dot = beam.dot(observerDir.normalized());
    double angle = std::acos(std::clamp(dot, -1.0, 1.0));

    return angle < coneAngle;
}

// ============================================================================
// CosmicObjectFactory Implementation
// ============================================================================

std::unique_ptr<BlackHole> CosmicObjectFactory::createCygnusX1() {
    // Cygnus X-1: First confirmed black hole
    // Mass: ~21 solar masses
    // Spin: High (a ~ 0.95)
    auto metric = std::make_unique<physics::Kerr>(21.0, 0.95);
    return std::make_unique<BlackHole>(std::move(metric), "Cygnus X-1");
}

std::unique_ptr<BlackHole> CosmicObjectFactory::createSagittariusAStar() {
    // Sgr A*: Black hole at center of Milky Way
    // Mass: ~4.15 million solar masses
    // Spin: Unknown, assume moderate
    auto metric = std::make_unique<physics::Kerr>(4.15e6, 0.5);
    return std::make_unique<BlackHole>(std::move(metric), "Sagittarius A*");
}

std::unique_ptr<BlackHole> CosmicObjectFactory::createM87() {
    // M87*: First black hole photographed by EHT
    // Mass: ~6.5 billion solar masses
    // Spin: High (a ~ 0.9)
    auto metric = std::make_unique<physics::Kerr>(6.5e9, 0.9);
    return std::make_unique<BlackHole>(std::move(metric), "M87*");
}

std::unique_ptr<BlackHole> CosmicObjectFactory::createGargantua() {
    // Gargantua from Interstellar
    // Mass: ~100 million solar masses (estimate)
    // Spin: Maximum (a = 0.998)
    auto metric = std::make_unique<physics::Kerr>(1.0e8, 0.998);
    return std::make_unique<BlackHole>(std::move(metric), "Gargantua");
}

std::unique_ptr<Pulsar> CosmicObjectFactory::createCrabPulsar() {
    // Crab Pulsar (PSR B0531+21)
    // Mass: ~1.4 solar masses
    // Radius: ~10 km
    // Period: 0.033 seconds (33 milliseconds)
    auto pulsar = std::make_unique<Pulsar>(1.4, 10.0, 0.033);
    pulsar->setMagneticTilt(30.0); // 30° tilt
    pulsar->setMagneticField(3.8e12); // 3.8×10^12 Gauss
    return pulsar;
}

std::unique_ptr<NeutronStar> CosmicObjectFactory::createPSRJ0348() {
    // PSR J0348+0432: Massive neutron star
    // Mass: 2.01 ± 0.04 solar masses (one of the heaviest known)
    // Radius: ~13 km
    return std::make_unique<NeutronStar>(2.01, 13.0, 5.0e5);
}

std::unique_ptr<BlackHole> CosmicObjectFactory::createBlackHole(
    double mass, double spin, const std::string& name) {

    std::unique_ptr<physics::Metric> metric;

    if (std::abs(spin) < 1e-6) {
        // Non-rotating
        metric = std::make_unique<physics::Schwarzschild>(mass);
    } else {
        // Rotating
        metric = std::make_unique<physics::Kerr>(mass, spin);
    }

    return std::make_unique<BlackHole>(std::move(metric), name);
}

} // namespace cosmic
