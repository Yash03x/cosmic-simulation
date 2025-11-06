#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <memory>

namespace cosmic {

// Forward declarations
namespace physics {
    class Metric;
}

namespace tools {

/**
 * @brief Initial conditions for a particle
 */
struct ParticleInitialConditions {
    glm::vec3 position;      // Initial position (x, y, z)
    glm::vec3 velocity;      // Initial velocity (dx/dt, dy/dt, dz/dt)
    float energy;            // Specific energy (E/m)
    float angularMomentum;   // Specific angular momentum (L/m)
    bool isMassive;          // True for massive particles, false for photons
};

/**
 * @brief Single trajectory point
 */
struct TrajectoryPoint {
    glm::vec3 position;
    glm::vec3 velocity;
    float properTime;        // Proper time along trajectory
    float coordinateTime;    // Coordinate time
};

/**
 * @brief A single particle trajectory
 */
class ParticleTrajectory {
public:
    /**
     * @brief Create a new trajectory
     * @param initialConditions Starting state
     * @param metric Black hole metric
     * @param maxPoints Maximum number of trajectory points
     */
    ParticleTrajectory(const ParticleInitialConditions& initialConditions,
                       const physics::Metric* metric,
                       size_t maxPoints = 5000);

    /**
     * @brief Integrate trajectory forward one step
     * @param stepSize Integration step size
     * @return True if integration succeeded, false if trajectory ended
     */
    bool integrate(float stepSize);

    /**
     * @brief Integrate trajectory for specified time
     * @param totalTime Total coordinate time to integrate
     * @param stepSize Integration step size
     */
    void integrateForTime(float totalTime, float stepSize = 0.01f);

    /**
     * @brief Get all trajectory points
     */
    const std::vector<TrajectoryPoint>& getPoints() const { return points_; }

    /**
     * @brief Get initial conditions
     */
    const ParticleInitialConditions& getInitialConditions() const { return initialConditions_; }

    /**
     * @brief Check if trajectory hit event horizon
     */
    bool hitEventHorizon() const { return hitEventHorizon_; }

    /**
     * @brief Check if trajectory escaped to infinity
     */
    bool escaped() const { return escaped_; }

    /**
     * @brief Get current position
     */
    glm::vec3 getCurrentPosition() const {
        return points_.empty() ? glm::vec3(0.0f) : points_.back().position;
    }

    /**
     * @brief Get trajectory color (for rendering)
     */
    glm::vec3 getColor() const { return color_; }

    /**
     * @brief Set trajectory color
     */
    void setColor(const glm::vec3& color) { color_ = color; }

    /**
     * @brief Check if trajectory is active (still integrating)
     */
    bool isActive() const { return !hitEventHorizon_ && !escaped_ && points_.size() < maxPoints_; }

private:
    ParticleInitialConditions initialConditions_;
    const physics::Metric* metric_;
    std::vector<TrajectoryPoint> points_;
    size_t maxPoints_;
    bool hitEventHorizon_;
    bool escaped_;
    glm::vec3 color_;

    /**
     * @brief Calculate geodesic acceleration in curved spacetime
     * @param position Current position
     * @param velocity Current 4-velocity
     * @return 4-acceleration
     */
    glm::vec3 calculateGeodesicAcceleration(const glm::vec3& position,
                                             const glm::vec3& velocity) const;

    /**
     * @brief Convert initial conditions to proper 4-velocity
     */
    glm::vec3 normalizeVelocity(const glm::vec3& position,
                                 const glm::vec3& velocity) const;
};

/**
 * @brief Manager for multiple particle trajectories
 */
class ParticleSystem {
public:
    ParticleSystem();

    /**
     * @brief Add a new particle with initial conditions
     */
    void addParticle(const ParticleInitialConditions& initialConditions,
                     const physics::Metric* metric);

    /**
     * @brief Add a circular orbit at specified radius
     * @param radius Orbital radius
     * @param metric Black hole metric
     */
    void addCircularOrbit(float radius, const physics::Metric* metric);

    /**
     * @brief Add an elliptical orbit
     * @param periapsis Closest approach distance
     * @param apoapsis Farthest distance
     * @param metric Black hole metric
     */
    void addEllipticalOrbit(float periapsis, float apoapsis,
                            const physics::Metric* metric);

    /**
     * @brief Add a radial infall trajectory
     * @param startRadius Starting radius
     * @param metric Black hole metric
     */
    void addRadialInfall(float startRadius, const physics::Metric* metric);

    /**
     * @brief Update all active trajectories
     * @param deltaTime Time step
     */
    void update(float deltaTime);

    /**
     * @brief Clear all trajectories
     */
    void clear() { trajectories_.clear(); }

    /**
     * @brief Get all trajectories
     */
    const std::vector<std::unique_ptr<ParticleTrajectory>>& getTrajectories() const {
        return trajectories_;
    }

    /**
     * @brief Set whether system is active (integrating)
     */
    void setActive(bool active) { active_ = active; }

    /**
     * @brief Check if system is active
     */
    bool isActive() const { return active_; }

    /**
     * @brief Get number of trajectories
     */
    size_t getTrajectoryCount() const { return trajectories_.size(); }

private:
    std::vector<std::unique_ptr<ParticleTrajectory>> trajectories_;
    bool active_;
    float integrationStepSize_;
};

} // namespace tools
} // namespace cosmic
