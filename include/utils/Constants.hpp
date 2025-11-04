#pragma once

#include <cmath>

namespace cosmic {

// Physical constants in geometric units (G = c = 1)
namespace constants {
    // Speed of light (for reference, = 1 in geometric units)
    constexpr double SPEED_OF_LIGHT = 1.0;

    // Gravitational constant (= 1 in geometric units)
    constexpr double GRAVITATIONAL_CONSTANT = 1.0;

    // Planck constant (cgs units)
    constexpr double PLANCK = 6.626e-27; // erg·s

    // Mathematical constants
    constexpr double PI = 3.14159265358979323846;
    constexpr double TWO_PI = 2.0 * PI;
    constexpr double HALF_PI = 0.5 * PI;

    // Conversion factors (for reference)
    // 1 solar mass in geometric units ≈ 1.477 km ≈ 4.926×10^-6 seconds
    constexpr double SOLAR_MASS_TO_KM = 1.477;
    constexpr double SOLAR_MASS_TO_SECONDS = 4.926e-6;

    // Simulation parameters
    constexpr double DEFAULT_BLACK_HOLE_MASS = 1.0; // In solar masses
    constexpr double MIN_BLACK_HOLE_MASS = 0.1;
    constexpr double MAX_BLACK_HOLE_MASS = 100.0;

    // Integration parameters
    constexpr double DEFAULT_STEP_SIZE = 0.1;
    constexpr double MIN_STEP_SIZE = 0.001;
    constexpr double MAX_STEP_SIZE = 1.0;
    constexpr int MAX_INTEGRATION_STEPS = 2000;
    constexpr double NULL_CONSTRAINT_TOLERANCE = 1e-8;
    constexpr double ADAPTIVE_TOLERANCE = 1e-6;
    constexpr double ADAPTIVE_SAFETY_FACTOR = 0.9;

    // Rendering parameters
    constexpr double ESCAPE_RADIUS = 100.0; // Ray escapes if r > this
    constexpr double EPSILON = 1e-6;        // Small value for comparisons
}

// Helper functions
namespace math {
    inline double square(double x) {
        return x * x;
    }

    inline double cube(double x) {
        return x * x * x;
    }

    template<typename T>
    inline T clamp(T value, T min, T max) {
        return value < min ? min : (value > max ? max : value);
    }
}

} // namespace cosmic
