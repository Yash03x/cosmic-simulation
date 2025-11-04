#include "physics/AccretionDisk.hpp"
#include <cmath>
#include <algorithm>
#include <stdexcept>

namespace cosmic {
namespace physics {

AccretionDisk::AccretionDisk(const Metric* metric, const Configuration& config)
    : metric_(metric), config_(config) {
    if (!metric_) {
        throw std::invalid_argument("Metric pointer cannot be null");
    }
}

AccretionDisk::DiskProperties AccretionDisk::propertiesAtRadius(
    double radius) const {

    switch (config_.model) {
        case Model::ShakuraSunyaev:
            return shakuraSunyaevModel(radius);
        case Model::ADAF:
            return adafModel(radius);
        case Model::SlimDisk:
            return slimDiskModel(radius);
        default:
            return shakuraSunyaevModel(radius);
    }
}

AccretionDisk::DiskProperties AccretionDisk::shakuraSunyaevModel(
    double radius) const {

    DiskProperties props{};

    // Convert to physical units
    double r_cm = geometricToPhysical(radius);
    double M_g = config_.mass * M_SUN;

    // Keplerian angular velocity: Ω_K = √(GM/r³)
    double omega_K = std::sqrt(G_GRAV * M_g / (r_cm * r_cm * r_cm));
    props.angularVelocity = omega_K;

    // Specific angular momentum: l = r²Ω
    double l_spec = r_cm * r_cm * omega_K;

    // Energy dissipation rate per unit area (viscous heating)
    // D = (3GMṀ/8πr³) [1 - √(r_in/r)]
    double r_in = geometricToPhysical(config_.innerRadius);
    double mdot_cgs = config_.accretionRate * 1.4e18 * config_.mass; // g/s (Eddington)

    double heating = (3.0 * G_GRAV * M_g * mdot_cgs) /
                    (8.0 * constants::PI * r_cm * r_cm * r_cm);
    heating *= (1.0 - std::sqrt(r_in / r_cm));

    // Effective temperature from Stefan-Boltzmann: σT_eff⁴ = D
    props.effectiveTemperature = std::pow(heating / SIGMA_SB, 0.25);

    // Sound speed: c_s = √(kT/μm_p) where μ ≈ 0.6 for ionized gas
    constexpr double MU_GAS = 0.6;

    // For thin disk, use vertical energy balance to get midplane temperature
    // T_c ≈ (3τ/8) T_eff where τ is optical depth
    // First iteration: assume τ ~ 1
    props.temperature = 1.5 * props.effectiveTemperature;

    props.soundSpeed = std::sqrt(K_BOLTZMANN * props.temperature /
                                 (MU_GAS * M_PROTON));

    // Scale height: H = c_s / Ω_K
    double H = props.soundSpeed / omega_K;
    props.scaleHeight = H / r_cm;

    // Surface density from α-viscosity and mass conservation
    // Σ = Ṁ / (3πν) where ν = α c_s H
    double nu_visc = config_.viscosityAlpha * props.soundSpeed * H;
    props.surfaceDensity = mdot_cgs / (3.0 * constants::PI * nu_visc);

    // Midplane density: ρ_0 ≈ Σ / (√(2π) H)
    props.density = props.surfaceDensity / (std::sqrt(constants::TWO_PI) * H);

    // Opacity (Rosseland mean)
    props.opacity = rosselandMeanOpacity(props.temperature, props.density);

    // Optical depth: τ = κΣ/2
    props.opticalDepth = props.opacity * props.surfaceDensity / 2.0;

    // Refine midplane temperature using optical depth
    // T_c⁴ = (3τ/8 + 1/2 + 1/4τ) T_eff⁴
    double tau = props.opticalDepth;
    double temp_factor = 3.0 * tau / 8.0 + 0.5 + 0.25 / (tau + 0.1);
    props.temperature = std::pow(temp_factor, 0.25) * props.effectiveTemperature;

    // Gas pressure: P_gas = ρkT/μm_p
    double P_gas = props.density * K_BOLTZMANN * props.temperature /
                   (MU_GAS * M_PROTON);

    // Radiation pressure: P_rad = aT⁴/3
    double P_rad = A_RAD * std::pow(props.temperature, 4.0) / 3.0;

    props.pressure = P_gas + P_rad;

    // Radial velocity (accretion): v_r ≈ -ν/r
    props.accretionVelocity = -nu_visc / r_cm;

    // Velocity field (Keplerian + radial inflow)
    props.velocity = Eigen::Vector3d(props.accretionVelocity,
                                    0.0,
                                    r_cm * omega_K);

    // Magnetic field (if enabled)
    if (config_.includeMagneticFields) {
        // Equipartition: B² ≈ 8παP
        double P_mag = config_.viscosityAlpha * props.pressure;
        double B_mag = std::sqrt(8.0 * constants::PI * P_mag);

        // Predominantly toroidal
        props.magneticField = Eigen::Vector3d(0.1 * B_mag, 0.0, B_mag);
    }

    props.viscosity = nu_visc;

    return props;
}

AccretionDisk::DiskProperties AccretionDisk::adafModel(double radius) const {
    DiskProperties props{};

    // ADAF: advection-dominated, geometrically thick
    // Key feature: most of the viscously generated heat is advected
    // rather than radiated

    double r_cm = geometricToPhysical(radius);
    double M_g = config_.mass * M_SUN;

    double omega_K = std::sqrt(G_GRAV * M_g / (r_cm * r_cm * r_cm));
    props.angularVelocity = omega_K;

    // ADAF has much higher temperatures and lower densities
    // Virial temperature: kT ~ GMm_p/r
    props.temperature = G_GRAV * M_g * M_PROTON / (K_BOLTZMANN * r_cm);

    // Scale height: H/r ~ 1 (geometrically thick)
    props.scaleHeight = 0.3;

    // Much lower surface density
    double mdot_cgs = config_.accretionRate * 1.4e18 * config_.mass;
    double H = props.scaleHeight * r_cm;
    props.soundSpeed = omega_K * H;

    double nu_visc = config_.viscosityAlpha * props.soundSpeed * H;
    props.surfaceDensity = mdot_cgs / (3.0 * constants::PI * nu_visc);
    props.density = props.surfaceDensity / H;

    // Very optically thin
    props.opacity = 0.4; // Thomson scattering only
    props.opticalDepth = props.opacity * props.surfaceDensity / 2.0;

    // Lower effective temperature (most energy advected)
    double advection_factor = 0.1; // ~10% radiated
    props.effectiveTemperature = std::pow(
        advection_factor * SIGMA_SB * std::pow(props.temperature, 4.0) / SIGMA_SB,
        0.25);

    double P_gas = props.density * K_BOLTZMANN * props.temperature / M_PROTON;
    props.pressure = P_gas;

    props.accretionVelocity = -nu_visc / r_cm;
    props.velocity = Eigen::Vector3d(props.accretionVelocity, 0.0, r_cm * omega_K);
    props.viscosity = nu_visc;

    return props;
}

AccretionDisk::DiskProperties AccretionDisk::slimDiskModel(double radius) const {
    // Slim disk: intermediate between thin disk and ADAF
    // Important for super-Eddington accretion

    DiskProperties props = shakuraSunyaevModel(radius);

    // Modifications for advection
    if (config_.accretionRate > 1.0) {
        // Enhance scale height
        props.scaleHeight *= std::sqrt(config_.accretionRate);

        // Reduce effective temperature (advection cooling)
        props.effectiveTemperature /= std::pow(config_.accretionRate, 0.25);
    }

    return props;
}

double AccretionDisk::opacityAtFrequency(double temperature,
                                        double density,
                                        double frequency) const {
    // Thomson scattering (frequency-independent)
    double kappa_es = 0.4; // cm²/g

    // Free-free absorption: κ_ff ∝ ρT^(-7/2)ν^(-3)
    double kappa_ff = 6.4e22 * density * std::pow(temperature, -3.5) *
                     std::pow(frequency, -3.0);

    // Bound-free (photoionization) - simplified
    double nu_edge = 3.29e15; // Hz (Lyman edge)
    double kappa_bf = 0.0;
    if (frequency > nu_edge) {
        kappa_bf = 4.3e25 * density / temperature *
                  std::pow(frequency / nu_edge, -3.0);
    }

    // Molecular opacity (cool regions)
    double kappa_mol = 0.0;
    if (temperature < 4000.0) {
        kappa_mol = 0.01 * std::exp(-(temperature - 1000.0) / 1000.0);
    }

    // Total
    double kappa_total = kappa_es + kappa_ff + kappa_bf + kappa_mol;

    return std::max(kappa_total, kappa_es); // At least Thomson
}

double AccretionDisk::rosselandMeanOpacity(double temperature,
                                          double density) const {
    OpacityComponents comp = opacityBreakdown(temperature, density);
    return comp.total;
}

AccretionDisk::OpacityComponents AccretionDisk::opacityBreakdown(
    double temperature,
    double density) const {

    OpacityComponents comp{};

    // Thomson scattering
    comp.electronScattering = 0.4; // cm²/g

    // Kramers opacity (free-free + bound-free)
    comp.freeFreeBound = kramersOpacity(temperature, density);

    // H⁻ opacity (important 3000-8000 K)
    if (temperature >= 2000.0 && temperature <= 10000.0) {
        comp.hMinusIon = hMinusOpacity(temperature, density);
    }

    // Molecular opacity (cool)
    if (temperature < 4000.0) {
        comp.molecules = 0.01 * std::exp(-(temperature - 1000.0) / 1000.0);
    }

    // Dust opacity (very cool)
    if (temperature < 1500.0) {
        comp.dust = 2.0 * std::exp(-(temperature - 500.0) / 500.0);
    }

    // Combine using Rosseland averaging (approximate as additive)
    comp.total = comp.electronScattering + comp.freeFreeBound +
                 comp.hMinusIon + comp.molecules + comp.dust;

    return comp;
}

std::vector<std::pair<double, double>> AccretionDisk::computeSED(
    int numFrequencies,
    double minFrequency,
    double maxFrequency,
    double inclination) const {

    std::vector<std::pair<double, double>> sed;
    sed.reserve(numFrequencies);

    // Log-spaced frequencies
    double log_min = std::log10(minFrequency);
    double log_max = std::log10(maxFrequency);
    double dlog_nu = (log_max - log_min) / numFrequencies;

    for (int i = 0; i < numFrequencies; ++i) {
        double log_nu = log_min + (i + 0.5) * dlog_nu;
        double nu = std::pow(10.0, log_nu);

        // Integrate flux over disk annuli
        double total_flux = 0.0;
        int num_radial = 100;
        double dr = (config_.outerRadius - config_.innerRadius) / num_radial;

        for (int ir = 0; ir < num_radial; ++ir) {
            double r = config_.innerRadius + (ir + 0.5) * dr;
            DiskProperties props = propertiesAtRadius(r);

            // Planck function at this temperature
            double T = props.effectiveTemperature;
            double x = constants::PLANCK * nu / (K_BOLTZMANN * T);
            double B_nu = 2.0 * constants::PLANCK * nu * nu * nu /
                         (C_LIGHT * C_LIGHT * (std::exp(x) - 1.0));

            // Emission from annulus
            double r_cm = geometricToPhysical(r);
            double area = constants::TWO_PI * r_cm * dr * geometricToPhysical(1.0);

            // Viewing angle correction
            double cos_i = std::cos(inclination);

            total_flux += 2.0 * B_nu * area * cos_i; // Factor of 2 for both sides
        }

        sed.push_back({nu, total_flux});
    }

    return sed;
}

double AccretionDisk::totalLuminosity() const {
    double eta = radiativeEfficiency();
    double mdot_cgs = config_.accretionRate * 1.4e18 * config_.mass; // g/s
    return eta * mdot_cgs * C_LIGHT * C_LIGHT; // erg/s
}

double AccretionDisk::radiativeEfficiency() const {
    // η = 1 - E_ISCO
    // For Schwarzschild: η ≈ 0.057
    // For maximal Kerr: η ≈ 0.42

    double r_isco = metric_->iscoRadius();

    // Specific energy at ISCO (Schwarzschild approximation)
    double E_isco = std::sqrt(1.0 - 2.0 / (3.0 * r_isco));

    return 1.0 - E_isco;
}

double AccretionDisk::eddingtonRatio() const {
    // L_Edd = 4πGMm_p c / σ_T
    double M_g = config_.mass * M_SUN;
    double L_Edd = 4.0 * constants::PI * G_GRAV * M_g * M_PROTON * C_LIGHT /
                   SIGMA_THOMSON;

    double L = totalLuminosity();
    return L / L_Edd;
}

bool AccretionDisk::isRadiationDominated(double radius) const {
    DiskProperties props = propertiesAtRadius(radius);

    double P_gas = props.density * K_BOLTZMANN * props.temperature /
                   (0.6 * M_PROTON);
    double P_rad = A_RAD * std::pow(props.temperature, 4.0) / 3.0;

    return P_rad > P_gas;
}

double AccretionDisk::verticalDensity(double radius, double height) const {
    DiskProperties props = propertiesAtRadius(radius);

    double r_cm = geometricToPhysical(radius);
    double H = props.scaleHeight * r_cm;

    // Gaussian profile
    return props.density * std::exp(-height * height / (2.0 * H * H));
}

double AccretionDisk::verticalTemperature(double radius, double height) const {
    DiskProperties props = propertiesAtRadius(radius);

    double r_cm = geometricToPhysical(radius);
    double H = props.scaleHeight * r_cm;

    // Temperature decreases with height
    double z_ratio = std::abs(height) / H;

    // Interpolate between midplane and effective temperature
    double T_mid = props.temperature;
    double T_eff = props.effectiveTemperature;

    return T_mid - (T_mid - T_eff) * std::tanh(z_ratio);
}

Eigen::Vector3d AccretionDisk::magneticField(double radius, double height) const {
    if (!config_.includeMagneticFields) {
        return Eigen::Vector3d::Zero();
    }

    DiskProperties props = propertiesAtRadius(radius);
    return props.magneticField * std::exp(-height * height /
                                         (2.0 * props.scaleHeight * props.scaleHeight));
}

double AccretionDisk::stressTensor(double radius) const {
    DiskProperties props = propertiesAtRadius(radius);

    double P_total = props.pressure;
    if (config_.includeMagneticFields) {
        double B2 = props.magneticField.squaredNorm();
        P_total += B2 / (8.0 * constants::PI);
    }

    return config_.viscosityAlpha * P_total;
}

std::vector<std::array<double, 4>> AccretionDisk::verticalStructure(
    double radius,
    int numVerticalPoints) const {

    std::vector<std::array<double, 4>> structure;
    structure.reserve(numVerticalPoints);

    DiskProperties props = propertiesAtRadius(radius);
    double r_cm = geometricToPhysical(radius);
    double H = props.scaleHeight * r_cm;

    for (int i = 0; i < numVerticalPoints; ++i) {
        double z = -3.0 * H + (6.0 * H * i) / numVerticalPoints;

        double T = verticalTemperature(radius, z);
        double rho = verticalDensity(radius, z);
        double P = rho * K_BOLTZMANN * T / (0.6 * M_PROTON);

        structure.push_back({z, T, rho, P});
    }

    return structure;
}

double AccretionDisk::coolingTimescale(double radius) const {
    DiskProperties props = propertiesAtRadius(radius);

    double r_cm = geometricToPhysical(radius);
    double H = props.scaleHeight * r_cm;

    // Thermal energy: E_th ~ ρ H c_s²
    double E_thermal = props.density * H * props.soundSpeed * props.soundSpeed;

    // Cooling luminosity: L ~ σT_eff⁴
    double L_cool = SIGMA_SB * std::pow(props.effectiveTemperature, 4.0);

    return E_thermal / L_cool;
}

double AccretionDisk::viscousTimescale(double radius) const {
    DiskProperties props = propertiesAtRadius(radius);
    double r_cm = geometricToPhysical(radius);

    // t_visc = r² / ν
    return r_cm * r_cm / props.viscosity;
}

bool AccretionDisk::isThermallyStable(double radius) const {
    // S-curve stability: check if dT/dΣ > 0

    double dr = 0.01 * radius;
    DiskProperties props = propertiesAtRadius(radius);
    DiskProperties props_plus = propertiesAtRadius(radius + dr);

    double dT = props_plus.temperature - props.temperature;
    double dSigma = props_plus.surfaceDensity - props.surfaceDensity;

    return (dT * dSigma > 0.0);
}

void AccretionDisk::setConfiguration(const Configuration& config) {
    config_ = config;
}

double AccretionDisk::geometricToPhysical(double r_geom) const {
    double M_g = config_.mass * M_SUN;
    double r_g = G_GRAV * M_g / (C_LIGHT * C_LIGHT); // Gravitational radius
    return r_geom * r_g; // Convert to cm
}

double AccretionDisk::kramersOpacity(double temperature, double density) const {
    // Kramers law: κ = κ_0 ρT^(-7/2)
    constexpr double kappa_0 = 5.0e24; // cgs units

    return kappa_0 * density * std::pow(temperature, -3.5);
}

double AccretionDisk::hMinusOpacity(double temperature, double density) const {
    // H⁻ opacity (important in stellar atmospheres and cool disk regions)
    // Simplified approximation

    if (temperature < 2000.0 || temperature > 10000.0) {
        return 0.0;
    }

    // Peak around 5000-8000 K
    double T_peak = 6000.0;
    double width = 2000.0;

    double gaussian = std::exp(-math::square(temperature - T_peak) /
                              (2.0 * width * width));

    return 2.5e-31 * density * std::pow(temperature, 9.0) * gaussian;
}

} // namespace physics
} // namespace cosmic
