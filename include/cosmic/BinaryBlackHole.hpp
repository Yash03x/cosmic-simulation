#pragma once

#include <glm/glm.hpp>
#include <string>
#include <memory>

namespace cosmic {

/**
 * @brief Properties for a binary black hole system
 */
struct BinaryBlackHoleProperties {
    std::string name;
    float mass1;              // Mass of first BH (solar masses)
    float mass2;              // Mass of second BH (solar masses)
    float initialSeparation;  // Initial orbital separation (M)
    float eccentricity;       // Orbital eccentricity (0 = circular)
    float inclination;        // Orbital inclination (radians)
    float mergerTime;         // Time to merger (seconds)

    // Calculated properties
    float totalMass() const { return mass1 + mass2; }
    float reducedMass() const { return (mass1 * mass2) / totalMass(); }
    float chirpMass() const {
        return std::pow(mass1 * mass2, 0.6f) / std::pow(totalMass(), 0.2f);
    }
    float massRatio() const { return mass2 / mass1; }  // Assumes mass1 >= mass2
};

/**
 * @brief Preset binary black hole systems based on real LIGO/Virgo detections
 */
class BinaryBlackHolePresets {
public:
    static BinaryBlackHoleProperties getGW150914();    // First detection
    static BinaryBlackHoleProperties getGW170814();    // First 3-detector
    static BinaryBlackHoleProperties getGW190521();    // Intermediate mass
    static BinaryBlackHoleProperties getGW190412();    // Unequal masses
    static BinaryBlackHoleProperties getGenericCircular();  // Generic circular orbit
    static BinaryBlackHoleProperties getGenericEccentric(); // Generic eccentric orbit
};

/**
 * @brief Orbital state of binary black hole system
 */
enum class BinaryState {
    Inspiral,      // Gradual inspiral due to GW emission
    Merger,        // Rapid plunge and merger
    Ringdown,      // Final black hole settling down
    Complete       // Merger complete, single BH remains
};

/**
 * @brief Binary black hole system simulation
 *
 * Simulates two black holes orbiting each other, emitting gravitational waves,
 * and eventually merging into a single black hole.
 */
class BinaryBlackHole {
public:
    /**
     * @brief Construct binary system from properties
     */
    explicit BinaryBlackHole(const BinaryBlackHoleProperties& props);

    /**
     * @brief Update orbital dynamics
     * @param deltaTime Time step
     */
    void update(float deltaTime);

    /**
     * @brief Reset to initial conditions
     */
    void reset();

    /**
     * @brief Get position of first black hole
     */
    glm::vec3 getPosition1() const { return position1_; }

    /**
     * @brief Get position of second black hole
     */
    glm::vec3 getPosition2() const { return position2_; }

    /**
     * @brief Get current orbital separation
     */
    float getSeparation() const;

    /**
     * @brief Get current orbital velocity
     */
    float getOrbitalVelocity() const;

    /**
     * @brief Get current orbital frequency (Hz)
     */
    float getOrbitalFrequency() const { return orbitalFrequency_; }

    /**
     * @brief Get gravitational wave frequency (Hz)
     * GW frequency is twice orbital frequency for circular orbits
     */
    float getGWFrequency() const { return 2.0f * orbitalFrequency_; }

    /**
     * @brief Get current gravitational wave strain amplitude
     */
    float getGWStrain() const;

    /**
     * @brief Get current state
     */
    BinaryState getState() const { return state_; }

    /**
     * @brief Get time until merger (seconds)
     */
    float getTimeToMerger() const;

    /**
     * @brief Get properties
     */
    const BinaryBlackHoleProperties& getProperties() const { return props_; }

    /**
     * @brief Check if system is visible
     */
    bool isVisible() const { return visible_; }

    /**
     * @brief Set visibility
     */
    void setVisible(bool visible) { visible_ = visible; }

    /**
     * @brief Get mass of final merged black hole
     */
    float getFinalMass() const;

    /**
     * @brief Get spin of final merged black hole
     */
    float getFinalSpin() const;

    /**
     * @brief Get total energy radiated in gravitational waves
     */
    float getEnergyRadiated() const;

    /**
     * @brief Get current phase of orbit (radians)
     */
    float getOrbitalPhase() const { return orbitalPhase_; }

private:
    BinaryBlackHoleProperties props_;

    // Orbital state
    glm::vec3 position1_;
    glm::vec3 position2_;
    glm::vec3 velocity1_;
    glm::vec3 velocity2_;

    float separation_;           // Current orbital separation
    float orbitalFrequency_;     // Current orbital frequency (Hz)
    float orbitalPhase_;         // Current orbital phase (radians)
    float elapsedTime_;          // Total elapsed time

    BinaryState state_;
    bool visible_;

    // Merger state
    float mergerStartTime_;
    float ringdownStartTime_;
    glm::vec3 finalPosition_;
    float finalMass_;
    float finalSpin_;

    /**
     * @brief Calculate inspiral rate due to GW emission
     * Using Peters-Mathews formula for circular orbits
     */
    float calculateInspiralRate() const;

    /**
     * @brief Update inspiral phase
     */
    void updateInspiral(float deltaTime);

    /**
     * @brief Update merger phase
     */
    void updateMerger(float deltaTime);

    /**
     * @brief Update ringdown phase
     */
    void updateRingdown(float deltaTime);

    /**
     * @brief Calculate positions from orbital parameters
     */
    void updatePositions();

    /**
     * @brief Calculate final black hole properties
     */
    void calculateFinalProperties();
};

} // namespace cosmic
