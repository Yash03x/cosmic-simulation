#include "tools/ParticleTrajectory.hpp"
#include "physics/Metric.hpp"
#include <glm/gtc/constants.hpp>
#include <cmath>

namespace cosmic {
namespace tools {

// ==================== ParticleTrajectory ====================

ParticleTrajectory::ParticleTrajectory(const ParticleInitialConditions& initialConditions,
                                         const physics::Metric* metric,
                                         size_t maxPoints)
    : initialConditions_(initialConditions),
      metric_(metric),
      maxPoints_(maxPoints),
      hitEventHorizon_(false),
      escaped_(false),
      color_(1.0f, 0.8f, 0.2f) {  // Default: golden color

    // Add initial point
    TrajectoryPoint initialPoint;
    initialPoint.position = initialConditions.position;
    initialPoint.velocity = normalizeVelocity(initialConditions.position, initialConditions.velocity);
    initialPoint.properTime = 0.0f;
    initialPoint.coordinateTime = 0.0f;
    points_.push_back(initialPoint);
}

bool ParticleTrajectory::integrate(float stepSize) {
    if (!isActive()) {
        return false;
    }

    const auto& current = points_.back();

    // Check if inside event horizon
    float r = glm::length(current.position);
    double eventHorizon = metric_->eventHorizonRadius();

    if (r < eventHorizon * 1.01f) {  // Small margin for numerical stability
        hitEventHorizon_ = true;
        return false;
    }

    // Check if escaped to infinity
    if (r > 100.0f) {  // Arbitrary large distance
        escaped_ = true;
        return false;
    }

    // RK4 integration
    // k1 = f(y_n)
    glm::vec3 k1_v = calculateGeodesicAcceleration(current.position, current.velocity);
    glm::vec3 k1_x = current.velocity;

    // k2 = f(y_n + h/2 * k1)
    glm::vec3 pos2 = current.position + 0.5f * stepSize * k1_x;
    glm::vec3 vel2 = current.velocity + 0.5f * stepSize * k1_v;
    glm::vec3 k2_v = calculateGeodesicAcceleration(pos2, vel2);
    glm::vec3 k2_x = vel2;

    // k3 = f(y_n + h/2 * k2)
    glm::vec3 pos3 = current.position + 0.5f * stepSize * k2_x;
    glm::vec3 vel3 = current.velocity + 0.5f * stepSize * k2_v;
    glm::vec3 k3_v = calculateGeodesicAcceleration(pos3, vel3);
    glm::vec3 k3_x = vel3;

    // k4 = f(y_n + h * k3)
    glm::vec3 pos4 = current.position + stepSize * k3_x;
    glm::vec3 vel4 = current.velocity + stepSize * k3_v;
    glm::vec3 k4_v = calculateGeodesicAcceleration(pos4, vel4);
    glm::vec3 k4_x = vel4;

    // Combine
    TrajectoryPoint next;
    next.position = current.position + (stepSize / 6.0f) * (k1_x + 2.0f*k2_x + 2.0f*k3_x + k4_x);
    next.velocity = current.velocity + (stepSize / 6.0f) * (k1_v + 2.0f*k2_v + 2.0f*k3_v + k4_v);
    next.coordinateTime = current.coordinateTime + stepSize;

    // Calculate proper time increment
    float dt_proper = stepSize * glm::length(next.velocity);
    next.properTime = current.properTime + dt_proper;

    points_.push_back(next);

    return true;
}

void ParticleTrajectory::integrateForTime(float totalTime, float stepSize) {
    float elapsed = 0.0f;
    while (elapsed < totalTime && isActive()) {
        if (!integrate(stepSize)) {
            break;
        }
        elapsed += stepSize;
    }
}

glm::vec3 ParticleTrajectory::calculateGeodesicAcceleration(
    const glm::vec3& position, const glm::vec3& velocity) const {

    // Simplified geodesic equation for Schwarzschild geometry
    // a^i = -Γ^i_jk v^j v^k
    //
    // For Schwarzschild in Cartesian-like coords, the main effect is:
    // a = -GM/r^3 * r_vec * (correction factors from GR)

    float r = glm::length(position);
    if (r < 0.001f) r = 0.001f;  // Avoid singularity

    double eventHorizon = metric_->eventHorizonRadius();
    float rs = 2.0f * static_cast<float>(eventHorizon);  // Schwarzschild radius

    // Newtonian-like acceleration with GR corrections
    float grFactor = 1.0f + 3.0f * rs / (2.0f * r);  // Simple GR correction

    // Radial component
    glm::vec3 r_hat = glm::normalize(position);
    float v_radial = glm::dot(velocity, r_hat);

    // Acceleration
    glm::vec3 a_radial = -(rs / (2.0f * r * r)) * r_hat * grFactor;

    // Add frame-dragging for non-zero spin (Kerr)
    // Simplified: orbital precession effect
    glm::vec3 L = glm::cross(position, velocity);  // Angular momentum
    if (glm::length(L) > 0.001f) {
        glm::vec3 L_hat = glm::normalize(L);
        float precessionRate = rs / (2.0f * r * r * r);
        glm::vec3 a_precession = precessionRate * glm::cross(velocity, L_hat);
        return a_radial + a_precession;
    }

    return a_radial;
}

glm::vec3 ParticleTrajectory::normalizeVelocity(
    const glm::vec3& position, const glm::vec3& velocity) const {

    // For massive particles, ensure velocity is timelike
    // For photons, ensure velocity is lightlike (|v| = 1 in our units)

    if (!initialConditions_.isMassive) {
        // Photon: normalize to speed of light (c = 1)
        return glm::normalize(velocity);
    } else {
        // Massive particle: velocity should be < c
        float v_mag = glm::length(velocity);
        if (v_mag > 0.9f) {  // Cap at 90% of light speed for stability
            return 0.9f * glm::normalize(velocity);
        }
        return velocity;
    }
}

// ==================== ParticleSystem ====================

ParticleSystem::ParticleSystem()
    : active_(false),
      integrationStepSize_(0.01f) {
}

void ParticleSystem::addParticle(const ParticleInitialConditions& initialConditions,
                                  const physics::Metric* metric) {
    auto trajectory = std::make_unique<ParticleTrajectory>(initialConditions, metric);

    // Assign color based on trajectory type
    if (!initialConditions.isMassive) {
        trajectory->setColor(glm::vec3(1.0f, 1.0f, 0.5f));  // Photons: yellow
    } else if (glm::length(initialConditions.velocity) < 0.3f) {
        trajectory->setColor(glm::vec3(1.0f, 0.3f, 0.3f));  // Slow: red
    } else {
        trajectory->setColor(glm::vec3(0.3f, 0.8f, 1.0f));  // Fast: blue
    }

    trajectories_.push_back(std::move(trajectory));
}

void ParticleSystem::addCircularOrbit(float radius, const physics::Metric* metric) {
    // Calculate circular orbital velocity
    // v = sqrt(GM/r) = sqrt(r_s/(2r)) in our units
    double eventHorizon = metric->eventHorizonRadius();
    float rs = 2.0f * static_cast<float>(eventHorizon);
    float v_orbital = std::sqrt(rs / (2.0f * radius));

    ParticleInitialConditions ic;
    ic.position = glm::vec3(radius, 0.0f, 0.0f);
    ic.velocity = glm::vec3(0.0f, 0.0f, v_orbital);  // Tangential velocity
    ic.isMassive = true;
    ic.energy = 1.0f;
    ic.angularMomentum = radius * v_orbital;

    addParticle(ic, metric);
}

void ParticleSystem::addEllipticalOrbit(float periapsis, float apoapsis,
                                         const physics::Metric* metric) {
    // Start at periapsis with velocity for elliptical orbit
    double eventHorizon = metric->eventHorizonRadius();
    float rs = 2.0f * static_cast<float>(eventHorizon);

    // Semi-major axis
    float a = (periapsis + apoapsis) / 2.0f;

    // Velocity at periapsis (vis-viva equation)
    float v_peri = std::sqrt(rs / (2.0f * a) * (apoapsis / periapsis));

    ParticleInitialConditions ic;
    ic.position = glm::vec3(periapsis, 0.0f, 0.0f);
    ic.velocity = glm::vec3(0.0f, 0.0f, v_peri);
    ic.isMassive = true;
    ic.energy = 1.0f;
    ic.angularMomentum = periapsis * v_peri;

    addParticle(ic, metric);
}

void ParticleSystem::addRadialInfall(float startRadius, const physics::Metric* metric) {
    ParticleInitialConditions ic;
    ic.position = glm::vec3(startRadius, 0.0f, 0.0f);
    ic.velocity = glm::vec3(-0.1f, 0.0f, 0.0f);  // Small inward velocity
    ic.isMassive = true;
    ic.energy = 1.0f;
    ic.angularMomentum = 0.0f;

    auto trajectory = std::make_unique<ParticleTrajectory>(ic, metric);
    trajectory->setColor(glm::vec3(1.0f, 0.5f, 0.0f));  // Orange for infall
    trajectories_.push_back(std::move(trajectory));
}

void ParticleSystem::update(float deltaTime) {
    if (!active_) {
        return;
    }

    // Integrate all active trajectories
    for (auto& trajectory : trajectories_) {
        if (trajectory->isActive()) {
            trajectory->integrate(integrationStepSize_);
        }
    }
}

} // namespace tools
} // namespace cosmic
