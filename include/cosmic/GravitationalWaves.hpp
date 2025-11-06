#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace cosmic {

/**
 * @brief Gravitational wave event properties
 *
 * Based on real LIGO/Virgo detections of binary mergers
 */
struct GWEventProperties {
    std::string name;
    std::string description;

    // Source properties
    float mass1;              // Primary mass (M☉)
    float mass2;              // Secondary mass (M☉)
    float finalMass;          // Final black hole mass (M☉)
    float energyRadiated;     // Energy in gravitational waves (M☉c²)
    float distance;           // Distance to source (megaparsecs)

    // Waveform properties
    float peakFrequency;      // Peak frequency (Hz)
    float peakStrain;         // Peak strain amplitude h (dimensionless)
    float chirpMass;          // Chirp mass M = (m1*m2)^(3/5) / (m1+m2)^(1/5)
    float duration;           // Observable duration (seconds)

    // Event type
    enum class Type {
        BinaryBlackHole,      // BBH
        BinaryNeutronStar,    // BNS
        NeutronStarBlackHole  // NSBH
    } type;

    // Visual properties
    glm::vec3 waveColor;
    float visualAmplification;  // Scale factor for visualization

    // Detection date
    std::string detectionDate;
};

/**
 * @brief Gravitational wave presets from real detections
 */
class GravitationalWavePresets {
public:
    static std::vector<GWEventProperties> getAllPresets();
    static GWEventProperties getPreset(const std::string& name);

    // Famous detections
    static GWEventProperties getGW150914();   // First detection!
    static GWEventProperties getGW170817();   // Neutron star merger (with EM counterpart)
    static GWEventProperties getGW190521();   // Intermediate mass black hole
    static GWEventProperties getGW190814();   // Mystery object (2.6 M☉)
    static GWEventProperties getGW200105();   // NSBH merger
    static GWEventProperties getGW200115();   // Another NSBH merger
};

/**
 * @brief Single gravitational wave pulse
 */
struct GWPulse {
    glm::vec3 origin;         // Source location
    float currentRadius;      // Current wave front radius
    float amplitude;          // Current amplitude (decays with distance)
    float frequency;          // Wave frequency
    float phase;              // Current phase
    float birthTime;          // When wave was emitted
    bool active;              // Is pulse still visible
};

/**
 * @brief Gravitational wave visualization system
 *
 * Visualizes spacetime ripples as:
 * - Expanding rings/shells
 * - Grid distortions
 * - Color-coded strain amplitude
 * - Frequency chirp (increasing pitch before merger)
 */
class GravitationalWaveSystem {
public:
    GravitationalWaveSystem();

    /**
     * @brief Set current GW event
     */
    void setEvent(const GWEventProperties& event);

    /**
     * @brief Update wave propagation
     */
    void update(float deltaTime);

    /**
     * @brief Emit a new gravitational wave pulse
     */
    void emitPulse();

    /**
     * @brief Get active wave pulses for rendering
     */
    const std::vector<GWPulse>& getPulses() const { return pulses_; }

    /**
     * @brief Clear all pulses
     */
    void clear() { pulses_.clear(); }

    /**
     * @brief Check if system is active
     */
    bool isActive() const { return active_; }

    /**
     * @brief Set system active state
     */
    void setActive(bool active) { active_ = active; }

    /**
     * @brief Get current event properties
     */
    const GWEventProperties& getEvent() const { return event_; }

    /**
     * @brief Get wave source position
     */
    glm::vec3 getSourcePosition() const { return sourcePosition_; }

    /**
     * @brief Set wave source position
     */
    void setSourcePosition(const glm::vec3& position) { sourcePosition_ = position; }

    /**
     * @brief Calculate strain amplitude at distance r
     * h(r) = (4G/c^4) * (E/r) where E is radiated energy
     */
    float calculateStrainAt(float distance) const;

    /**
     * @brief Get visualization mode
     */
    enum class VisualizationMode {
        Rings,           // Expanding rings
        Grid,            // Distorted grid
        RingsAndGrid     // Both
    };

    void setVisualizationMode(VisualizationMode mode) { visualizationMode_ = mode; }
    VisualizationMode getVisualizationMode() const { return visualizationMode_; }

    /**
     * @brief Enable/disable chirp (frequency increases before merger)
     */
    void setChirpEnabled(bool enabled) { chirpEnabled_ = enabled; }
    bool isChirpEnabled() const { return chirpEnabled_; }

    /**
     * @brief Get number of active pulses
     */
    size_t getActivePulseCount() const {
        size_t count = 0;
        for (const auto& pulse : pulses_) {
            if (pulse.active) count++;
        }
        return count;
    }

private:
    GWEventProperties event_;
    std::vector<GWPulse> pulses_;
    glm::vec3 sourcePosition_;
    bool active_;
    VisualizationMode visualizationMode_;
    bool chirpEnabled_;

    // Emission parameters
    float timeSinceLastEmission_;
    float emissionInterval_;        // Time between pulses

    // Wave propagation speed (c = 1 in geometric units)
    static constexpr float WAVE_SPEED = 1.0f;

    // Maximum visible radius
    static constexpr float MAX_RADIUS = 200.0f;

    // Constants for strain calculation
    static constexpr float G = 6.674e-11f;        // m^3/(kg·s^2)
    static constexpr float C = 2.998e8f;          // m/s
    static constexpr float SOLAR_MASS_KG = 1.989e30f;
};

} // namespace cosmic
