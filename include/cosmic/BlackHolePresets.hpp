#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../physics/Metric.hpp"
#include "../physics/Kerr.hpp"
#include "../physics/Schwarzschild.hpp"

namespace cosmic {

/**
 * @brief Preset configuration for famous black holes
 */
struct BlackHolePreset {
    std::string name;
    std::string description;
    float mass;              // in solar masses
    float spin;              // dimensionless spin parameter a/M
    float distance;          // from Earth in kpc
    float accretionRate;     // Mdot in solar masses per year
    bool hasJets;

    // Camera presets
    float cameraDistance;    // initial viewing distance in M
    float cameraHeight;      // initial height above equatorial plane in M
};

/**
 * @brief Factory for creating famous black hole configurations
 */
class BlackHolePresets {
public:
    /**
     * @brief Get all available presets
     */
    static std::vector<BlackHolePreset> getAllPresets();

    /**
     * @brief Get preset by name
     */
    static BlackHolePreset getPreset(const std::string& name);

    /**
     * @brief Create metric from preset
     */
    static std::unique_ptr<physics::Metric> createMetric(const BlackHolePreset& preset);

    // Famous black hole presets
    static BlackHolePreset m87();
    static BlackHolePreset sgr_a_star();
    static BlackHolePreset cygnus_x1();
    static BlackHolePreset gw150914_final();
    static BlackHolePreset stellar_mass();
    static BlackHolePreset intermediate_mass();
    static BlackHolePreset supermassive();
    static BlackHolePreset maximal_kerr();
};

} // namespace cosmic
