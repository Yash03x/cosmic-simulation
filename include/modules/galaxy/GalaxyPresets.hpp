#pragma once

#include <string>
#include <vector>

namespace cosmic {
namespace modules {
namespace galaxy {

/**
 * @brief Galaxy types based on Hubble classification
 */
enum class GalaxyType {
    Spiral,      // Spiral galaxy (Milky Way, M31)
    Elliptical,  // Elliptical galaxy (M87)
    Irregular,   // Irregular galaxy (Large Magellanic Cloud)
    Barred       // Barred spiral galaxy (NGC 1300)
};

/**
 * @brief Real galaxy properties based on astronomical observations
 */
struct GalaxyProperties {
    std::string name;
    std::string description;
    GalaxyType type;

    // Physical properties (real astronomical values)
    double totalMass;           // Total mass (solar masses)
    double stellarMass;         // Stellar mass (solar masses)
    double darkMatterMass;      // Dark matter mass (solar masses)
    double radius;              // Disk radius (kiloparsecs)
    double scaleHeight;         // Disk thickness (kiloparsecs)
    double centralBulgeRadius;  // Bulge radius (kiloparsecs)

    // Dynamics
    double rotationVelocity;    // Rotation velocity at R=8.5kpc (km/s)
    double velocityDispersion;  // Velocity dispersion for ellipticals (km/s)

    // Observable properties
    int numStarsSimulated;      // Number of stars in simulation (sample)
    long long actualStarCount;  // Actual estimated star count
    double luminosity;          // Bolometric luminosity (solar luminosities)

    // Morphology
    int numSpiralArms;          // Number of spiral arms (0 for non-spiral)
    double armPitch;            // Spiral arm pitch angle (degrees)
    double barLength;           // Bar length for barred spirals (kpc)
};

/**
 * @brief Famous galaxy presets based on real observations
 */
class GalaxyPresets {
public:
    /**
     * @brief Milky Way - Our home galaxy
     *
     * Properties from latest observations (2024):
     * - Mass: ~1.5 trillion solar masses (including dark matter)
     * - Diameter: ~100,000 light-years (30 kpc)
     * - Stars: ~100-400 billion
     * - Type: SBbc (barred spiral)
     * - Rotation at Sun's position: ~220 km/s
     */
    static GalaxyProperties getMilkyWay();

    /**
     * @brief Andromeda Galaxy (M31) - Our nearest large neighbor
     *
     * Properties:
     * - Mass: ~1.5 trillion solar masses
     * - Diameter: ~220,000 light-years (67 kpc)
     * - Stars: ~1 trillion
     * - Type: SA(s)b (unbarred spiral)
     * - Distance: 2.5 million light-years
     * - On collision course with Milky Way (4 billion years)
     */
    static GalaxyProperties getAndromeda();

    /**
     * @brief M87 - Giant elliptical galaxy
     *
     * Properties:
     * - Mass: ~6.5 trillion solar masses
     * - Diameter: ~120,000 light-years (37 kpc)
     * - Stars: ~1 trillion
     * - Type: E0-1 (elliptical)
     * - Central black hole: 6.5 billion solar masses (EHT 2019)
     * - Distance: 53.5 million light-years
     */
    static GalaxyProperties getM87();

    /**
     * @brief NGC 1300 - Classic barred spiral
     *
     * Properties:
     * - Mass: ~900 billion solar masses
     * - Diameter: ~110,000 light-years (33 kpc)
     * - Stars: ~400 billion
     * - Type: SB(s)bc (barred spiral)
     * - Bar length: ~44,000 light-years (13.5 kpc)
     * - Distance: 61 million light-years
     */
    static GalaxyProperties getNGC1300();

    /**
     * @brief Triangulum Galaxy (M33) - Small spiral
     *
     * Properties:
     * - Mass: ~50 billion solar masses
     * - Diameter: ~60,000 light-years (18 kpc)
     * - Stars: ~40 billion
     * - Type: SA(s)cd (unbarred spiral)
     * - Third-largest in Local Group
     * - Distance: 2.7 million light-years
     */
    static GalaxyProperties getTriangulum();

    /**
     * @brief Sombrero Galaxy (M104) - Edge-on spiral
     *
     * Properties:
     * - Mass: ~800 billion solar masses
     * - Diameter: ~50,000 light-years (15 kpc)
     * - Stars: ~100 billion
     * - Type: SA(s)a (unbarred spiral)
     * - Prominent dust lane
     * - Distance: 29.3 million light-years
     */
    static GalaxyProperties getSombrero();

    /**
     * @brief Large Magellanic Cloud - Irregular satellite
     *
     * Properties:
     * - Mass: ~10 billion solar masses
     * - Diameter: ~14,000 light-years (4.3 kpc)
     * - Stars: ~30 billion
     * - Type: Irr/SB(s)m (irregular/barred)
     * - Satellite of Milky Way
     * - Distance: 163,000 light-years
     */
    static GalaxyProperties getLMC();

    /**
     * @brief Get all available presets
     */
    static std::vector<GalaxyProperties> getAllPresets();
};

} // namespace galaxy
} // namespace modules
} // namespace cosmic
