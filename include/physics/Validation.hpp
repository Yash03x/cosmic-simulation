#pragma once

#include "Metric.hpp"
#include "Geodesic.hpp"
#include "Schwarzschild.hpp"
#include "Kerr.hpp"
#include "../utils/Constants.hpp"
#include <Eigen/Dense>
#include <vector>
#include <string>
#include <functional>

namespace cosmic {
namespace physics {

/**
 * @brief Physics validation suite
 *
 * Tests simulation results against known analytical solutions
 * and published results from the literature to ensure accuracy
 */
class PhysicsValidator {
public:
    /**
     * @brief Validation test result
     */
    struct TestResult {
        std::string testName;
        bool passed;
        double computedValue;
        double expectedValue;
        double relativeError;
        double tolerance;
        std::string notes;

        TestResult(const std::string& name, bool pass, double computed,
                  double expected, double error, double tol, const std::string& n = "")
            : testName(name), passed(pass), computedValue(computed),
              expectedValue(expected), relativeError(error), tolerance(tol), notes(n) {}
    };

    /**
     * @brief Test suite summary
     */
    struct ValidationReport {
        std::vector<TestResult> results;
        int totalTests;
        int passedTests;
        int failedTests;
        double averageError;

        void addResult(const TestResult& result) {
            results.push_back(result);
            totalTests++;
            if (result.passed) {
                passedTests++;
            } else {
                failedTests++;
            }
        }

        void computeStatistics() {
            if (totalTests > 0) {
                double sum = 0.0;
                for (const auto& r : results) {
                    sum += r.relativeError;
                }
                averageError = sum / totalTests;
            }
        }
    };

    PhysicsValidator();

    /**
     * @brief Run all validation tests
     *
     * @return Complete validation report
     */
    ValidationReport runAllTests();

    /**
     * @brief Test Schwarzschild metric properties
     */
    ValidationReport testSchwarzschildMetric();

    /**
     * @brief Test Kerr metric properties
     */
    ValidationReport testKerrMetric();

    /**
     * @brief Test geodesic integration accuracy
     */
    ValidationReport testGeodesicIntegration();

    /**
     * @brief Test conservation laws
     */
    ValidationReport testConservationLaws();

    /**
     * @brief Test analytical orbits
     */
    ValidationReport testOrbitalMechanics();

    /**
     * @brief Test against published black hole images
     */
    ValidationReport testImageComparison();

    // Individual test methods

    /**
     * @brief Test that Schwarzschild radius is correct
     *
     * r_s = 2GM/c² = 2M in geometric units
     */
    TestResult testSchwarzschildRadius();

    /**
     * @brief Test photon sphere radius
     *
     * Schwarzschild: r_ph = 3M
     * Kerr: depends on spin and direction
     */
    TestResult testPhotonSphereRadius(double spin = 0.0);

    /**
     * @brief Test ISCO radius
     *
     * Schwarzschild: r_ISCO = 6M
     * Kerr prograde (a=0.998): r_ISCO ≈ 1.24M
     * Kerr retrograde (a=-0.998): r_ISCO ≈ 8.88M
     */
    TestResult testISCORadius(double spin = 0.0);

    /**
     * @brief Test circular photon orbit stability
     *
     * Photons at r = 3M should orbit (but unstably)
     */
    TestResult testCircularPhotonOrbit();

    /**
     * @brief Test geodesic energy conservation
     *
     * E = -p_t should be constant along geodesic
     */
    TestResult testEnergyConservation();

    /**
     * @brief Test angular momentum conservation
     *
     * L = p_φ should be constant for axisymmetric spacetime
     */
    TestResult testAngularMomentumConservation();

    /**
     * @brief Test null geodesic constraint
     *
     * g_μν p^μ p^ν = 0 for photon
     */
    TestResult testNullGeodesicConstraint();

    /**
     * @brief Test gravitational redshift formula
     *
     * 1 + z = √(g_tt(∞) / g_tt(r))
     * For Schwarzschild: 1 + z = 1/√(1 - 2M/r)
     */
    TestResult testGravitationalRedshift();

    /**
     * @brief Test light bending angle
     *
     * For Schwarzschild, light bending for impact parameter b:
     * Δφ ≈ 4M/b for b >> M (weak field)
     * Δφ = π for b ≈ 2.6M (strong field)
     */
    TestResult testLightDeflection();

    /**
     * @brief Test Shapiro time delay
     *
     * Δt ≈ 2M ln(4r/b) for light passing at impact parameter b
     */
    TestResult testShapiroDelay();

    /**
     * @brief Test Kerr frame dragging
     *
     * ω = dφ/dt for ZAMO should match analytical formula
     * ω = 2Mra / (r³ + a²r + 2Ma²)
     */
    TestResult testFrameDragging(double spin);

    /**
     * @brief Test epicyclic frequencies
     *
     * For Schwarzschild at r = 10M:
     * ν_r = ν_K √(1 - 6M/r) ≈ 0.632 ν_K
     */
    TestResult testEpicyclicFrequencies();

    /**
     * @brief Test against Bardeen et al. (1972) ISCO results
     */
    TestResult testBardeenISCO();

    /**
     * @brief Test against Cunningham (1975) transfer function
     */
    TestResult testCunninghamTransferFunction();

    /**
     * @brief Test against Fabian et al. (1989) iron line profile
     */
    TestResult testFabianIronLine();

    /**
     * @brief Test against EHT M87 shadow size
     *
     * Shadow radius should be ≈ 2.6 √(27) M ≈ 5.2M for Schwarzschild
     */
    TestResult testShadowRadius(double spin = 0.0);

    /**
     * @brief Test Christoffel symbol accuracy
     *
     * Verify Γ^μ_αβ computed numerically matches analytical formulas
     */
    TestResult testChristoffelSymbols();

    /**
     * @brief Test metric symmetries
     *
     * - Time translation: ∂g/∂t = 0
     * - Axial symmetry: ∂g/∂φ = 0
     */
    TestResult testMetricSymmetries();

    /**
     * @brief Test against Novikov-Thorne disk model
     */
    TestResult testNovikovThorneDisk();

    /**
     * @brief Test RK integrator convergence
     *
     * Check that RK4/RK5 converges at correct order
     */
    TestResult testIntegratorConvergence();

private:
    /**
     * @brief Helper: Compare with tolerance
     */
    bool compareWithTolerance(double computed, double expected,
                             double tolerance, double& relError);

    /**
     * @brief Helper: Integrate circular photon orbit
     */
    bool integrateCircularOrbit(const Metric* metric,
                               double radius,
                               double& finalRadius,
                               double& numOrbits);

    /**
     * @brief Helper: Compute light deflection angle
     */
    double computeDeflectionAngle(const Metric* metric,
                                 double impactParameter);

    /**
     * @brief Helper: Analytical Schwarzschild deflection
     */
    double analyticalDeflection(double mass, double impactParameter);
};

/**
 * @brief Benchmark suite for performance testing
 */
class PerformanceBenchmark {
public:
    struct BenchmarkResult {
        std::string testName;
        double timeSeconds;
        int iterations;
        double iterationsPerSecond;
        std::string notes;
    };

    PerformanceBenchmark();

    /**
     * @brief Benchmark geodesic integration speed
     */
    BenchmarkResult benchmarkGeodesicIntegration(int numGeodesics);

    /**
     * @brief Benchmark ray tracing performance
     */
    BenchmarkResult benchmarkRayTracing(int imageWidth, int imageHeight);

    /**
     * @brief Benchmark Christoffel symbol computation
     */
    BenchmarkResult benchmarkChristoffelComputation(int numEvaluations);

    /**
     * @brief Run all benchmarks
     */
    std::vector<BenchmarkResult> runAllBenchmarks();

private:
    double getCurrentTime() const;
};

} // namespace physics
} // namespace cosmic
