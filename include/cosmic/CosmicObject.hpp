#pragma once

#include "../physics/Metric.hpp"
#include <Eigen/Dense>
#include <memory>
#include <string>

namespace cosmic {

/**
 * @brief Base class for all cosmic objects in the simulation
 *
 * This provides a common interface for:
 * - Black holes (Schwarzschild, Kerr)
 * - Neutron stars
 * - Pulsars
 * - White dwarfs
 * - Wormholes (theoretical)
 */
class CosmicObject {
public:
    enum class Type {
        BlackHole,      // Schwarzschild or Kerr
        NeutronStar,    // Neutron star with surface
        Pulsar,         // Rotating neutron star with beams
        WhiteDwarf,     // Degenerate matter star
        Wormhole        // Theoretical traversable wormhole
    };

    virtual ~CosmicObject() = default;

    /**
     * @brief Get the type of cosmic object
     */
    virtual Type getType() const = 0;

    /**
     * @brief Get human-readable name
     */
    virtual std::string getName() const = 0;

    /**
     * @brief Get mass in solar masses
     */
    virtual double getMass() const = 0;

    /**
     * @brief Get radius in geometric units
     */
    virtual double getRadius() const = 0;

    /**
     * @brief Get the spacetime metric if applicable
     * @return Pointer to metric, or nullptr if no strong-field metric
     */
    virtual const physics::Metric* getMetric() const = 0;

    /**
     * @brief Check if object has a visible surface
     */
    virtual bool hasSurface() const = 0;

    /**
     * @brief Get surface temperature at given coordinates (if has surface)
     * @param theta Polar angle
     * @param phi Azimuthal angle
     * @return Temperature in Kelvin, or 0 if no surface
     */
    virtual double surfaceTemperature(double theta, double phi) const {
        return 0.0;
    }

    /**
     * @brief Check if object has magnetic field
     */
    virtual bool hasMagneticField() const { return false; }

    /**
     * @brief Get magnetic field strength at surface (Gauss)
     */
    virtual double surfaceMagneticField() const { return 0.0; }

    /**
     * @brief Check if object is rotating
     */
    virtual bool isRotating() const = 0;

    /**
     * @brief Get rotation period in seconds
     * @return Period, or infinity if not rotating
     */
    virtual double rotationPeriod() const = 0;
};

/**
 * @brief Black hole implementation
 */
class BlackHole : public CosmicObject {
public:
    explicit BlackHole(std::unique_ptr<physics::Metric> metric,
                       const std::string& name = "Black Hole");

    Type getType() const override { return Type::BlackHole; }
    std::string getName() const override { return name_; }
    double getMass() const override;
    double getRadius() const override; // Event horizon
    const physics::Metric* getMetric() const override { return metric_.get(); }
    bool hasSurface() const override { return false; } // No surface!
    bool isRotating() const override;
    double rotationPeriod() const override;

private:
    std::unique_ptr<physics::Metric> metric_;
    std::string name_;
};

/**
 * @brief Neutron star (future implementation)
 *
 * Properties:
 * - Mass: 1-2 M☉
 * - Radius: ~10-15 km
 * - Surface temperature: 10⁵ - 10⁶ K
 * - Surface gravity: ~10¹² g_earth
 * - Magnetic field: 10⁸ - 10¹⁵ Gauss
 */
class NeutronStar : public CosmicObject {
public:
    explicit NeutronStar(double mass, double radius, double temperature);

    Type getType() const override { return Type::NeutronStar; }
    std::string getName() const override { return "Neutron Star"; }
    double getMass() const override { return mass_; }
    double getRadius() const override { return radius_; }
    const physics::Metric* getMetric() const override { return nullptr; } // TODO: Add relativistic metric
    bool hasSurface() const override { return true; }
    double surfaceTemperature(double theta, double phi) const override;
    bool hasMagneticField() const override { return true; }
    double surfaceMagneticField() const override { return B_surface_; }
    bool isRotating() const override { return period_ < 1e10; }
    double rotationPeriod() const override { return period_; }

    void setRotationPeriod(double period) { period_ = period; }
    void setMagneticField(double B) { B_surface_ = B; }

private:
    double mass_;           // Solar masses
    double radius_;         // km
    double temperature_;    // K
    double period_;         // seconds
    double B_surface_;      // Gauss
};

/**
 * @brief Pulsar (rotating neutron star with beams)
 *
 * Emits beams of radiation from magnetic poles
 */
class Pulsar : public NeutronStar {
public:
    explicit Pulsar(double mass, double radius, double period);

    Type getType() const override { return Type::Pulsar; }
    std::string getName() const override { return "Pulsar"; }

    /**
     * @brief Get beam direction at given time
     * @param time Current time in seconds
     * @return Unit vector pointing to magnetic pole
     */
    Eigen::Vector3d beamDirection(double time) const;

    /**
     * @brief Check if beam is pointing toward observer at given time
     * @param time Current time
     * @param observerDir Direction to observer
     * @param coneAngle Beam opening angle (radians)
     * @return True if observer is in beam
     */
    bool isBeamVisible(double time,
                      const Eigen::Vector3d& observerDir,
                      double coneAngle = 0.1) const;

    /**
     * @brief Get magnetic axis tilt angle (degrees)
     */
    double getMagneticTilt() const { return tilt_; }

    void setMagneticTilt(double tilt) { tilt_ = tilt; }

private:
    double tilt_;  // Angle between rotation and magnetic axis (degrees)
};

/**
 * @brief Cosmic object factory
 */
class CosmicObjectFactory {
public:
    /**
     * @brief Create famous black hole examples
     */
    static std::unique_ptr<BlackHole> createCygnusX1();
    static std::unique_ptr<BlackHole> createSagittariusAStar();
    static std::unique_ptr<BlackHole> createM87();
    static std::unique_ptr<BlackHole> createGargantua(); // Interstellar

    /**
     * @brief Create famous neutron star/pulsar examples
     */
    static std::unique_ptr<Pulsar> createCrabPulsar();
    static std::unique_ptr<NeutronStar> createPSRJ0348();

    /**
     * @brief Create custom black hole
     */
    static std::unique_ptr<BlackHole> createBlackHole(
        double mass, double spin = 0.0, const std::string& name = "Black Hole");
};

} // namespace cosmic
