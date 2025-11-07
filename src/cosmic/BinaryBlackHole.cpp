#include "cosmic/BinaryBlackHole.hpp"
#include <glm/gtc/constants.hpp>
#include <cmath>

namespace cosmic {

// ==================== BinaryBlackHolePresets ====================

BinaryBlackHoleProperties BinaryBlackHolePresets::getGW150914() {
    BinaryBlackHoleProperties props;
    props.name = "GW150914";
    props.mass1 = 36.0f;  // Solar masses
    props.mass2 = 29.0f;
    props.initialSeparation = 350.0f;  // M (geometric units)
    props.eccentricity = 0.0f;  // Nearly circular
    props.inclination = 0.0f;
    props.mergerTime = 0.2f;  // 0.2 seconds of observable inspiral
    return props;
}

BinaryBlackHoleProperties BinaryBlackHolePresets::getGW170814() {
    BinaryBlackHoleProperties props;
    props.name = "GW170814";
    props.mass1 = 30.5f;
    props.mass2 = 25.3f;
    props.initialSeparation = 320.0f;
    props.eccentricity = 0.0f;
    props.inclination = 0.0f;
    props.mergerTime = 0.18f;
    return props;
}

BinaryBlackHoleProperties BinaryBlackHolePresets::getGW190521() {
    BinaryBlackHoleProperties props;
    props.name = "GW190521";
    props.mass1 = 85.0f;  // Intermediate mass
    props.mass2 = 66.0f;
    props.initialSeparation = 500.0f;
    props.eccentricity = 0.0f;
    props.inclination = 0.0f;
    props.mergerTime = 0.1f;  // Brief inspiral
    return props;
}

BinaryBlackHoleProperties BinaryBlackHolePresets::getGW190412() {
    BinaryBlackHoleProperties props;
    props.name = "GW190412";
    props.mass1 = 30.0f;
    props.mass2 = 8.0f;   // Highly unequal masses
    props.initialSeparation = 280.0f;
    props.eccentricity = 0.0f;
    props.inclination = 0.0f;
    props.mergerTime = 0.25f;
    return props;
}

BinaryBlackHoleProperties BinaryBlackHolePresets::getGenericCircular() {
    BinaryBlackHoleProperties props;
    props.name = "Generic Circular Binary";
    props.mass1 = 10.0f;
    props.mass2 = 10.0f;
    props.initialSeparation = 200.0f;
    props.eccentricity = 0.0f;
    props.inclination = 0.0f;
    props.mergerTime = 1.0f;  // Longer inspiral for visualization
    return props;
}

BinaryBlackHoleProperties BinaryBlackHolePresets::getGenericEccentric() {
    BinaryBlackHoleProperties props;
    props.name = "Generic Eccentric Binary";
    props.mass1 = 15.0f;
    props.mass2 = 10.0f;
    props.initialSeparation = 250.0f;
    props.eccentricity = 0.3f;  // Moderate eccentricity
    props.inclination = 0.3f;
    props.mergerTime = 1.2f;
    return props;
}

// ==================== BinaryBlackHole ====================

BinaryBlackHole::BinaryBlackHole(const BinaryBlackHoleProperties& props)
    : props_(props),
      separation_(props.initialSeparation),
      orbitalFrequency_(0.0f),
      orbitalPhase_(0.0f),
      elapsedTime_(0.0f),
      state_(BinaryState::Inspiral),
      visible_(true),
      mergerStartTime_(0.0f),
      ringdownStartTime_(0.0f),
      finalMass_(0.0f),
      finalSpin_(0.0f) {

    reset();
}

void BinaryBlackHole::reset() {
    separation_ = props_.initialSeparation;
    elapsedTime_ = 0.0f;
    orbitalPhase_ = 0.0f;
    state_ = BinaryState::Inspiral;

    // Calculate initial orbital frequency using Kepler's 3rd law
    // ω² = M / r³ (geometric units, G=c=1)
    float M = props_.totalMass();
    orbitalFrequency_ = std::sqrt(M / std::pow(separation_, 3.0f)) / (2.0f * glm::pi<float>());

    // Calculate center of mass positions
    // Position BHs relative to center of mass
    float r1 = separation_ * props_.mass2 / M;  // Distance of BH1 from COM
    float r2 = separation_ * props_.mass1 / M;  // Distance of BH2 from COM

    updatePositions();
    calculateFinalProperties();
}

void BinaryBlackHole::update(float deltaTime) {
    if (!visible_) return;

    elapsedTime_ += deltaTime;

    switch (state_) {
        case BinaryState::Inspiral:
            updateInspiral(deltaTime);
            break;
        case BinaryState::Merger:
            updateMerger(deltaTime);
            break;
        case BinaryState::Ringdown:
            updateRingdown(deltaTime);
            break;
        case BinaryState::Complete:
            // Nothing to update
            break;
    }
}

void BinaryBlackHole::updateInspiral(float deltaTime) {
    // Calculate inspiral rate (dr/dt) due to gravitational wave emission
    float drdt = calculateInspiralRate();

    // Update separation
    separation_ += drdt * deltaTime;

    // Check if we've reached merger
    // ISCO for equal mass binary is around r ~ 6M
    float M = props_.totalMass();
    float iscoSeparation = 6.0f * M;

    if (separation_ <= iscoSeparation || elapsedTime_ >= props_.mergerTime) {
        state_ = BinaryState::Merger;
        mergerStartTime_ = elapsedTime_;
        return;
    }

    // Update orbital frequency from separation
    orbitalFrequency_ = std::sqrt(M / std::pow(separation_, 3.0f)) / (2.0f * glm::pi<float>());

    // Update orbital phase
    orbitalPhase_ += 2.0f * glm::pi<float>() * orbitalFrequency_ * deltaTime;
    orbitalPhase_ = std::fmod(orbitalPhase_, 2.0f * glm::pi<float>());

    // Update positions
    updatePositions();
}

void BinaryBlackHole::updateMerger(float deltaTime) {
    float mergerDuration = 0.01f;  // Very rapid merger (0.01 seconds)
    float t = elapsedTime_ - mergerStartTime_;

    if (t >= mergerDuration) {
        state_ = BinaryState::Ringdown;
        ringdownStartTime_ = elapsedTime_;

        // Calculate final position (center of mass)
        finalPosition_ = (position1_ * props_.mass1 + position2_ * props_.mass2) / props_.totalMass();
        return;
    }

    // Rapid inspiral during merger
    float progress = t / mergerDuration;
    separation_ = separation_ * (1.0f - progress);

    // Very high frequency during merger
    float M = props_.totalMass();
    if (separation_ > 0.1f) {
        orbitalFrequency_ = std::sqrt(M / std::pow(separation_, 3.0f)) / (2.0f * glm::pi<float>());
        orbitalPhase_ += 2.0f * glm::pi<float>() * orbitalFrequency_ * deltaTime;
    }

    updatePositions();
}

void BinaryBlackHole::updateRingdown(float deltaTime) {
    float ringdownDuration = 0.05f;  // 0.05 seconds
    float t = elapsedTime_ - ringdownStartTime_;

    if (t >= ringdownDuration) {
        state_ = BinaryState::Complete;
        return;
    }

    // During ringdown, BHs are merging into final position
    float progress = t / ringdownDuration;
    float smoothProgress = progress * progress * (3.0f - 2.0f * progress);  // Smoothstep

    // Interpolate positions to final position
    position1_ = glm::mix(position1_, finalPosition_, smoothProgress);
    position2_ = glm::mix(position2_, finalPosition_, smoothProgress);

    separation_ = glm::length(position2_ - position1_);
}

void BinaryBlackHole::updatePositions() {
    float M = props_.totalMass();
    float r1 = separation_ * props_.mass2 / M;  // Distance from COM
    float r2 = separation_ * props_.mass1 / M;

    // Position along orbit (circular for now)
    float x1 = r1 * std::cos(orbitalPhase_);
    float y1 = r1 * std::sin(orbitalPhase_) * std::cos(props_.inclination);
    float z1 = r1 * std::sin(orbitalPhase_) * std::sin(props_.inclination);

    float x2 = -r2 * std::cos(orbitalPhase_);
    float y2 = -r2 * std::sin(orbitalPhase_) * std::cos(props_.inclination);
    float z2 = -r2 * std::sin(orbitalPhase_) * std::sin(props_.inclination);

    position1_ = glm::vec3(x1, y1, z1);
    position2_ = glm::vec3(x2, y2, z2);

    // Calculate velocities (tangent to orbit)
    float v = 2.0f * glm::pi<float>() * orbitalFrequency_;
    velocity1_ = glm::vec3(-y1 * v, x1 * v, 0.0f);
    velocity2_ = glm::vec3(-y2 * v, x2 * v, 0.0f);
}

float BinaryBlackHole::calculateInspiralRate() const {
    // Peters-Mathews formula for circular orbits
    // dr/dt = -64/5 * (M₁M₂/M²) * M / r³
    // In geometric units (G=c=1)

    float M = props_.totalMass();
    float mu = props_.reducedMass();
    float eta = mu / M;  // Symmetric mass ratio

    // Inspiral rate (negative = decreasing separation)
    float drdt = -(64.0f / 5.0f) * eta * M * M / std::pow(separation_, 3.0f);

    return drdt;
}

float BinaryBlackHole::getSeparation() const {
    if (state_ == BinaryState::Complete) {
        return 0.0f;
    }
    return separation_;
}

float BinaryBlackHole::getOrbitalVelocity() const {
    if (state_ == BinaryState::Complete) {
        return 0.0f;
    }

    // v = √(M/r) for circular orbit
    float M = props_.totalMass();
    return std::sqrt(M / separation_);
}

float BinaryBlackHole::getGWStrain() const {
    if (state_ == BinaryState::Complete) {
        return 0.0f;
    }

    // Simplified strain amplitude
    // h ~ (M_chirp^(5/3) * ω^(2/3)) / r
    // For visualization, use arbitrary distance r = 1000 M

    float M_chirp = props_.chirpMass();
    float omega = 2.0f * glm::pi<float>() * orbitalFrequency_;
    float distance = 1000.0f;  // Arbitrary viewing distance

    float h = std::pow(M_chirp, 5.0f/3.0f) * std::pow(omega, 2.0f/3.0f) / distance;

    // Amplify during merger
    if (state_ == BinaryState::Merger) {
        h *= 3.0f;
    } else if (state_ == BinaryState::Ringdown) {
        float t = elapsedTime_ - ringdownStartTime_;
        h *= 2.0f * std::exp(-t * 20.0f);  // Exponential decay
    }

    return h;
}

float BinaryBlackHole::getTimeToMerger() const {
    if (state_ == BinaryState::Complete) {
        return 0.0f;
    }

    // Approximate time to merger using Peters formula
    // t_merge = 5/256 * r⁴ / (M₁M₂M)

    float M = props_.totalMass();
    float mu = props_.reducedMass();

    float t = (5.0f / 256.0f) * std::pow(separation_, 4.0f) / (mu * M * M);

    return t;
}

void BinaryBlackHole::calculateFinalProperties() {
    // Calculate final mass and spin using fitting formulas
    // From numerical relativity simulations

    float M = props_.totalMass();
    float eta = props_.reducedMass() / M;

    // Energy radiated (simplified)
    // E_rad ~ η² M (for equal mass, ~4% of total mass)
    float E_rad = eta * eta * M * 0.04f;

    finalMass_ = M - E_rad;

    // Final spin (simplified Barausse-Rezzolla formula)
    // For equal mass non-spinning: a_final ~ 0.686
    float q = props_.massRatio();  // mass2 / mass1
    float s = q / ((1.0f + q) * (1.0f + q));  // Reduced mass ratio
    finalSpin_ = std::sqrt(12.0f) * s - 3.871f * s * s + 4.028f * s * s * s;
    finalSpin_ = glm::clamp(finalSpin_, 0.0f, 0.998f);
}

float BinaryBlackHole::getFinalMass() const {
    return finalMass_;
}

float BinaryBlackHole::getFinalSpin() const {
    return finalSpin_;
}

float BinaryBlackHole::getEnergyRadiated() const {
    float M = props_.totalMass();
    return M - finalMass_;
}

} // namespace cosmic
