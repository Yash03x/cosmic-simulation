#include "cosmic/GravitationalWaves.hpp"
#include <glm/gtc/constants.hpp>
#include <cmath>

namespace cosmic {

// ==================== GravitationalWavePresets ====================

std::vector<GWEventProperties> GravitationalWavePresets::getAllPresets() {
    return {
        getGW150914(),
        getGW170817(),
        getGW190521(),
        getGW190814(),
        getGW200105(),
        getGW200115()
    };
}

GWEventProperties GravitationalWavePresets::getPreset(const std::string& name) {
    auto presets = getAllPresets();
    for (const auto& preset : presets) {
        if (preset.name == name) {
            return preset;
        }
    }
    return presets[0];
}

GWEventProperties GravitationalWavePresets::getGW150914() {
    GWEventProperties props;
    props.name = "GW150914";
    props.description = "First gravitational wave detection! Binary black hole merger";
    props.mass1 = 36.0f;
    props.mass2 = 29.0f;
    props.finalMass = 62.0f;
    props.energyRadiated = 3.0f;  // 3 solar masses converted to GW energy!
    props.distance = 410.0f;  // Mpc
    props.peakFrequency = 250.0f;  // Hz
    props.peakStrain = 1.0e-21f;
    props.chirpMass = 30.0f;
    props.duration = 0.2f;  // seconds
    props.type = GWEventProperties::Type::BinaryBlackHole;
    props.waveColor = glm::vec3(0.2f, 0.8f, 1.0f);  // Cyan
    props.visualAmplification = 1e21f;
    props.detectionDate = "September 14, 2015";
    return props;
}

GWEventProperties GravitationalWavePresets::getGW170817() {
    GWEventProperties props;
    props.name = "GW170817";
    props.description = "Neutron star merger with electromagnetic counterpart (kilonova!)";
    props.mass1 = 1.46f;
    props.mass2 = 1.27f;
    props.finalMass = 2.7f;  // Including ejected material
    props.energyRadiated = 0.03f;  // Less than BBH
    props.distance = 40.0f;  // Mpc (much closer!)
    props.peakFrequency = 1000.0f;  // Higher frequency (smaller masses)
    props.peakStrain = 1.5e-22f;
    props.chirpMass = 1.188f;
    props.duration = 100.0f;  // Much longer (lower masses)
    props.type = GWEventProperties::Type::BinaryNeutronStar;
    props.waveColor = glm::vec3(1.0f, 0.8f, 0.2f);  // Gold
    props.visualAmplification = 1e22f;
    props.detectionDate = "August 17, 2017";
    return props;
}

GWEventProperties GravitationalWavePresets::getGW190521() {
    GWEventProperties props;
    props.name = "GW190521";
    props.description = "Massive black hole merger -> intermediate mass BH (142 M☉)";
    props.mass1 = 85.0f;
    props.mass2 = 66.0f;
    props.finalMass = 142.0f;  // Intermediate mass black hole!
    props.energyRadiated = 8.0f;  // Huge energy release
    props.distance = 5300.0f;  // Very distant
    props.peakFrequency = 60.0f;  // Low frequency (large masses)
    props.peakStrain = 1.5e-22f;
    props.chirpMass = 65.0f;
    props.duration = 0.1f;
    props.type = GWEventProperties::Type::BinaryBlackHole;
    props.waveColor = glm::vec3(1.0f, 0.3f, 0.3f);  // Red
    props.visualAmplification = 1e22f;
    props.detectionDate = "May 21, 2019";
    return props;
}

GWEventProperties GravitationalWavePresets::getGW190814() {
    GWEventProperties props;
    props.name = "GW190814";
    props.description = "Mystery object (2.6 M☉): Heaviest neutron star or lightest black hole?";
    props.mass1 = 23.0f;
    props.mass2 = 2.6f;  // The mystery object!
    props.finalMass = 25.0f;
    props.energyRadiated = 0.6f;
    props.distance = 241.0f;  // Mpc
    props.peakFrequency = 180.0f;
    props.peakStrain = 8.0e-22f;
    props.chirpMass = 5.7f;
    props.duration = 1.5f;
    props.type = GWEventProperties::Type::NeutronStarBlackHole;
    props.waveColor = glm::vec3(0.8f, 0.3f, 1.0f);  // Purple (mystery!)
    props.visualAmplification = 1e21f;
    props.detectionDate = "August 14, 2019";
    return props;
}

GWEventProperties GravitationalWavePresets::getGW200105() {
    GWEventProperties props;
    props.name = "GW200105";
    props.description = "Neutron star - black hole merger";
    props.mass1 = 8.9f;
    props.mass2 = 1.9f;
    props.finalMass = 10.8f;
    props.energyRadiated = 0.3f;
    props.distance = 280.0f;
    props.peakFrequency = 300.0f;
    props.peakStrain = 6.0e-22f;
    props.chirpMass = 3.4f;
    props.duration = 2.0f;
    props.type = GWEventProperties::Type::NeutronStarBlackHole;
    props.waveColor = glm::vec3(0.3f, 1.0f, 0.5f);  // Green
    props.visualAmplification = 1e22f;
    props.detectionDate = "January 5, 2020";
    return props;
}

GWEventProperties GravitationalWavePresets::getGW200115() {
    GWEventProperties props;
    props.name = "GW200115";
    props.description = "Another NSBH merger, 10 days after GW200105";
    props.mass1 = 5.7f;
    props.mass2 = 1.5f;
    props.finalMass = 7.2f;
    props.energyRadiated = 0.2f;
    props.distance = 300.0f;
    props.peakFrequency = 400.0f;
    props.peakStrain = 5.0e-22f;
    props.chirpMass = 2.7f;
    props.duration = 3.0f;
    props.type = GWEventProperties::Type::NeutronStarBlackHole;
    props.waveColor = glm::vec3(0.5f, 0.8f, 1.0f);  // Light blue
    props.visualAmplification = 1e22f;
    props.detectionDate = "January 15, 2020";
    return props;
}

// ==================== GravitationalWaveSystem ====================

GravitationalWaveSystem::GravitationalWaveSystem()
    : sourcePosition_(0.0f, 0.0f, 0.0f),
      active_(false),
      visualizationMode_(VisualizationMode::Rings),
      chirpEnabled_(true),
      timeSinceLastEmission_(0.0f),
      emissionInterval_(0.1f) {

    // Default to GW150914
    event_ = GravitationalWavePresets::getGW150914();
}

void GravitationalWaveSystem::setEvent(const GWEventProperties& event) {
    event_ = event;
    clear();  // Clear existing pulses
}

void GravitationalWaveSystem::update(float deltaTime) {
    if (!active_) {
        return;
    }

    // Update emission timer
    timeSinceLastEmission_ += deltaTime;

    // Emit new pulses periodically
    if (timeSinceLastEmission_ >= emissionInterval_) {
        emitPulse();
        timeSinceLastEmission_ = 0.0f;
    }

    // Update existing pulses
    for (auto& pulse : pulses_) {
        if (!pulse.active) continue;

        // Propagate wave front
        pulse.currentRadius += WAVE_SPEED * deltaTime;

        // Update phase (oscillation)
        pulse.phase += 2.0f * glm::pi<float>() * pulse.frequency * deltaTime;

        // Amplitude decays as 1/r
        if (pulse.currentRadius > 0.1f) {
            pulse.amplitude = event_.peakStrain * event_.visualAmplification / pulse.currentRadius;
        }

        // Deactivate if too far
        if (pulse.currentRadius > MAX_RADIUS) {
            pulse.active = false;
        }

        // Chirp: frequency increases as merger approaches
        if (chirpEnabled_) {
            float chirpFactor = 1.0f + 0.5f * (pulse.currentRadius / MAX_RADIUS);
            pulse.frequency = event_.peakFrequency * chirpFactor;
        }
    }

    // Remove inactive pulses (keep last 100)
    while (pulses_.size() > 100) {
        pulses_.erase(pulses_.begin());
    }
}

void GravitationalWaveSystem::emitPulse() {
    GWPulse pulse;
    pulse.origin = sourcePosition_;
    pulse.currentRadius = 0.1f;  // Start small
    pulse.amplitude = event_.peakStrain * event_.visualAmplification;
    pulse.frequency = event_.peakFrequency;
    pulse.phase = 0.0f;
    pulse.birthTime = 0.0f;  // Could use simulation time
    pulse.active = true;

    pulses_.push_back(pulse);
}

float GravitationalWaveSystem::calculateStrainAt(float distance) const {
    if (distance < 0.1f) {
        return event_.peakStrain * event_.visualAmplification;
    }

    // h ~ (4G/c^4) * (E/r)
    // For visualization, we use peak strain scaled by 1/r
    return (event_.peakStrain * event_.visualAmplification) / distance;
}

} // namespace cosmic
