#pragma once

#include "../physics/Metric.hpp"
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace cosmic {

/**
 * @brief Neutron star properties and physics
 *
 * Neutron stars are compact objects with:
 * - Mass: 1.4-2.1 M☉ (solar masses)
 * - Radius: ~10-12 km
 * - Density: ~10^17 kg/m³
 * - Surface gravity: ~10^12 m/s²
 * - Escape velocity: ~0.6c
 */
struct NeutronStarProperties {
    std::string name;
    std::string description;

    // Physical properties
    float mass;                   // Solar masses
    float radius;                 // Kilometers
    float rotationPeriod;         // Seconds (millisecond pulsars: 0.001-0.01s)
    float magneticFieldStrength;  // Tesla (10^8 to 10^11 T)
    float surfaceTemperature;     // Kelvin (10^5 to 10^6 K)

    // Visual properties
    glm::vec3 surfaceColor;       // Base color
    bool hasHotspots;             // Magnetic pole hotspots
    bool hasMountains;            // Surface irregularities (< 5mm tall!)
    float crustalDeformation;     // Oblateness from rotation

    // Camera defaults
    float cameraDistance;
    float cameraHeight;
};

/**
 * @brief Neutron star presets based on real observations
 */
class NeutronStarPresets {
public:
    /**
     * @brief Get all available neutron star presets
     */
    static std::vector<NeutronStarProperties> getAllPresets();

    /**
     * @brief Get specific preset by name
     */
    static NeutronStarProperties getPreset(const std::string& name);

    // Individual presets
    static NeutronStarProperties getPSRJ1748();      // Fastest known pulsar
    static NeutronStarProperties getCrabPulsar();    // Famous supernova remnant
    static NeutronStarProperties getVelaPulsar();    // Nearby, well-studied
    static NeutronStarProperties getMagnetar();      // Extreme magnetic field
    static NeutronStarProperties getMillisecond();   // Millisecond pulsar
    static NeutronStarProperties getTolmanOV();      // Maximum mass limit
};

/**
 * @brief Neutron star object for rendering and physics
 */
class NeutronStar {
public:
    /**
     * @brief Construct neutron star from properties
     */
    explicit NeutronStar(const NeutronStarProperties& properties);

    /**
     * @brief Update neutron star state
     * @param deltaTime Time since last update
     */
    void update(float deltaTime);

    /**
     * @brief Get current rotation angle (radians)
     */
    float getRotationAngle() const { return rotationAngle_; }

    /**
     * @brief Get rotation axis (normalized)
     */
    glm::vec3 getRotationAxis() const { return rotationAxis_; }

    /**
     * @brief Get magnetic axis (normalized, relative to rotation axis)
     */
    glm::vec3 getMagneticAxis() const { return magneticAxis_; }

    /**
     * @brief Get hotspot positions (magnetic poles)
     */
    std::vector<glm::vec3> getHotspots() const;

    /**
     * @brief Get properties
     */
    const NeutronStarProperties& getProperties() const { return properties_; }

    /**
     * @brief Get Schwarzschild radius (for gravitational lensing)
     */
    double getSchwarzschildRadius() const;

    /**
     * @brief Get physical radius in geometric units (M)
     */
    double getPhysicalRadius() const;

    /**
     * @brief Calculate surface redshift
     * Surface redshift: z = 1/sqrt(1 - 2GM/(Rc²)) - 1
     */
    float getSurfaceRedshift() const;

    /**
     * @brief Calculate surface gravity (in units of Earth gravity)
     */
    float getSurfaceGravity() const;

    /**
     * @brief Get position
     */
    glm::vec3 getPosition() const { return position_; }

    /**
     * @brief Set position
     */
    void setPosition(const glm::vec3& position) { position_ = position; }

    /**
     * @brief Check if neutron star is visible
     */
    bool isVisible() const { return visible_; }

    /**
     * @brief Set visibility
     */
    void setVisible(bool visible) { visible_ = visible; }

private:
    NeutronStarProperties properties_;
    glm::vec3 position_;
    bool visible_;

    // Rotation state
    float rotationAngle_;         // Current rotation angle (radians)
    float angularVelocity_;       // rad/s
    glm::vec3 rotationAxis_;      // Normalized rotation axis
    glm::vec3 magneticAxis_;      // Magnetic field axis (misaligned with rotation)

    // Constants
    static constexpr float SOLAR_MASS_KG = 1.989e30f;     // kg
    static constexpr float KM_TO_M = 1000.0f;              // m/km
    static constexpr float G = 6.674e-11f;                 // m³/(kg·s²)
    static constexpr float C = 2.998e8f;                   // m/s
};

} // namespace cosmic
