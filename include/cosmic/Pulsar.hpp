#pragma once

#include "NeutronStar.hpp"
#include <glm/glm.hpp>
#include <vector>

namespace cosmic {

/**
 * @brief Pulsar beam properties
 *
 * Pulsars emit beams of electromagnetic radiation from their magnetic poles.
 * As the neutron star rotates, these beams sweep across space like a lighthouse.
 */
struct PulsarBeamProperties {
    float beamWidth;          // Opening angle (degrees)
    float beamLength;         // Visible length (M)
    glm::vec3 beamColor;      // Color (usually radio/optical/X-ray)
    float intensity;          // Beam brightness
    float pulseFrequency;     // Observed pulse rate (Hz)

    // Beam pattern
    enum class Pattern {
        Cone,           // Hollow cone
        PencilBeam,     // Narrow pencil beam
        FanBeam         // Wide fan beam
    } pattern;
};

/**
 * @brief Single beam segment for rendering
 */
struct BeamSegment {
    glm::vec3 startPos;
    glm::vec3 endPos;
    glm::vec3 color;
    float intensity;
};

/**
 * @brief Pulsar - neutron star with visible beams
 *
 * Combines neutron star physics with lighthouse beam visualization.
 * Famous examples:
 * - Crab Pulsar: 30 Hz pulses
 * - Vela Pulsar: 11 Hz pulses
 * - PSR B1919+21: First pulsar discovered (1967)
 */
class Pulsar {
public:
    /**
     * @brief Construct pulsar from neutron star
     */
    explicit Pulsar(const NeutronStarProperties& nsProps);

    /**
     * @brief Update pulsar state (rotation, beams)
     */
    void update(float deltaTime);

    /**
     * @brief Get beam segments for rendering
     */
    std::vector<BeamSegment> getBeamSegments() const;

    /**
     * @brief Get underlying neutron star
     */
    const NeutronStar& getNeutronStar() const { return neutronStar_; }
    NeutronStar& getNeutronStar() { return neutronStar_; }

    /**
     * @brief Get beam properties
     */
    const PulsarBeamProperties& getBeamProperties() const { return beamProps_; }

    /**
     * @brief Set beam properties
     */
    void setBeamProperties(const PulsarBeamProperties& props) { beamProps_ = props; }

    /**
     * @brief Check if beams are visible
     */
    bool areBeamsVisible() const { return beamsVisible_; }

    /**
     * @brief Set beam visibility
     */
    void setBeamsVisible(bool visible) { beamsVisible_ = visible; }

    /**
     * @brief Get current rotation phase [0, 2π]
     */
    float getRotationPhase() const;

    /**
     * @brief Check if we're in the beam path (lighthouse on)
     */
    bool isBeamPointingAtObserver(const glm::vec3& observerPos) const;

    /**
     * @brief Get pulse profile (intensity vs phase)
     */
    float getPulseIntensity() const;

    /**
     * @brief Set position
     */
    void setPosition(const glm::vec3& position) {
        neutronStar_.setPosition(position);
    }

    /**
     * @brief Get position
     */
    glm::vec3 getPosition() const {
        return neutronStar_.getPosition();
    }

    /**
     * @brief Set visibility
     */
    void setVisible(bool visible) {
        neutronStar_.setVisible(visible);
    }

    /**
     * @brief Check if visible
     */
    bool isVisible() const {
        return neutronStar_.isVisible();
    }

private:
    NeutronStar neutronStar_;
    PulsarBeamProperties beamProps_;
    bool beamsVisible_;

    /**
     * @brief Generate beam segments from magnetic poles
     */
    void generateBeamSegments(std::vector<BeamSegment>& segments) const;

    /**
     * @brief Calculate beam direction at current rotation
     */
    glm::vec3 getBeamDirection(bool northPole) const;
};

/**
 * @brief Pulsar presets from famous observations
 */
class PulsarPresets {
public:
    /**
     * @brief Get default beam properties for a pulsar
     */
    static PulsarBeamProperties getDefaultBeamProperties();

    /**
     * @brief Get beam properties optimized for specific pulsar types
     */
    static PulsarBeamProperties getCrabBeamProperties();
    static PulsarBeamProperties getVelaBeamProperties();
    static PulsarBeamProperties getMillisecondBeamProperties();
    static PulsarBeamProperties getMagnetarBeamProperties();
};

} // namespace cosmic
