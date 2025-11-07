#include "modules/galaxy/GalaxyPresets.hpp"

namespace cosmic {
namespace modules {
namespace galaxy {

GalaxyProperties GalaxyPresets::getMilkyWay() {
    GalaxyProperties props;

    props.name = "Milky Way";
    props.description = "Our home galaxy - a barred spiral with ~200-400 billion stars";
    props.type = GalaxyType::Barred;

    // Physical properties (based on latest observations)
    props.totalMass = 1.5e12;           // 1.5 trillion solar masses
    props.stellarMass = 6.0e10;         // 60 billion solar masses in stars
    props.darkMatterMass = 1.44e12;     // ~96% is dark matter
    props.radius = 15.0;                // 15 kpc visible disk radius
    props.scaleHeight = 0.3;            // 300 pc disk thickness
    props.centralBulgeRadius = 3.0;     // 3 kpc bulge

    // Dynamics (measured at Sun's position, R = 8.5 kpc)
    props.rotationVelocity = 220.0;     // 220 km/s (Sun's orbital velocity)
    props.velocityDispersion = 30.0;    // Velocity dispersion in disk

    // Observable properties
    props.numStarsSimulated = 50000;    // Representative sample for real-time
    props.actualStarCount = 2.0e11;     // ~200 billion stars (conservative estimate)
    props.luminosity = 2.5e10;          // 25 billion solar luminosities

    // Morphology
    props.numSpiralArms = 4;            // 4 major spiral arms
    props.armPitch = 12.0;              // 12-degree pitch angle
    props.barLength = 5.0;              // ~5 kpc bar half-length

    return props;
}

GalaxyProperties GalaxyPresets::getAndromeda() {
    GalaxyProperties props;

    props.name = "Andromeda (M31)";
    props.description = "Nearest large spiral galaxy - will collide with Milky Way in 4 Gyr";
    props.type = GalaxyType::Spiral;

    // Physical properties
    props.totalMass = 1.5e12;           // 1.5 trillion solar masses (similar to MW)
    props.stellarMass = 1.0e11;         // 100 billion solar masses in stars
    props.darkMatterMass = 1.4e12;      // Dark matter halo
    props.radius = 21.0;                // 21 kpc disk radius (larger than MW)
    props.scaleHeight = 0.35;           // 350 pc
    props.centralBulgeRadius = 4.5;     // Larger bulge than MW

    // Dynamics
    props.rotationVelocity = 225.0;     // ~225 km/s at comparable radius
    props.velocityDispersion = 35.0;

    // Observable properties
    props.numStarsSimulated = 75000;    // Larger sample for larger galaxy
    props.actualStarCount = 1.0e12;     // ~1 trillion stars
    props.luminosity = 2.6e10;          // Slightly more luminous than MW

    // Morphology
    props.numSpiralArms = 2;            // 2 major spiral arms
    props.armPitch = 15.0;              // Looser spiral
    props.barLength = 0.0;              // Unbarred

    return props;
}

GalaxyProperties GalaxyPresets::getM87() {
    GalaxyProperties props;

    props.name = "M87 (Virgo A)";
    props.description = "Giant elliptical galaxy with 6.5 billion M☉ black hole (EHT 2019)";
    props.type = GalaxyType::Elliptical;

    // Physical properties
    props.totalMass = 6.5e12;           // 6.5 trillion solar masses
    props.stellarMass = 2.4e12;         // 2.4 trillion solar masses in stars
    props.darkMatterMass = 4.1e12;      // Dark matter halo
    props.radius = 18.5;                // 18.5 kpc effective radius
    props.scaleHeight = 18.5;           // Spheroidal (height ≈ radius)
    props.centralBulgeRadius = 8.0;     // Large core

    // Dynamics (elliptical - no rotation, high dispersion)
    props.rotationVelocity = 0.0;       // Minimal net rotation
    props.velocityDispersion = 350.0;   // High velocity dispersion (350 km/s)

    // Observable properties
    props.numStarsSimulated = 60000;
    props.actualStarCount = 1.0e12;     // ~1 trillion stars
    props.luminosity = 1.3e11;          // Much more luminous (old stars)

    // Morphology (elliptical - no spiral structure)
    props.numSpiralArms = 0;
    props.armPitch = 0.0;
    props.barLength = 0.0;

    return props;
}

GalaxyProperties GalaxyPresets::getNGC1300() {
    GalaxyProperties props;

    props.name = "NGC 1300";
    props.description = "Classic barred spiral galaxy - poster child for SB(s)bc type";
    props.type = GalaxyType::Barred;

    // Physical properties
    props.totalMass = 9.0e11;           // 900 billion solar masses
    props.stellarMass = 7.0e10;         // 70 billion solar masses
    props.darkMatterMass = 8.3e11;
    props.radius = 16.5;                // 16.5 kpc
    props.scaleHeight = 0.4;            // 400 pc
    props.centralBulgeRadius = 2.0;     // Small bulge

    // Dynamics
    props.rotationVelocity = 200.0;     // ~200 km/s
    props.velocityDispersion = 25.0;

    // Observable properties
    props.numStarsSimulated = 40000;
    props.actualStarCount = 4.0e11;     // ~400 billion stars
    props.luminosity = 1.8e10;

    // Morphology (prominent bar)
    props.numSpiralArms = 2;            // 2 main arms
    props.armPitch = 24.0;              // Open spiral
    props.barLength = 6.75;             // 13.5 kpc bar full length = 6.75 kpc half

    return props;
}

GalaxyProperties GalaxyPresets::getTriangulum() {
    GalaxyProperties props;

    props.name = "Triangulum (M33)";
    props.description = "Third-largest in Local Group - small unbarred spiral";
    props.type = GalaxyType::Spiral;

    // Physical properties (smaller galaxy)
    props.totalMass = 5.0e10;           // 50 billion solar masses
    props.stellarMass = 3.0e9;          // 3 billion solar masses
    props.darkMatterMass = 4.7e10;
    props.radius = 9.0;                 // 9 kpc
    props.scaleHeight = 0.2;            // 200 pc (thin disk)
    props.centralBulgeRadius = 0.5;     // Very small bulge

    // Dynamics
    props.rotationVelocity = 100.0;     // ~100 km/s (slower rotation)
    props.velocityDispersion = 15.0;

    // Observable properties
    props.numStarsSimulated = 20000;    // Smaller sample
    props.actualStarCount = 4.0e10;     // ~40 billion stars
    props.luminosity = 4.0e9;           // 4 billion solar luminosities

    // Morphology
    props.numSpiralArms = 2;
    props.armPitch = 18.0;
    props.barLength = 0.0;              // Unbarred

    return props;
}

GalaxyProperties GalaxyPresets::getSombrero() {
    GalaxyProperties props;

    props.name = "Sombrero (M104)";
    props.description = "Edge-on spiral with prominent dust lane and large bulge";
    props.type = GalaxyType::Spiral;

    // Physical properties
    props.totalMass = 8.0e11;           // 800 billion solar masses
    props.stellarMass = 1.0e11;         // 100 billion solar masses
    props.darkMatterMass = 7.0e11;
    props.radius = 7.5;                 // 7.5 kpc (compact disk)
    props.scaleHeight = 0.15;           // 150 pc (very thin - seen edge-on)
    props.centralBulgeRadius = 4.0;     // Unusually large bulge

    // Dynamics
    props.rotationVelocity = 250.0;     // Fast rotation
    props.velocityDispersion = 40.0;    // Higher dispersion (large bulge)

    // Observable properties
    props.numStarsSimulated = 35000;
    props.actualStarCount = 1.0e11;     // ~100 billion stars
    props.luminosity = 2.5e10;

    // Morphology
    props.numSpiralArms = 2;            // Tight spiral
    props.armPitch = 8.0;               // Very tight
    props.barLength = 0.0;

    return props;
}

GalaxyProperties GalaxyPresets::getLMC() {
    GalaxyProperties props;

    props.name = "Large Magellanic Cloud";
    props.description = "Irregular satellite galaxy of the Milky Way";
    props.type = GalaxyType::Irregular;

    // Physical properties (dwarf galaxy)
    props.totalMass = 1.0e10;           // 10 billion solar masses
    props.stellarMass = 3.0e9;          // 3 billion solar masses
    props.darkMatterMass = 7.0e9;
    props.radius = 2.15;                // 2.15 kpc (14,000 light-years)
    props.scaleHeight = 0.5;            // Irregular morphology
    props.centralBulgeRadius = 0.3;     // Minimal bulge

    // Dynamics (irregular - chaotic motions)
    props.rotationVelocity = 60.0;      // Some rotation
    props.velocityDispersion = 20.0;    // Lower dispersion

    // Observable properties
    props.numStarsSimulated = 15000;    // Smallest sample
    props.actualStarCount = 3.0e10;     // ~30 billion stars
    props.luminosity = 2.0e9;           // 2 billion solar luminosities

    // Morphology (irregular - minimal structure)
    props.numSpiralArms = 1;            // Hint of bar
    props.armPitch = 0.0;
    props.barLength = 1.5;              // Small bar

    return props;
}

std::vector<GalaxyProperties> GalaxyPresets::getAllPresets() {
    return {
        getMilkyWay(),
        getAndromeda(),
        getM87(),
        getNGC1300(),
        getTriangulum(),
        getSombrero(),
        getLMC()
    };
}

} // namespace galaxy
} // namespace modules
} // namespace cosmic
