#pragma once

#include "Metric.hpp"
#include "../utils/Constants.hpp"
#include <Eigen/Dense>
#include <vector>
#include <functional>

namespace cosmic {
namespace physics {

/**
 * @brief Observational diagnostics for black hole systems
 *
 * Provides tools for computing observable quantities that can be
 * compared with real astrophysical observations:
 * - X-ray spectra and line profiles
 * - Transfer functions for reverberation mapping
 * - Light curves and variability
 * - Redshift/blueshift maps
 */
class Observables {
public:
    /**
     * @brief Photon four-momentum and trajectory information
     */
    struct Photon {
        Metric::FourVector position;      // Current position (t, r, θ, φ)
        Metric::FourVector momentum;      // Four-momentum p^μ
        double energy;                    // Observed energy at detector
        double emissionEnergy;            // Rest-frame emission energy
        double redshift;                  // Gravitational + Doppler redshift (1+z)
        double affineParameter;           // Affine parameter along geodesic
        double travelTime;                // Coordinate time traveled
        bool hitDisk;                     // Whether photon hit accretion disk
        Eigen::Vector3d emissionPoint;    // Emission coordinates (r, θ, φ)
        Eigen::Vector3d emissionVelocity; // Local velocity at emission
    };

    /**
     * @brief Iron K-alpha line parameters
     */
    struct IronLine {
        double restEnergy;       // Rest-frame energy (6.4 keV for neutral Fe)
        double width;            // Natural line width (eV)
        double equivalentWidth;  // Equivalent width in keV
        double emissivity;       // Radial emissivity index (typically 3)
    };

    /**
     * @brief X-ray spectrum binning
     */
    struct Spectrum {
        std::vector<double> energies;    // Energy bin centers (keV)
        std::vector<double> flux;        // Photon counts per bin
        std::vector<double> errors;      // Statistical errors
        double totalFlux;                // Integrated flux
        double meanEnergy;               // Flux-weighted mean energy
    };

    /**
     * @brief Transfer function for time lags
     */
    struct TransferFunction {
        std::vector<double> timeLags;    // Time delays (geometric units)
        std::vector<double> weights;     // Response weights
        double meanLag;                  // Mean time lag
        double rmsLag;                   // RMS time lag
    };

    explicit Observables(const Metric* metric);

    /**
     * @brief Compute total gravitational + Doppler redshift
     *
     * For a photon emitted from a moving source in curved spacetime:
     * 1 + z = (p_obs · u_obs) / (p_em · u_em)
     *
     * where u is the four-velocity of observer/emitter
     *
     * @param emissionPos Emission position (r, θ, φ)
     * @param observerPos Observer position
     * @param photonMomentum Photon four-momentum
     * @param sourceVelocity Source four-velocity in local frame
     * @return Redshift factor (1 + z)
     */
    double computeRedshift(const Metric::FourVector& emissionPos,
                          const Metric::FourVector& observerPos,
                          const Metric::FourVector& photonMomentum,
                          const Metric::FourVector& sourceVelocity) const;

    /**
     * @brief Compute gravitational redshift (no Doppler)
     *
     * 1 + z_grav = √(-g_tt(r_obs)) / √(-g_tt(r_em))
     *
     * @param emissionRadius Radius of emission
     * @param observerRadius Radius of observer (usually infinity)
     * @param theta Polar angle
     * @return Gravitational redshift (1 + z)
     */
    double gravitationalRedshift(double emissionRadius,
                                 double observerRadius,
                                 double theta = constants::HALF_PI) const;

    /**
     * @brief Compute relativistic Doppler factor
     *
     * D = 1 / [γ(1 - β·cosθ)]
     *
     * where γ = Lorentz factor, β = v/c, θ = angle to observer
     *
     * @param velocity Three-velocity in local frame
     * @param viewAngle Angle between velocity and line of sight
     * @return Doppler factor D
     */
    double dopplerFactor(const Eigen::Vector3d& velocity,
                        double viewAngle) const;

    /**
     * @brief Compute Keplerian orbital velocity at radius r
     *
     * v_K = √(GM/r) = √(M/r) in geometric units
     *
     * For Kerr metric, includes frame dragging corrections
     *
     * @param r Orbital radius
     * @param theta Polar angle
     * @return Orbital velocity (0 to 1, in units of c)
     */
    double keplerianVelocity(double r, double theta = constants::HALF_PI) const;

    /**
     * @brief Generate iron K-alpha line profile
     *
     * Computes the characteristic double-horned line profile from
     * a relativistic accretion disk, including:
     * - Gravitational redshift
     * - Transverse Doppler effect
     * - Orbital motion Doppler shifts
     * - Relativistic beaming
     *
     * @param line Iron line parameters
     * @param innerRadius Inner disk radius (r_ISCO or greater)
     * @param outerRadius Outer disk radius
     * @param inclination Observer inclination angle (0 = face-on, π/2 = edge-on)
     * @param energyBins Number of energy bins for spectrum
     * @return Energy spectrum showing line profile
     */
    Spectrum ironLineProfile(const IronLine& line,
                            double innerRadius,
                            double outerRadius,
                            double inclination,
                            int energyBins = 200) const;

    /**
     * @brief Compute X-ray continuum spectrum with reflection
     *
     * Models thermal Comptonization in the corona plus
     * reflection from the accretion disk
     *
     * @param innerRadius Inner disk radius
     * @param outerRadius Outer disk radius
     * @param coronaTemperature Electron temperature (keV)
     * @param diskTemperature Disk temperature at ISCO (K)
     * @param reflectionFraction Fraction of coronal emission intercepted by disk
     * @param energyBins Number of energy bins
     * @return X-ray spectrum from 0.1 to 100 keV
     */
    Spectrum xraySpectrum(double innerRadius,
                         double outerRadius,
                         double coronaTemperature,
                         double diskTemperature,
                         double reflectionFraction,
                         int energyBins = 200) const;

    /**
     * @brief Compute transfer function for reverberation mapping
     *
     * Maps light travel time delays from different disk radii to observer,
     * used to measure black hole mass and spin via X-ray reverberation
     *
     * @param sourceHeight Height of illuminating source (corona) above disk
     * @param innerRadius Inner disk radius
     * @param outerRadius Outer disk radius
     * @param inclination Observer inclination
     * @param numSamples Number of radial samples
     * @return Transfer function ψ(τ) giving lag distribution
     */
    TransferFunction transferFunction(double sourceHeight,
                                     double innerRadius,
                                     double outerRadius,
                                     double inclination,
                                     int numSamples = 100) const;

    /**
     * @brief Compute time delay between direct and reflected photon paths
     *
     * @param emissionPoint Point on disk (r, θ, φ)
     * @param sourceHeight Height of corona
     * @param observerPos Observer position
     * @return Time delay in geometric units
     */
    double lightTravelTime(const Eigen::Vector3d& emissionPoint,
                          double sourceHeight,
                          const Metric::FourVector& observerPos) const;

    /**
     * @brief Compute disk emissivity profile
     *
     * Relativistic disk illuminated by point source above axis
     * ε(r) ∝ (r/r_g)^(-q) where q is emissivity index
     *
     * Includes corrections for:
     * - General relativistic ray tracing
     * - Returning radiation (photons that orbit before hitting disk)
     *
     * @param radius Disk radius
     * @param sourceHeight Corona height
     * @param emissivityIndex Power-law index q (typically 3 for lamppost)
     * @return Emissivity at that radius
     */
    double diskEmissivity(double radius,
                         double sourceHeight,
                         double emissivityIndex = 3.0) const;

    /**
     * @brief Compute image of accretion disk on observer's sky
     *
     * Ray-traces photons backward from observer to disk,
     * recording redshift and intensity for each image pixel
     *
     * @param inclination Observer inclination angle
     * @param imageWidth Image width in pixels
     * @param imageHeight Image height in pixels
     * @param pixelSize Angular size of pixel (in M)
     * @return 2D map of (redshift, intensity) pairs
     */
    std::vector<std::vector<std::pair<double, double>>>
        diskImage(double inclination,
                 int imageWidth,
                 int imageHeight,
                 double pixelSize) const;

    /**
     * @brief Compute flux amplification from gravitational lensing
     *
     * Accounts for:
     * - Solid angle focusing
     * - Redshift suppression: (1+z)^4 factor
     * - Relativistic beaming: D^3 or D^4 depending on emission mechanism
     *
     * @param redshift Observed redshift (1 + z)
     * @param dopplerFactor Doppler factor D
     * @param spectralIndex Emission spectral index α (for D^(3+α) beaming)
     * @return Flux amplification factor
     */
    double fluxAmplification(double redshift,
                            double dopplerFactor,
                            double spectralIndex = 0.0) const;

    /**
     * @brief Compute photon ring structure and higher-order images
     *
     * For extreme lensing near black hole, photons can orbit multiple
     * times before reaching observer, creating nested photon rings
     *
     * @param inclination Observer inclination
     * @param maxOrder Maximum number of orbits to trace
     * @return Vector of (impact parameter, demagnification) for each ring order
     */
    std::vector<std::pair<double, double>>
        photonRings(double inclination, int maxOrder = 3) const;

    /**
     * @brief Check if photon trajectory intersects accretion disk
     *
     * @param position Current position
     * @param momentum Four-momentum
     * @param innerRadius Disk inner edge
     * @param outerRadius Disk outer edge
     * @param diskThickness Disk scale height
     * @return True if photon hits disk
     */
    bool intersectsDisk(const Metric::FourVector& position,
                       const Metric::FourVector& momentum,
                       double innerRadius,
                       double outerRadius,
                       double diskThickness) const;

private:
    const Metric* metric_;

    /**
     * @brief Helper: Convert between coordinate and local frames
     */
    Eigen::Vector3d coordinateToLocal(const Metric::FourVector& coordinateVel,
                                      const Metric::FourVector& position) const;

    /**
     * @brief Helper: Compute ZAMO (Zero Angular Momentum Observer) frame
     *
     * In Kerr spacetime, ZAMOs follow orbits with zero angular momentum
     * and represent "naturally" rotating observers
     */
    Metric::FourVector zamoFourVelocity(const Metric::FourVector& position) const;
};

/**
 * @brief Quasi-Periodic Oscillation (QPO) analyzer
 *
 * Models and detects QPOs in light curves, thought to originate from
 * orbital motions in the inner accretion disk
 */
class QPOAnalyzer {
public:
    struct QPOSignature {
        double frequency;           // QPO frequency (Hz)
        double amplitude;           // RMS amplitude (fraction of mean)
        double coherence;           // Quality factor Q = ν/Δν
        double centroidFrequency;   // Centroid in power spectrum
        std::string type;           // Type A, B, C, or kHz QPO
    };

    explicit QPOAnalyzer(const Metric* metric);

    /**
     * @brief Compute expected QPO frequencies for orbital motions
     *
     * Predicts frequencies based on fundamental orbital modes:
     * - ν_K: Keplerian orbital frequency
     * - ν_r: Radial epicyclic frequency
     * - ν_θ: Vertical epicyclic frequency
     * - ν_p: Periastron precession frequency
     * - ν_n: Nodal precession frequency
     *
     * @param radius Orbital radius
     * @return Vector of (mode name, frequency) pairs
     */
    std::vector<std::pair<std::string, double>>
        orbitalFrequencies(double radius) const;

    /**
     * @brief Compute epicyclic frequencies
     *
     * For geodesic orbits in Kerr spacetime:
     * ν_r² = ν_K² (1 - 6M/r ± 8a√(M/r³) - 3a²/r²)
     * ν_θ² = ν_K² (1 - 4a√(M/r³) + 3a²/r²)
     *
     * @param radius Orbital radius
     * @param nu_r Output radial epicyclic frequency
     * @param nu_theta Output vertical epicyclic frequency
     * @param nu_K Output Keplerian frequency
     */
    void epicyclicFrequencies(double radius,
                             double& nu_r,
                             double& nu_theta,
                             double& nu_K) const;

    /**
     * @brief Predict QPO from parametric resonance model
     *
     * Models QPOs as resonances between orbital modes:
     * - 3:2 resonance between ν_K and ν_r (microquasars)
     * - 2:1 resonance (some neutron stars)
     *
     * @param innerRadius Inner edge of resonance region
     * @param outerRadius Outer edge
     * @return Predicted QPO signatures
     */
    std::vector<QPOSignature> parametricResonanceModel(
        double innerRadius,
        double outerRadius) const;

private:
    const Metric* metric_;
};

} // namespace physics
} // namespace cosmic
