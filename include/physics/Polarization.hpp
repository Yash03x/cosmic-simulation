#pragma once

#include "Metric.hpp"
#include "Geodesic.hpp"
#include "../utils/Constants.hpp"
#include <Eigen/Dense>
#include <complex>

namespace cosmic {
namespace physics {

/**
 * @brief Parallel transport of polarization vectors in curved spacetime
 *
 * For electromagnetic radiation propagating through curved spacetime,
 * the polarization vector must be parallel transported along the
 * photon geodesic to properly account for gravitational effects.
 *
 * The parallel transport equation is:
 * Dπ^μ/dλ = -Γ^μ_αβ k^α π^β = 0
 *
 * where π^μ is the polarization four-vector, k^μ is the photon momentum,
 * and λ is the affine parameter.
 */
class PolarizationTransport {
public:
    /**
     * @brief Polarization state
     */
    struct PolarizationState {
        Metric::FourVector position;         // Current position
        Metric::FourVector photonMomentum;   // Photon four-momentum k^μ
        Metric::FourVector polarization;     // Polarization four-vector π^μ
        double stokesI;                      // Stokes I (total intensity)
        double stokesQ;                      // Stokes Q (linear polarization)
        double stokesU;                      // Stokes U (linear polarization)
        double stokesV;                      // Stokes V (circular polarization)
        double affineParameter;              // Affine parameter

        PolarizationState()
            : position(Metric::FourVector::Zero()),
              photonMomentum(Metric::FourVector::Zero()),
              polarization(Metric::FourVector::Zero()),
              stokesI(1.0),
              stokesQ(0.0),
              stokesU(0.0),
              stokesV(0.0),
              affineParameter(0.0) {}
    };

    /**
     * @brief Walker-Penrose constant for Kerr spacetime
     *
     * In Kerr spacetime, there exists a constant of motion for
     * parallel-transported polarization: κ = π_μ K^μ where K^μ
     * is a Killing vector
     */
    struct WalkerPenroseConstant {
        std::complex<double> kappa;  // Complex constant κ
        bool isConserved;             // Whether κ is conserved (to numerical precision)
        double conservationError;     // |κ(λ) - κ(0)|
    };

    explicit PolarizationTransport(const Metric* metric);

    /**
     * @brief Parallel transport polarization along geodesic
     *
     * Integrates the parallel transport equation:
     * dπ^μ/dλ + Γ^μ_αβ k^α π^β = 0
     *
     * @param state Initial polarization state
     * @param stepSize Integration step size
     * @return Updated polarization state
     */
    void transportStep(PolarizationState& state, double stepSize);

    /**
     * @brief Compute Stokes parameters from polarization vector
     *
     * The Stokes parameters (I, Q, U, V) describe the polarization
     * state in terms of measurable intensities:
     * - I: Total intensity
     * - Q: Linear polarization (0° vs 90°)
     * - U: Linear polarization (45° vs 135°)
     * - V: Circular polarization (left vs right)
     *
     * @param polarization Polarization four-vector
     * @param photonMomentum Photon four-momentum
     * @param observerVelocity Observer four-velocity
     * @param I Output Stokes I
     * @param Q Output Stokes Q
     * @param U Output Stokes U
     * @param V Output Stokes V
     */
    void computeStokesParameters(const Metric::FourVector& polarization,
                                 const Metric::FourVector& photonMomentum,
                                 const Metric::FourVector& observerVelocity,
                                 double& I, double& Q, double& U, double& V) const;

    /**
     * @brief Construct orthonormal tetrad for observer
     *
     * Creates a local Lorentz frame for an observer with given
     * four-velocity. The tetrad consists of four orthonormal vectors:
     * e_0 (timelike), e_1, e_2, e_3 (spacelike)
     *
     * @param position Observer position
     * @param velocity Observer four-velocity
     * @return Array of 4 tetrad vectors
     */
    std::array<Metric::FourVector, 4> constructTetrad(
        const Metric::FourVector& position,
        const Metric::FourVector& velocity) const;

    /**
     * @brief Project polarization to observer frame
     *
     * Projects the parallel-transported polarization vector into
     * the observer's local frame using the observer tetrad
     *
     * @param polarization Covariant polarization vector
     * @param tetrad Observer tetrad
     * @return 3-vector in observer frame
     */
    Eigen::Vector3d projectToObserverFrame(
        const Metric::FourVector& polarization,
        const std::array<Metric::FourVector, 4>& tetrad) const;

    /**
     * @brief Compute polarization rotation angle
     *
     * Computes the rotation of the polarization plane as photon
     * travels from emitter to observer (Faraday rotation + gravitational)
     *
     * @param initialState State at emission
     * @param finalState State at observation
     * @return Rotation angle in radians
     */
    double polarizationRotationAngle(const PolarizationState& initialState,
                                     const PolarizationState& finalState) const;

    /**
     * @brief Compute Walker-Penrose constant (Kerr only)
     *
     * For Kerr spacetime, the Walker-Penrose constant is conserved
     * along geodesics with parallel-transported polarization
     *
     * @param state Polarization state
     * @return Walker-Penrose constant κ
     */
    WalkerPenroseConstant computeWalkerPenrose(const PolarizationState& state) const;

    /**
     * @brief Initialize polarization vector orthogonal to momentum
     *
     * Creates an initial polarization vector satisfying:
     * - π · k = 0 (transverse to propagation)
     * - π · π = 0 (null vector for photon)
     *
     * @param photonMomentum Photon four-momentum
     * @param position Current position
     * @param polarizationAngle Angle in plane perpendicular to k
     * @return Polarization four-vector
     */
    Metric::FourVector initializePolarization(
        const Metric::FourVector& photonMomentum,
        const Metric::FourVector& position,
        double polarizationAngle = 0.0) const;

    /**
     * @brief Compute Faraday rotation from magnetized plasma
     *
     * Rotation angle = RM * λ^2
     * where RM is rotation measure, λ is wavelength
     *
     * RM = (e^3 / 2πm_e^2 c^4) ∫ n_e B·dl
     *
     * @param electronDensity Electron density (cm^-3)
     * @param magneticField Magnetic field strength (Gauss)
     * @param pathLength Path length through medium
     * @param wavelength Wavelength (cm)
     * @return Rotation angle (radians)
     */
    double faradayRotation(double electronDensity,
                          double magneticField,
                          double pathLength,
                          double wavelength) const;

    /**
     * @brief Compute polarization fraction from accretion disk
     *
     * Synchrotron radiation from accretion disk has intrinsic
     * polarization fraction depending on:
     * - Magnetic field geometry
     * - Optical depth
     * - Scattering
     *
     * @param magneticFieldStrength B field in Gauss
     * @param opticalDepth Thomson optical depth
     * @param temperature Electron temperature (K)
     * @return Linear polarization fraction (0 to 1)
     */
    double diskPolarizationFraction(double magneticFieldStrength,
                                   double opticalDepth,
                                   double temperature) const;

private:
    const Metric* metric_;

    /**
     * @brief Compute derivative of polarization for parallel transport
     */
    Metric::FourVector computePolarizationDerivative(
        const Metric::FourVector& position,
        const Metric::FourVector& photonMomentum,
        const Metric::FourVector& polarization) const;

    /**
     * @brief Gram-Schmidt orthogonalization
     */
    void orthogonalize(Metric::FourVector& vec,
                      const Metric::FourVector& against,
                      const Metric::FourVector& position) const;

    /**
     * @brief Compute inner product using metric
     */
    double innerProduct(const Metric::FourVector& a,
                       const Metric::FourVector& b,
                       const Metric::FourVector& position) const;
};

/**
 * @brief Synchrotron radiation polarization calculator
 *
 * Computes polarization signatures from synchrotron emission
 * in accretion disk magnetic fields
 */
class SynchrotronPolarization {
public:
    struct EmissionProperties {
        double totalIntensity;        // Total emission I
        double polarizationFraction;  // Fractional polarization
        double polarizationAngle;     // EVPA (Electric Vector Position Angle)
        double spectralIndex;         // Spectral index α (I ∝ ν^α)
        Eigen::Vector3d magneticField;// Local B field direction
    };

    explicit SynchrotronPolarization(const Metric* metric);

    /**
     * @brief Compute synchrotron emission properties
     *
     * @param position Emission position
     * @param magneticField B field vector (Gauss)
     * @param electronEnergy Electron Lorentz factor γ
     * @param viewingAngle Angle to observer
     * @param frequency Observing frequency (Hz)
     * @return Emission properties including polarization
     */
    EmissionProperties computeEmission(
        const Metric::FourVector& position,
        const Eigen::Vector3d& magneticField,
        double electronEnergy,
        double viewingAngle,
        double frequency) const;

    /**
     * @brief Compute polarization map of accretion disk
     *
     * Ray-traces and computes Stokes parameters for each image pixel
     *
     * @param inclination Observer inclination
     * @param imageWidth Image width (pixels)
     * @param imageHeight Image height (pixels)
     * @return 2D array of Stokes (I, Q, U, V)
     */
    std::vector<std::vector<std::array<double, 4>>>
        polarizationMap(double inclination,
                       int imageWidth,
                       int imageHeight) const;

private:
    const Metric* metric_;
};

} // namespace physics
} // namespace cosmic
