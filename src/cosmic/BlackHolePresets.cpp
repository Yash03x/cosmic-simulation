#include "cosmic/BlackHolePresets.hpp"
#include <stdexcept>

namespace cosmic {

std::vector<BlackHolePreset> BlackHolePresets::getAllPresets() {
    return {
        m87(),
        sgr_a_star(),
        cygnus_x1(),
        gw150914_final(),
        stellar_mass(),
        intermediate_mass(),
        supermassive(),
        maximal_kerr()
    };
}

BlackHolePreset BlackHolePresets::getPreset(const std::string& name) {
    auto presets = getAllPresets();
    for (const auto& preset : presets) {
        if (preset.name == name) {
            return preset;
        }
    }
    throw std::runtime_error("Preset not found: " + name);
}

std::unique_ptr<physics::Metric> BlackHolePresets::createMetric(const BlackHolePreset& preset) {
    if (std::abs(preset.spin) < 1e-6) {
        return std::make_unique<physics::Schwarzschild>(preset.mass);
    } else {
        return std::make_unique<physics::Kerr>(preset.mass, preset.spin);
    }
}

BlackHolePreset BlackHolePresets::m87() {
    return {
        "M87*",
        "First black hole ever photographed by Event Horizon Telescope (2019). "
        "Supermassive black hole at the center of galaxy M87, 55 million light-years away. "
        "Shadow diameter: 42 microarcseconds.",
        6.5e9f,      // 6.5 billion solar masses
        0.9f,        // High spin (likely, based on jet power)
        16800.0f,    // 55 million light-years = 16.8 Mpc
        0.001f,      // Low accretion rate (dim compared to AGN)
        true,        // Famous relativistic jet
        15.0f,       // View from 15M
        5.0f         // Slightly above equator to see disk
    };
}

BlackHolePreset BlackHolePresets::sgr_a_star() {
    return {
        "Sgr A*",
        "Supermassive black hole at the center of our Milky Way galaxy. "
        "Second black hole photographed by EHT (2022). "
        "Distance: 26,000 light-years from Earth.",
        4.15e6f,     // 4.15 million solar masses (Nobel Prize 2020 measurement)
        0.5f,        // Moderate spin (uncertain)
        8.127f,      // 26,000 light-years = 8.127 kpc
        1e-7f,       // Very low accretion rate (very dim)
        false,       // No prominent jet
        20.0f,       // View from 20M
        10.0f        // Higher elevation to see quiet disk
    };
}

BlackHolePreset BlackHolePresets::cygnus_x1() {
    return {
        "Cygnus X-1",
        "First confirmed black hole (1972). X-ray binary system with a blue supergiant star. "
        "Stellar-mass black hole actively accreting from companion. "
        "Distance: ~6,000 light-years.",
        21.2f,       // 21 solar masses (LIGO/GWTC refined estimate)
        0.95f,       // Very high spin (from X-ray spectroscopy)
        2.0f,        // 6,000 light-years = 1.8 kpc
        0.5f,        // High accretion rate (bright X-ray source)
        true,        // Radio jets observed
        25.0f,       // View from 25M
        8.0f         // Medium elevation
    };
}

BlackHolePreset BlackHolePresets::gw150914_final() {
    return {
        "GW150914 (Final)",
        "First gravitational wave detection (2015). "
        "Final black hole from merger of 36 + 29 solar mass black holes. "
        "3 solar masses radiated as gravitational waves!",
        62.0f,       // 36 + 29 - 3 = 62 solar masses
        0.67f,       // Moderate final spin
        410.0f,      // 1.3 billion light-years
        0.0f,        // No accretion disk (isolated)
        false,       // No jets (no material)
        30.0f,       // View from 30M
        15.0f        // Higher elevation (no disk to see)
    };
}

BlackHolePreset BlackHolePresets::stellar_mass() {
    return {
        "Stellar-Mass (10 M☉)",
        "Typical stellar-mass black hole from supernova. "
        "Progenitor star: 20-25 solar masses. "
        "Tidal forces: humans torn apart before crossing horizon!",
        10.0f,       // 10 solar masses (typical)
        0.5f,        // Moderate spin
        1.0f,        // Hypothetical nearby
        0.0f,        // Isolated (no companion)
        false,       // No jets
        35.0f,       // View from 35M
        10.0f        // Medium elevation
    };
}

BlackHolePreset BlackHolePresets::intermediate_mass() {
    return {
        "Intermediate-Mass",
        "Rare class of black holes between stellar and supermassive. "
        "May form from stellar collisions in dense clusters. "
        "Mass: 100-100,000 solar masses.",
        10000.0f,    // 10,000 solar masses
        0.7f,        // High spin
        10.0f,       // Hypothetical in nearby cluster
        0.01f,       // Moderate accretion
        true,        // May have jets
        20.0f,       // View from 20M
        7.0f         // Medium elevation
    };
}

BlackHolePreset BlackHolePresets::supermassive() {
    return {
        "Supermassive (1B M☉)",
        "Typical supermassive black hole in galaxy center. "
        "Tidal forces: comfortable crossing of event horizon! "
        "Mass: millions to billions of solar masses.",
        1e9f,        // 1 billion solar masses
        0.8f,        // High spin (typical for AGN)
        1000.0f,     // Distant galaxy
        0.1f,        // Moderate accretion (AGN)
        true,        // Powerful jets
        15.0f,       // View from 15M
        5.0f         // Low elevation to see disk
    };
}

BlackHolePreset BlackHolePresets::maximal_kerr() {
    return {
        "Maximal Kerr (a/M=0.998)",
        "Theoretical black hole spinning at near-maximal rate. "
        "ISCO: 1.24M (incredibly close!). "
        "Frame-dragging: extreme. Energy extraction via Penrose process possible!",
        1.0f,        // 1 solar mass (for simplicity)
        0.998f,      // Near-maximal spin
        1.0f,        // Hypothetical
        0.2f,        // High accretion
        true,        // Strong jets
        12.0f,       // View from 12M (close!)
        4.0f         // Low elevation
    };
}

} // namespace cosmic
