#include "cosmic/NeutronStar.hpp"
#include <glm/gtc/constants.hpp>
#include <cmath>

namespace cosmic {

// ==================== NeutronStarPresets ====================

std::vector<NeutronStarProperties> NeutronStarPresets::getAllPresets() {
    return {
        getPSRJ1748(),
        getCrabPulsar(),
        getVelaPulsar(),
        getMagnetar(),
        getMillisecond(),
        getTolmanOV()
    };
}

NeutronStarProperties NeutronStarPresets::getPreset(const std::string& name) {
    auto presets = getAllPresets();
    for (const auto& preset : presets) {
        if (preset.name == name) {
            return preset;
        }
    }
    return presets[0]; // Default to first preset
}

NeutronStarProperties NeutronStarPresets::getPSRJ1748() {
    NeutronStarProperties props;
    props.name = "PSR J1748-2446ad";
    props.description = "Fastest-spinning neutron star known (716 Hz)";
    props.mass = 1.4f;
    props.radius = 10.0f;  // km
    props.rotationPeriod = 0.00139f;  // 1.39 milliseconds!
    props.magneticFieldStrength = 1e9f;  // 10^9 Tesla
    props.surfaceTemperature = 1e6f;  // ~1 million K
    props.surfaceColor = glm::vec3(0.8f, 0.9f, 1.0f);  // Hot blue-white
    props.hasHotspots = true;
    props.hasMountains = true;
    props.crustalDeformation = 0.01f;
    props.cameraDistance = 100.0f;
    props.cameraHeight = 30.0f;
    return props;
}

NeutronStarProperties NeutronStarPresets::getCrabPulsar() {
    NeutronStarProperties props;
    props.name = "Crab Pulsar (PSR B0531+21)";
    props.description = "Supernova remnant from 1054 AD, spins 30 times/sec";
    props.mass = 1.4f;
    props.radius = 10.0f;
    props.rotationPeriod = 0.033f;  // 33 milliseconds
    props.magneticFieldStrength = 1e8f;
    props.surfaceTemperature = 1.6e6f;
    props.surfaceColor = glm::vec3(0.9f, 0.8f, 0.7f);  // Warm white
    props.hasHotspots = true;
    props.hasMountains = true;
    props.crustalDeformation = 0.005f;
    props.cameraDistance = 120.0f;
    props.cameraHeight = 25.0f;
    return props;
}

NeutronStarProperties NeutronStarPresets::getVelaPulsar() {
    NeutronStarProperties props;
    props.name = "Vela Pulsar (PSR J0835-4510)";
    props.description = "Young pulsar, 11 times per second, nearby (959 ly)";
    props.mass = 1.4f;
    props.radius = 12.0f;
    props.rotationPeriod = 0.089f;  // 89 milliseconds
    props.magneticFieldStrength = 3.4e8f;
    props.surfaceTemperature = 6e5f;
    props.surfaceColor = glm::vec3(1.0f, 0.9f, 0.8f);  // Yellowish-white
    props.hasHotspots = true;
    props.hasMountains = false;
    props.crustalDeformation = 0.003f;
    props.cameraDistance = 150.0f;
    props.cameraHeight = 40.0f;
    return props;
}

NeutronStarProperties NeutronStarPresets::getMagnetar() {
    NeutronStarProperties props;
    props.name = "Magnetar SGR 1806-20";
    props.description = "Strongest magnetic field known: 10^11 Tesla!";
    props.mass = 1.8f;
    props.radius = 10.0f;
    props.rotationPeriod = 7.5f;  // Slow rotation (7.5 seconds)
    props.magneticFieldStrength = 1e11f;  // 100 billion Tesla!
    props.surfaceTemperature = 1e7f;  // 10 million K
    props.surfaceColor = glm::vec3(1.0f, 0.4f, 0.3f);  // Reddish (starquakes!)
    props.hasHotspots = true;
    props.hasMountains = true;
    props.crustalDeformation = 0.02f;  // Extreme stress
    props.cameraDistance = 200.0f;
    props.cameraHeight = 50.0f;
    return props;
}

NeutronStarProperties NeutronStarPresets::getMillisecond() {
    NeutronStarProperties props;
    props.name = "Millisecond Pulsar (Generic)";
    props.description = "Recycled pulsar spun up by companion star";
    props.mass = 1.5f;
    props.radius = 11.0f;
    props.rotationPeriod = 0.003f;  // 3 milliseconds
    props.magneticFieldStrength = 1e8f;  // Weaker field (old age)
    props.surfaceTemperature = 5e5f;
    props.surfaceColor = glm::vec3(0.9f, 0.9f, 1.0f);  // Cool blue-white
    props.hasHotspots = true;
    props.hasMountains = false;
    props.crustalDeformation = 0.008f;
    props.cameraDistance = 110.0f;
    props.cameraHeight = 30.0f;
    return props;
}

NeutronStarProperties NeutronStarPresets::getTolmanOV() {
    NeutronStarProperties props;
    props.name = "Tolman-Oppenheimer-Volkoff Limit";
    props.description = "Maximum mass neutron star (~2.1 M☉) before collapse";
    props.mass = 2.1f;
    props.radius = 11.5f;
    props.rotationPeriod = 0.5f;  // Moderate rotation
    props.magneticFieldStrength = 5e9f;
    props.surfaceTemperature = 8e5f;
    props.surfaceColor = glm::vec3(0.95f, 0.85f, 0.75f);
    props.hasHotspots = true;
    props.hasMountains = true;
    props.crustalDeformation = 0.015f;  // Near collapse!
    props.cameraDistance = 130.0f;
    props.cameraHeight = 35.0f;
    return props;
}

// ==================== NeutronStar ====================

NeutronStar::NeutronStar(const NeutronStarProperties& properties)
    : properties_(properties),
      position_(0.0f, 0.0f, 0.0f),
      visible_(false),
      rotationAngle_(0.0f) {

    // Calculate angular velocity from period
    angularVelocity_ = 2.0f * glm::pi<float>() / properties_.rotationPeriod;

    // Set rotation axis (slightly tilted for visual interest)
    rotationAxis_ = glm::normalize(glm::vec3(0.05f, 0.98f, 0.1f));

    // Magnetic axis is misaligned from rotation axis
    // This causes the "lighthouse" effect in pulsars
    float misalignAngle = glm::radians(15.0f);  // 15 degree tilt
    glm::vec3 perpendicular = glm::normalize(glm::cross(rotationAxis_, glm::vec3(1.0f, 0.0f, 0.0f)));
    magneticAxis_ = glm::normalize(
        rotationAxis_ * std::cos(misalignAngle) +
        perpendicular * std::sin(misalignAngle)
    );
}

void NeutronStar::update(float deltaTime) {
    if (!visible_) return;

    // Update rotation
    rotationAngle_ += angularVelocity_ * deltaTime;

    // Keep angle in [0, 2π]
    if (rotationAngle_ > 2.0f * glm::pi<float>()) {
        rotationAngle_ -= 2.0f * glm::pi<float>();
    }
}

std::vector<glm::vec3> NeutronStar::getHotspots() const {
    if (!properties_.hasHotspots) {
        return {};
    }

    // Rotate magnetic axis by current rotation angle
    float c = std::cos(rotationAngle_);
    float s = std::sin(rotationAngle_);

    // Rodrigues' rotation formula
    glm::vec3 k = rotationAxis_;
    glm::vec3 v = magneticAxis_;
    glm::vec3 rotated = v * c + glm::cross(k, v) * s + k * glm::dot(k, v) * (1.0f - c);

    // Two hotspots at magnetic poles
    float radius = static_cast<float>(getPhysicalRadius());
    std::vector<glm::vec3> hotspots;
    hotspots.push_back(position_ + rotated * radius);
    hotspots.push_back(position_ - rotated * radius);

    return hotspots;
}

double NeutronStar::getSchwarzschildRadius() const {
    // r_s = 2GM/c²
    // Convert to geometric units where c=1, G=1, M_sun=1
    return properties_.mass;  // In solar masses
}

double NeutronStar::getPhysicalRadius() const {
    // Convert km to geometric units (solar masses)
    // 1 M☉ = 1.477 km in geometric units
    return properties_.radius / 1.477;
}

float NeutronStar::getSurfaceRedshift() const {
    // z = 1/sqrt(1 - r_s/R) - 1
    double rs = getSchwarzschildRadius();
    double R = getPhysicalRadius();

    if (R <= rs) {
        return 1e10f;  // Collapsed to black hole!
    }

    double factor = 1.0 - rs / R;
    if (factor <= 0.0) {
        return 1e10f;
    }

    return static_cast<float>(1.0 / std::sqrt(factor) - 1.0);
}

float NeutronStar::getSurfaceGravity() const {
    // g = GM/R²
    // In units of Earth's gravity (9.8 m/s²)
    float M_kg = properties_.mass * SOLAR_MASS_KG;
    float R_m = properties_.radius * KM_TO_M;

    float g_surface = G * M_kg / (R_m * R_m);
    float g_earth = 9.8f;

    return g_surface / g_earth;
}

} // namespace cosmic
