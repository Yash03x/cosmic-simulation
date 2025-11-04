#pragma once

#include "Metric.hpp"
#include "../utils/Constants.hpp"
#include <Eigen/Dense>
#include <vector>
#include <functional>

namespace cosmic {
namespace physics {

/**
 * @brief Advanced accretion disk model with full radiative transfer
 *
 * Implements physics-accurate accretion disk models including:
 * - Shakura-Sunyaev thin disk
 * - Advection-dominated accretion flow (ADAF)
 * - Slim disk (for super-Eddington accretion)
 * - Multi-opacity radiative transfer
 * - Vertical structure
 * - Magnetic pressure support
 */
class AccretionDisk {
public:
    /**
     * @brief Disk model types
     */
    enum class Model {
        ShakuraSunyaev,  // Standard thin disk
        ADAF,            // Advection-dominated (hot, geometrically thick)
        SlimDisk,        // Super-Eddington, radiation-pressure dominated
        GRMHD            // General relativistic MHD (simplified)
    };

    /**
     * @brief Physical parameters at a disk location
     */
    struct DiskProperties {
        double surfaceDensity;        // Σ (g/cm²)
        double temperature;           // Midplane temperature (K)
        double effectiveTemperature;  // Effective radiating temperature (K)
        double scaleHeight;           // H/r ratio
        double pressure;              // Total pressure (dyn/cm²)
        double density;               // Midplane density (g/cm³)
        double opacity;               // Rosseland mean opacity (cm²/g)
        double opticalDepth;          // Vertical optical depth τ
        double soundSpeed;            // Sound speed (cm/s)
        double viscosity;             // Kinematic viscosity ν (cm²/s)
        double accretionVelocity;     // Radial velocity (cm/s)
        double angularVelocity;       // Orbital frequency Ω (rad/s)
        Eigen::Vector3d velocity;     // 3D velocity field
        Eigen::Vector3d magneticField;// Magnetic field (Gauss)
    };

    /**
     * @brief Opacity sources
     */
    struct OpacityComponents {
        double electronScattering;    // Thomson scattering (0.4 cm²/g)
        double freeFreeBound;         // Free-free and bound-free
        double molecules;             // Molecular opacity (cool regions)
        double dust;                  // Dust grains (< 1500 K)
        double hMinusIon;             // H⁻ ion (stellar atmospheres)
        double total;                 // Combined Rosseland mean
    };

    /**
     * @brief Disk configuration parameters
     */
    struct Configuration {
        Model model = Model::ShakuraSunyaev;
        double mass = 1.0;                    // Black hole mass (M_sun)
        double accretionRate = 0.1;           // Ṁ (Eddington units)
        double viscosityAlpha = 0.1;          // α parameter
        double innerRadius = 6.0;             // r_inner (GM/c²)
        double outerRadius = 1000.0;          // r_outer (GM/c²)
        double inclination = 0.0;             // Disk inclination (rad)
        double spinParameter = 0.0;           // Black hole spin a/M
        bool includeMagneticFields = false;   // Include B fields
        bool includeRadiationPressure = true; // Include P_rad
        bool includeAdvection = false;        // Include radial advection
    };

    explicit AccretionDisk(const Metric* metric, const Configuration& config);

    /**
     * @brief Compute disk properties at given radius
     *
     * Solves the coupled equations for disk structure:
     * - Hydrostatic equilibrium in vertical direction
     * - Energy balance (viscous heating = radiative cooling)
     * - Angular momentum transport
     * - Equation of state
     *
     * @param radius Cylindrical radius (geometric units)
     * @return Disk properties at that radius
     */
    DiskProperties propertiesAtRadius(double radius) const;

    /**
     * @brief Compute multi-frequency opacity
     *
     * Computes frequency-dependent opacity including:
     * - Thomson scattering (gray, κ_es = 0.4 cm²/g)
     * - Free-free absorption (Kramers law)
     * - Bound-free (photoionization)
     * - Molecular lines (cool disks)
     * - Dust (T < 1500 K)
     *
     * @param temperature Local temperature (K)
     * @param density Local density (g/cm³)
     * @param frequency Photon frequency (Hz)
     * @return Opacity at that frequency (cm²/g)
     */
    double opacityAtFrequency(double temperature,
                             double density,
                             double frequency) const;

    /**
     * @brief Compute Rosseland mean opacity
     *
     * κ_R = ∫ (∂B/∂T) dν / ∫ (∂B/∂T) / κ_ν dν
     *
     * where B is Planck function
     *
     * @param temperature Temperature (K)
     * @param density Density (g/cm³)
     * @return Rosseland mean opacity (cm²/g)
     */
    double rosselandMeanOpacity(double temperature, double density) const;

    /**
     * @brief Compute all opacity components separately
     *
     * @param temperature Temperature (K)
     * @param density Density (g/cm³)
     * @return Breakdown of opacity sources
     */
    OpacityComponents opacityBreakdown(double temperature,
                                       double density) const;

    /**
     * @brief Compute spectral energy distribution (SED)
     *
     * Integrates emission from entire disk to produce
     * observed spectrum at Earth
     *
     * @param numFrequencies Number of frequency bins
     * @param minFrequency Minimum frequency (Hz)
     * @param maxFrequency Maximum frequency (Hz)
     * @param inclination Observer inclination (rad)
     * @return Vector of (frequency, flux) pairs
     */
    std::vector<std::pair<double, double>> computeSED(
        int numFrequencies,
        double minFrequency,
        double maxFrequency,
        double inclination) const;

    /**
     * @brief Compute disk luminosity
     *
     * L = η Ṁ c² where η is radiative efficiency
     * η ≈ 0.06 for Schwarzschild, up to 0.42 for maximal Kerr
     *
     * @return Bolometric luminosity (erg/s)
     */
    double totalLuminosity() const;

    /**
     * @brief Compute radiative efficiency
     *
     * η = 1 - E_ISCO where E_ISCO is specific energy at ISCO
     *
     * @return Efficiency (0 to 1)
     */
    double radiativeEfficiency() const;

    /**
     * @brief Compute Eddington ratio
     *
     * L/L_Edd where L_Edd = 4πGMm_p c / σ_T
     *
     * @return Ratio L/L_Edd
     */
    double eddingtonRatio() const;

    /**
     * @brief Check if disk is radiation pressure dominated
     *
     * Compares radiation pressure P_rad = aT⁴/3 with gas pressure P_gas = ρkT/μm_p
     *
     * @param radius Radius to check
     * @return True if P_rad > P_gas
     */
    bool isRadiationDominated(double radius) const;

    /**
     * @brief Compute vertical density profile
     *
     * ρ(z) = ρ_0 exp(-z²/2H²) for Gaussian profile
     * or more realistic profile from vertical structure integration
     *
     * @param radius Cylindrical radius
     * @param height Height above midplane z
     * @return Density at (r, z)
     */
    double verticalDensity(double radius, double height) const;

    /**
     * @brief Compute vertical temperature profile
     *
     * T(z) varies from T_midplane to T_effective at photosphere
     *
     * @param radius Cylindrical radius
     * @param height Height above midplane
     * @return Temperature at (r, z)
     */
    double verticalTemperature(double radius, double height) const;

    /**
     * @brief Compute magnetic field structure
     *
     * For MHD simulations, approximate toroidal and poloidal components
     * B_φ ∝ sqrt(8πα P) (turbulent field from MRI)
     *
     * @param radius Cylindrical radius
     * @param height Height above midplane
     * @return Magnetic field vector (B_r, B_θ, B_φ) in Gauss
     */
    Eigen::Vector3d magneticField(double radius, double height) const;

    /**
     * @brief Compute stress tensor component T^r_φ
     *
     * T^r_φ = α (P + P_mag) where P is pressure, P_mag = B²/8π
     *
     * @param radius Radius
     * @return Stress (dyn/cm²)
     */
    double stressTensor(double radius) const;

    /**
     * @brief Solve vertical structure self-consistently
     *
     * Integrates hydrostatic balance + energy transport to find
     * T(z), ρ(z), P(z) profiles
     *
     * @param radius Radius
     * @param numVerticalPoints Number of grid points in z
     * @return Vector of (z, T, ρ, P) profiles
     */
    std::vector<std::array<double, 4>> verticalStructure(
        double radius,
        int numVerticalPoints = 100) const;

    /**
     * @brief Compute cooling timescale
     *
     * t_cool = E_thermal / L_radiative
     *
     * @param radius Radius
     * @return Cooling time (seconds)
     */
    double coolingTimescale(double radius) const;

    /**
     * @brief Compute viscous timescale
     *
     * t_visc = r² / ν where ν = α c_s H
     *
     * @param radius Radius
     * @return Viscous time (seconds)
     */
    double viscousTimescale(double radius) const;

    /**
     * @brief Check if disk is thermally stable
     *
     * Checks S-curve stability criterion: d(log T_c) / d(log Σ) > 0
     *
     * @param radius Radius
     * @return True if thermally stable
     */
    bool isThermallyStable(double radius) const;

    /**
     * @brief Get current configuration
     */
    const Configuration& getConfiguration() const { return config_; }

    /**
     * @brief Update configuration
     */
    void setConfiguration(const Configuration& config);

private:
    const Metric* metric_;
    Configuration config_;

    // Physical constants (cgs units)
    static constexpr double M_SUN = 1.989e33;          // g
    static constexpr double G_GRAV = 6.674e-8;         // cm³/g/s²
    static constexpr double C_LIGHT = 2.998e10;        // cm/s
    static constexpr double SIGMA_SB = 5.670e-5;       // erg/cm²/s/K⁴
    static constexpr double K_BOLTZMANN = 1.381e-16;   // erg/K
    static constexpr double M_PROTON = 1.673e-24;      // g
    static constexpr double SIGMA_THOMSON = 6.652e-25; // cm²
    static constexpr double A_RAD = 7.566e-15;         // erg/cm³/K⁴

    /**
     * @brief Helper: Convert geometric to physical units
     */
    double geometricToPhysical(double r_geom) const;

    /**
     * @brief Helper: Solve Shakura-Sunyaev equations
     */
    DiskProperties shakuraSunyaevModel(double radius) const;

    /**
     * @brief Helper: Solve ADAF equations
     */
    DiskProperties adafModel(double radius) const;

    /**
     * @brief Helper: Solve slim disk equations
     */
    DiskProperties slimDiskModel(double radius) const;

    /**
     * @brief Helper: Compute Kramers opacity
     */
    double kramersOpacity(double temperature, double density) const;

    /**
     * @brief Helper: Compute H⁻ opacity
     */
    double hMinusOpacity(double temperature, double density) const;
};

} // namespace physics
} // namespace cosmic
