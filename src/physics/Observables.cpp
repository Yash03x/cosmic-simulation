#include "physics/Observables.hpp"
#include "physics/Kerr.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>

namespace cosmic {
namespace physics {

Observables::Observables(const Metric* metric) : metric_(metric) {
    if (!metric_) {
        throw std::invalid_argument("Metric pointer cannot be null");
    }
}

double Observables::computeRedshift(const Metric::FourVector& emissionPos,
                                   const Metric::FourVector& observerPos,
                                   const Metric::FourVector& photonMomentum,
                                   const Metric::FourVector& sourceVelocity) const {
    // Compute metric at emission and observer locations
    Metric::MetricTensor g_em = metric_->metricTensor(emissionPos);
    Metric::MetricTensor g_obs = metric_->metricTensor(observerPos);

    // Observer four-velocity (assumed at rest at infinity)
    Metric::FourVector u_obs = Metric::FourVector::Zero();
    u_obs[0] = 1.0 / std::sqrt(-g_obs(0, 0)); // Normalized timelike

    // Emitter four-velocity (sourceVelocity is already a four-vector)
    Metric::FourVector u_em = sourceVelocity;

    // Energy at observer: E_obs = -p · u_obs
    double E_obs = 0.0;
    for (int mu = 0; mu < 4; ++mu) {
        for (int nu = 0; nu < 4; ++nu) {
            E_obs -= g_obs(mu, nu) * photonMomentum[mu] * u_obs[nu];
        }
    }

    // Energy at emission: E_em = -p · u_em
    double E_em = 0.0;
    for (int mu = 0; mu < 4; ++mu) {
        for (int nu = 0; nu < 4; ++nu) {
            E_em -= g_em(mu, nu) * photonMomentum[mu] * u_em[nu];
        }
    }

    // Redshift: 1 + z = E_em / E_obs
    if (std::abs(E_obs) < constants::EPSILON) {
        return 1.0;
    }

    return E_em / E_obs;
}

double Observables::gravitationalRedshift(double emissionRadius,
                                         double observerRadius,
                                         double theta) const {
    Metric::FourVector emissionPos(0.0, emissionRadius, theta, 0.0);
    Metric::MetricTensor g_em = metric_->metricTensor(emissionPos);

    double g_tt_em = g_em(0, 0);

    if (observerRadius > 1e6) {
        // Observer at infinity: g_tt → -1
        return 1.0 / std::sqrt(-g_tt_em);
    } else {
        Metric::FourVector observerPos(0.0, observerRadius, theta, 0.0);
        Metric::MetricTensor g_obs = metric_->metricTensor(observerPos);
        double g_tt_obs = g_obs(0, 0);

        return std::sqrt(-g_tt_obs / g_tt_em);
    }
}

double Observables::dopplerFactor(const Eigen::Vector3d& velocity,
                                 double viewAngle) const {
    double v_mag = velocity.norm();
    if (v_mag < constants::EPSILON) {
        return 1.0;
    }

    double beta = std::min(v_mag, 0.999); // Cap at near lightspeed
    double gamma = 1.0 / std::sqrt(1.0 - beta * beta);

    double cos_theta = std::cos(viewAngle);
    double denominator = gamma * (1.0 - beta * cos_theta);

    if (std::abs(denominator) < constants::EPSILON) {
        return 1e6; // Very large beaming
    }

    return 1.0 / denominator;
}

double Observables::keplerianVelocity(double r, double theta) const {
    // For Schwarzschild: v_K = sqrt(M/r)
    double M = metric_->getMass();

    // Check if this is a Kerr metric for frame dragging correction
    const Kerr* kerr = dynamic_cast<const Kerr*>(metric_);
    if (kerr) {
        // Frame dragging angular velocity
        double omega_frame = kerr->frameDraggingOmega(r, theta);

        // Keplerian angular velocity (specific angular momentum)
        double omega_K = std::sqrt(M / (r * r * r));

        // Effective velocity including frame dragging
        double v_phi = r * (omega_K + omega_frame);
        return std::min(v_phi, 0.999);
    }

    // Schwarzschild case
    return std::sqrt(M / r);
}

Observables::Spectrum Observables::ironLineProfile(
    const IronLine& line,
    double innerRadius,
    double outerRadius,
    double inclination,
    int energyBins) const {

    Spectrum spectrum;
    spectrum.energies.resize(energyBins);
    spectrum.flux.resize(energyBins, 0.0);
    spectrum.errors.resize(energyBins, 0.0);

    // Energy range: typically 3-9 keV for Fe K-alpha
    double E_rest = line.restEnergy; // 6.4 keV
    double E_min = 0.5 * E_rest;
    double E_max = 1.5 * E_rest;
    double dE = (E_max - E_min) / energyBins;

    for (int i = 0; i < energyBins; ++i) {
        spectrum.energies[i] = E_min + (i + 0.5) * dE;
    }

    // Integrate over disk annuli
    int numRadialBins = 100;
    double dr = (outerRadius - innerRadius) / numRadialBins;

    int numAzimuthalBins = 36; // Every 10 degrees
    double dphi = constants::TWO_PI / numAzimuthalBins;

    double cos_i = std::cos(inclination);
    double sin_i = std::sin(inclination);

    for (int ir = 0; ir < numRadialBins; ++ir) {
        double r = innerRadius + (ir + 0.5) * dr;

        // Disk emissivity profile (power law)
        double emissivity = std::pow(r / innerRadius, -line.emissivity);

        // Keplerian velocity at this radius
        double v_K = keplerianVelocity(r);
        double gamma = 1.0 / std::sqrt(1.0 - v_K * v_K);

        for (int iphi = 0; iphi < numAzimuthalBins; ++iphi) {
            double phi = iphi * dphi;

            // Velocity components in observer frame
            double v_x = -v_K * std::sin(phi);
            double v_y = v_K * std::cos(phi);

            // Line-of-sight velocity
            double v_los = v_y * sin_i;

            // Combined redshift
            double z_grav = gravitationalRedshift(r, 1e10) - 1.0; // Gravitational
            double z_doppler = -v_los; // First-order Doppler
            double z_transverse = (gamma - 1.0); // Transverse Doppler

            double z_total = z_grav + z_doppler + z_transverse;
            double E_observed = E_rest / (1.0 + z_total);

            // Relativistic beaming factor
            double view_angle = std::acos(v_los / (v_K + constants::EPSILON));
            double D = dopplerFactor(Eigen::Vector3d(0, v_K, 0), view_angle);

            // Flux enhancement
            double flux_factor = D * D * D * emissivity;

            // Add to appropriate energy bin
            int bin = static_cast<int>((E_observed - E_min) / dE);
            if (bin >= 0 && bin < energyBins) {
                spectrum.flux[bin] += flux_factor * r * dr * dphi; // Jacobian r dr dφ
            }
        }
    }

    // Normalize and compute statistics
    spectrum.totalFlux = std::accumulate(spectrum.flux.begin(),
                                        spectrum.flux.end(), 0.0);

    if (spectrum.totalFlux > constants::EPSILON) {
        spectrum.meanEnergy = 0.0;
        for (int i = 0; i < energyBins; ++i) {
            spectrum.meanEnergy += spectrum.energies[i] * spectrum.flux[i];
            spectrum.errors[i] = std::sqrt(std::max(spectrum.flux[i], 1.0));
        }
        spectrum.meanEnergy /= spectrum.totalFlux;
    }

    return spectrum;
}

Observables::Spectrum Observables::xraySpectrum(
    double innerRadius,
    double outerRadius,
    double coronaTemperature,
    double diskTemperature,
    double reflectionFraction,
    int energyBins) const {

    Spectrum spectrum;
    spectrum.energies.resize(energyBins);
    spectrum.flux.resize(energyBins, 0.0);

    // Energy range: 0.1 to 100 keV
    double E_min = 0.1;
    double E_max = 100.0;

    // Log-spaced energy bins
    double log_E_min = std::log10(E_min);
    double log_E_max = std::log10(E_max);
    double d_log_E = (log_E_max - log_E_min) / energyBins;

    for (int i = 0; i < energyBins; ++i) {
        spectrum.energies[i] = std::pow(10.0, log_E_min + (i + 0.5) * d_log_E);
    }

    // Comptonization: thermal Comptonization model
    // F(E) ∝ E^(-α) exp(-E/kT_e) for photon index α
    double photon_index = 2.0; // Typical for AGN
    double kT_e = coronaTemperature; // Already in keV

    // Disk reflection: simplified model
    // Includes Compton hump around 20-30 keV

    for (int i = 0; i < energyBins; ++i) {
        double E = spectrum.energies[i];

        // Primary continuum (power law with exponential cutoff)
        double continuum = std::pow(E, -photon_index) *
                          std::exp(-E / kT_e);

        // Reflection component
        double reflection = 0.0;
        if (reflectionFraction > constants::EPSILON) {
            // Compton hump (simplified Gaussian)
            double E_hump = 25.0; // keV
            double width_hump = 10.0;
            double hump = std::exp(-math::square(E - E_hump) /
                                  (2.0 * width_hump * width_hump));

            // Fe K-alpha line (simplified)
            double E_Fe = 6.4;
            double width_Fe = 0.1;
            double line = std::exp(-math::square(E - E_Fe) /
                                  (2.0 * width_Fe * width_Fe));

            reflection = reflectionFraction * (0.5 * hump + 2.0 * line);
        }

        spectrum.flux[i] = continuum + reflection;
    }

    // Normalize
    spectrum.totalFlux = 0.0;
    for (int i = 0; i < energyBins; ++i) {
        spectrum.totalFlux += spectrum.flux[i];
    }

    if (spectrum.totalFlux > constants::EPSILON) {
        double norm = 1.0 / spectrum.totalFlux;
        for (int i = 0; i < energyBins; ++i) {
            spectrum.flux[i] *= norm;
        }
        spectrum.totalFlux = 1.0;
    }

    return spectrum;
}

Observables::TransferFunction Observables::transferFunction(
    double sourceHeight,
    double innerRadius,
    double outerRadius,
    double inclination,
    int numSamples) const {

    TransferFunction tf;
    tf.timeLags.reserve(numSamples);
    tf.weights.reserve(numSamples);

    double M = metric_->getMass();
    double cos_i = std::cos(inclination);

    // Sample disk radially
    double dr = (outerRadius - innerRadius) / numSamples;

    for (int i = 0; i < numSamples; ++i) {
        double r = innerRadius + (i + 0.5) * dr;

        // Lamppost geometry: source at height h on symmetry axis
        // Distance from source to disk element
        double d_source = std::sqrt(r * r + sourceHeight * sourceHeight);

        // Distance from disk element to observer (at infinity, inclination i)
        // Approximate as r * sin(i) for path length difference
        double d_observer = r * cos_i;

        // Coordinate time difference (geometric units)
        double t_lag = d_source + d_observer;

        // Weight by emissivity and solid angle
        double emissivity = diskEmissivity(r, sourceHeight);
        double solid_angle = r * dr / (d_source * d_source);
        double weight = emissivity * solid_angle;

        tf.timeLags.push_back(t_lag);
        tf.weights.push_back(weight);
    }

    // Compute statistics
    double total_weight = std::accumulate(tf.weights.begin(),
                                         tf.weights.end(), 0.0);

    if (total_weight > constants::EPSILON) {
        tf.meanLag = 0.0;
        for (size_t i = 0; i < tf.timeLags.size(); ++i) {
            tf.meanLag += tf.timeLags[i] * tf.weights[i];
        }
        tf.meanLag /= total_weight;

        tf.rmsLag = 0.0;
        for (size_t i = 0; i < tf.timeLags.size(); ++i) {
            double diff = tf.timeLags[i] - tf.meanLag;
            tf.rmsLag += diff * diff * tf.weights[i];
        }
        tf.rmsLag = std::sqrt(tf.rmsLag / total_weight);
    }

    return tf;
}

double Observables::lightTravelTime(const Eigen::Vector3d& emissionPoint,
                                   double sourceHeight,
                                   const Metric::FourVector& observerPos) const {
    double r = emissionPoint[0];
    double theta = emissionPoint[1];
    double phi = emissionPoint[2];

    // Simplified: coordinate time for lamppost geometry
    // Full calculation would require geodesic integration

    // Source to disk
    double r_source = 0.0; // On axis
    double z_source = sourceHeight;
    double r_disk = r * std::sin(theta);
    double z_disk = r * std::cos(theta);

    double d_source = std::sqrt(math::square(r_disk - r_source) +
                               math::square(z_disk - z_source));

    // Disk to observer (approximate)
    double r_obs = observerPos[1];
    double d_obs = r_obs - r;

    return d_source + d_obs;
}

double Observables::diskEmissivity(double radius,
                                  double sourceHeight,
                                  double emissivityIndex) const {
    // Power-law emissivity for lamppost illumination
    // ε(r) ∝ r^(-q)
    //
    // For a point source at height h on axis:
    // ε(r) ∝ h / (r² + h²)^(3/2)  (isotropic emission)

    if (sourceHeight < constants::EPSILON) {
        // Default power law
        return std::pow(radius, -emissivityIndex);
    }

    double r2 = radius * radius;
    double h2 = sourceHeight * sourceHeight;
    double d2 = r2 + h2;

    // Solid angle factor
    return sourceHeight / (d2 * std::sqrt(d2));
}

std::vector<std::vector<std::pair<double, double>>>
    Observables::diskImage(double inclination,
                          int imageWidth,
                          int imageHeight,
                          double pixelSize) const {

    std::vector<std::vector<std::pair<double, double>>> image;
    image.resize(imageHeight);
    for (int i = 0; i < imageHeight; ++i) {
        image[i].resize(imageWidth, {1.0, 0.0}); // {redshift, intensity}
    }

    // Center of image
    double center_x = imageWidth / 2.0;
    double center_y = imageHeight / 2.0;

    // This would require full backward ray tracing
    // Placeholder implementation returns simple structure

    return image;
}

double Observables::fluxAmplification(double redshift,
                                     double dopplerFactor,
                                     double spectralIndex) const {
    // Redshift suppression: (1+z)^(-4) for conserved photon number
    double z_factor = std::pow(redshift, -4.0);

    // Relativistic beaming: D^(3+α) for spectral index α
    double beaming_factor = std::pow(dopplerFactor, 3.0 + spectralIndex);

    return z_factor * beaming_factor;
}

std::vector<std::pair<double, double>>
    Observables::photonRings(double inclination, int maxOrder) const {

    std::vector<std::pair<double, double>> rings;

    double r_photon = metric_->photonSphereRadius();

    // Each subsequent ring is increasingly demagnified
    for (int n = 0; n < maxOrder; ++n) {
        // Impact parameter decreases slightly for each order
        double b = r_photon * (1.0 - 0.01 * n);

        // Demagnification factor increases exponentially
        double demag = std::pow(10.0, -n);

        rings.push_back({b, demag});
    }

    return rings;
}

bool Observables::intersectsDisk(const Metric::FourVector& position,
                                const Metric::FourVector& momentum,
                                double innerRadius,
                                double outerRadius,
                                double diskThickness) const {
    double r = position[1];
    double theta = position[2];

    // Check if near equatorial plane
    double z = r * std::cos(theta);
    if (std::abs(z) > diskThickness) {
        return false;
    }

    // Check if in radial range
    double r_cyl = r * std::sin(theta);
    if (r_cyl < innerRadius || r_cyl > outerRadius) {
        return false;
    }

    return true;
}

Eigen::Vector3d Observables::coordinateToLocal(
    const Metric::FourVector& coordinateVel,
    const Metric::FourVector& position) const {

    // Transform coordinate velocity to local tetrad
    // This requires ZAMO frame for Kerr metric

    return Eigen::Vector3d(coordinateVel[1], coordinateVel[2], coordinateVel[3]);
}

Metric::FourVector Observables::zamoFourVelocity(
    const Metric::FourVector& position) const {

    Metric::MetricTensor g = metric_->metricTensor(position);

    // ZAMO four-velocity: u^t = sqrt(-g^tt), u^r = u^θ = 0, u^φ = -g^tφ/g^φφ u^t
    Metric::FourVector u = Metric::FourVector::Zero();

    double g_tt = g(0, 0);
    double g_tphi = g(0, 3);
    double g_phiphi = g(3, 3);

    if (g_tt < 0.0) {
        u[0] = 1.0 / std::sqrt(-g_tt);
        if (std::abs(g_phiphi) > constants::EPSILON) {
            u[3] = -g_tphi * u[0] / g_phiphi;
        }
    }

    return u;
}

// QPOAnalyzer implementation

QPOAnalyzer::QPOAnalyzer(const Metric* metric) : metric_(metric) {
    if (!metric_) {
        throw std::invalid_argument("Metric pointer cannot be null");
    }
}

std::vector<std::pair<std::string, double>>
    QPOAnalyzer::orbitalFrequencies(double radius) const {

    std::vector<std::pair<std::string, double>> frequencies;

    double nu_r, nu_theta, nu_K;
    epicyclicFrequencies(radius, nu_r, nu_theta, nu_K);

    frequencies.push_back({"Keplerian", nu_K});
    frequencies.push_back({"Radial Epicyclic", nu_r});
    frequencies.push_back({"Vertical Epicyclic", nu_theta});
    frequencies.push_back({"Periastron Precession", nu_K - nu_r});
    frequencies.push_back({"Nodal Precession", nu_K - nu_theta});

    return frequencies;
}

void QPOAnalyzer::epicyclicFrequencies(double radius,
                                      double& nu_r,
                                      double& nu_theta,
                                      double& nu_K) const {
    double M = metric_->getMass();

    // Check if Kerr metric
    const Kerr* kerr = dynamic_cast<const Kerr*>(metric_);

    if (kerr) {
        // Kerr metric epicyclic frequencies
        double a = kerr->getSpin() * M;
        double r2 = radius * radius;
        double a2 = a * a;

        // Keplerian frequency
        double omega_K = std::sqrt(M / (r2 * radius));
        nu_K = omega_K / constants::TWO_PI;

        // Radial epicyclic frequency
        double term1 = 1.0 - 6.0 * M / radius;
        double term2 = 8.0 * a * std::sqrt(M / (radius * radius * radius));
        double term3 = 3.0 * a2 / r2;

        double omega_r_sq = omega_K * omega_K * (term1 + term2 - term3);
        nu_r = std::sqrt(std::max(omega_r_sq, 0.0)) / constants::TWO_PI;

        // Vertical epicyclic frequency
        double omega_theta_sq = omega_K * omega_K * (1.0 - 4.0 * term2 + term3);
        nu_theta = std::sqrt(std::max(omega_theta_sq, 0.0)) / constants::TWO_PI;

    } else {
        // Schwarzschild metric
        double omega_K = std::sqrt(M / (radius * radius * radius));
        nu_K = omega_K / constants::TWO_PI;

        double omega_r_sq = omega_K * omega_K * (1.0 - 6.0 * M / radius);
        nu_r = std::sqrt(std::max(omega_r_sq, 0.0)) / constants::TWO_PI;

        nu_theta = nu_K; // Equal for Schwarzschild
    }
}

std::vector<QPOAnalyzer::QPOSignature>
    QPOAnalyzer::parametricResonanceModel(double innerRadius,
                                         double outerRadius) const {

    std::vector<QPOSignature> qpos;

    // Search for 3:2 resonance (nu_K : nu_r)
    int numSamples = 100;
    double dr = (outerRadius - innerRadius) / numSamples;

    for (int i = 0; i < numSamples; ++i) {
        double r = innerRadius + (i + 0.5) * dr;

        double nu_r, nu_theta, nu_K;
        epicyclicFrequencies(r, nu_r, nu_theta, nu_K);

        // Check for 3:2 resonance
        double ratio = nu_K / (nu_r + constants::EPSILON);
        if (std::abs(ratio - 1.5) < 0.05) {
            QPOSignature qpo;
            qpo.frequency = nu_K * 1e3; // Convert to Hz (rough estimate)
            qpo.amplitude = 0.1; // 10% RMS
            qpo.coherence = 10.0; // Quality factor
            qpo.centroidFrequency = qpo.frequency;
            qpo.type = "Type C (3:2 resonance)";
            qpos.push_back(qpo);
        }
    }

    return qpos;
}

} // namespace physics
} // namespace cosmic
