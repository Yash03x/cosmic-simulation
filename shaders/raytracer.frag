#version 410 core

in vec2 vTexCoord;
in vec2 vScreenPos;

out vec4 FragColor;

// Camera uniforms
uniform vec3 uCameraPos;
uniform vec3 uCameraFront;
uniform vec3 uCameraUp;
uniform vec3 uCameraRight;
uniform float uFov;
uniform float uAspect;

// Black hole uniforms
uniform float uBlackHoleMass;
uniform float uEventHorizon;
uniform float uPhotonSphere;
uniform float uISCO;

// Ray tracing parameters
uniform int uMaxSteps;
uniform float uStepSize;
uniform bool uAccretionDiskEnabled;

#define PI 3.14159265359
#define TWO_PI 6.28318530718
#define ESCAPE_RADIUS 50.0
#define EPSILON 1e-6

// ENHANCED: Much denser, more beautiful starfield
vec3 enhancedStarfield(vec3 dir) {
    vec3 d = normalize(dir);
    vec3 color = vec3(0.005, 0.005, 0.015); // Darker background for contrast

    // LAYER 1: Very bright stars (giants)
    vec2 p1 = vec2(atan(d.y, d.x), acos(d.z)) * 25.0;
    float s1 = fract(sin(dot(p1, vec2(12.9898, 78.233))) * 43758.5453);
    if (s1 > 0.9985) {
        float brightness = (s1 - 0.9985) / 0.0015;
        vec3 starColor = mix(vec3(1.0, 0.95, 0.85), vec3(0.85, 0.9, 1.0), fract(s1 * 10.0));
        color += starColor * brightness * 6.0;
    }

    // LAYER 2: Bright stars
    vec2 p2 = vec2(atan(d.y, d.x), acos(d.z)) * 60.0;
    float s2 = fract(sin(dot(p2, vec2(15.234, 82.123))) * 43758.5453);
    if (s2 > 0.997) {
        float brightness = (s2 - 0.997) / 0.003;
        vec3 starColor = mix(vec3(1.0, 1.0, 1.0), vec3(0.9, 0.95, 1.0), fract(s2 * 7.0));
        color += starColor * brightness * 3.0;
    }

    // LAYER 3: Medium stars
    vec2 p3 = vec2(atan(d.y, d.x), acos(d.z)) * 120.0;
    float s3 = fract(sin(dot(p3, vec2(18.456, 85.789))) * 43758.5453);
    if (s3 > 0.994) {
        float brightness = (s3 - 0.994) / 0.006;
        color += vec3(0.9, 0.9, 1.0) * brightness * 1.5;
    }

    // LAYER 4: Small stars (dust)
    vec2 p4 = vec2(atan(d.y, d.x), acos(d.z)) * 200.0;
    float s4 = fract(sin(dot(p4, vec2(21.789, 88.456))) * 43758.5453);
    if (s4 > 0.992) {
        float brightness = (s4 - 0.992) / 0.008;
        color += vec3(0.7, 0.8, 0.9) * brightness * 0.8;
    }

    // LAYER 5: Micro stars (depth)
    vec2 p5 = vec2(atan(d.y, d.x), acos(d.z)) * 350.0;
    float s5 = fract(sin(dot(p5, vec2(24.123, 91.789))) * 43758.5453);
    if (s5 > 0.9905) {
        color += vec3(0.5, 0.6, 0.7) * 0.3;
    }

    // Milky Way gradient (more pronounced)
    float theta = acos(d.z);
    float phi = atan(d.y, d.x);
    float gradient = abs(sin(theta * 2.0)) * abs(cos(phi * 0.5));
    color += vec3(0.04, 0.05, 0.08) * gradient * 0.6;

    // Nebula-like wisps
    float nebula = abs(sin(theta * 5.0 + phi * 3.0)) * abs(cos(theta * 3.0));
    color += vec3(0.02, 0.03, 0.05) * nebula * 0.3;

    return color;
}

// ENHANCED: Photon sphere glow effect
vec3 photonSphereGlow(float r, float closestApproach, vec3 baseColor) {
    float photonR = uPhotonSphere;

    // Distance from photon sphere
    float distToPhotonSphere = abs(closestApproach - photonR);

    // Glow intensity (peaks at photon sphere)
    float glowIntensity = 0.0;
    if (distToPhotonSphere < 2.0) {
        glowIntensity = (2.0 - distToPhotonSphere) / 2.0;
        glowIntensity = pow(glowIntensity, 2.0); // Sharper falloff
    }

    // Orange-yellow glow color
    vec3 glowColor = vec3(1.0, 0.6, 0.2);

    return baseColor + glowColor * glowIntensity * 0.3;
}

// ENHANCED: Accretion disk with full physics
vec3 accretionDisk(vec3 pos, vec3 rayDir, float r, float theta) {
    if (!uAccretionDiskEnabled) return vec3(0.0);

    float r_inner = uISCO;
    float r_outer = r_inner * 8.0;

    // Check if in disk plane (equatorial, theta ~ PI/2)
    float thetaFromEquator = abs(theta - PI/2.0);
    if (thetaFromEquator > 0.1) return vec3(0.0); // Not in disk

    if (r < r_inner || r > r_outer) return vec3(0.0);

    // Temperature profile (Shakura-Sunyaev)
    float T_max = 12000.0; // Kelvin
    float temp = T_max * pow(r_inner / r, 0.75) * pow(max(1.0 - sqrt(r_inner / r), 0.01), 0.25);

    // Blackbody color
    vec3 color;
    float t = temp / 15000.0;
    if (t < 0.5) {
        color = mix(vec3(1.0, 0.2, 0.0), vec3(1.0, 0.6, 0.0), t * 2.0); // Red to orange
    } else {
        color = mix(vec3(1.0, 0.6, 0.0), vec3(1.0, 1.0, 0.9), (t - 0.5) * 2.0); // Orange to white
    }

    // Orbital velocity (Keplerian)
    float v_orbital = sqrt(uBlackHoleMass / r);
    float beta = v_orbital; // v/c in geometric units

    // Doppler shift (phi-dependent)
    float phi = atan(pos.y, pos.x);
    float dopplerFactor = 1.0 + beta * sin(phi) * 0.7;

    // Blue shift (approaching) vs red shift (receding)
    if (dopplerFactor > 1.0) {
        // Blue shift
        color = mix(color, vec3(0.3, 0.5, 1.0), (dopplerFactor - 1.0) * 0.5);
    } else {
        // Red shift
        color = mix(color, vec3(1.0, 0.3, 0.2), (1.0 - dopplerFactor) * 0.5);
    }

    // Relativistic beaming
    float gamma = 1.0 / sqrt(1.0 - beta * beta);
    float cosTheta = sin(phi); // Simplified
    float D = 1.0 / (gamma * (1.0 - beta * cosTheta));
    float beaming = pow(abs(D), 2.5);

    // Brightness falloff
    float brightness = beaming / (r / r_inner);

    // Add turbulence
    float turbulence = fract(sin(dot(pos.xy * 20.0, vec2(12.9898, 78.233))) * 43758.5453);
    brightness *= (0.7 + turbulence * 0.6);

    // Distance-based opacity
    float alpha = clamp(brightness * 0.4, 0.0, 0.8);

    return color * brightness * alpha;
}

// ENHANCED: Ray marching with all effects
vec3 traceRay(vec3 origin, vec3 dir) {
    vec3 pos = origin;
    vec3 rayDir = normalize(dir);

    float dt = 0.15;
    float closestApproach = 1000.0;
    vec3 accumulatedDisk = vec3(0.0);
    float diskAlpha = 0.0;

    for (int i = 0; i < uMaxSteps; i++) {
        float r = length(pos);
        closestApproach = min(closestApproach, r);

        // Hit event horizon
        if (r < uEventHorizon) {
            return vec3(0.0);
        }

        // Check for accretion disk intersection
        if (uAccretionDiskEnabled) {
            float theta = acos(clamp(pos.z / r, -1.0, 1.0));
            vec3 diskColor = accretionDisk(pos, rayDir, r, theta);
            if (length(diskColor) > 0.0) {
                accumulatedDisk += diskColor * (1.0 - diskAlpha);
                diskAlpha += 0.15 * (1.0 - diskAlpha);
            }
        }

        // Escaped
        if (r > ESCAPE_RADIUS) {
            vec3 skyColor = enhancedStarfield(rayDir);
            vec3 finalColor = mix(skyColor, accumulatedDisk, diskAlpha);

            // Add photon sphere glow
            finalColor = photonSphereGlow(r, closestApproach, finalColor);

            return finalColor;
        }

        // Gravitational Deflection - Simplified Model
        // This uses an approximation rather than solving the full geodesic equation
        // for real-time performance. The actual physics requires solving:
        // d²x^μ/dλ² + Γ^μ_αβ (dx^α/dλ)(dx^β/dλ) = 0
        // where Γ^μ_αβ are Christoffel symbols of the Schwarzschild metric.
        //
        // Instead, we use a simplified inverse-cube law for the deflection angle:
        // deflection ∝ rs³/r³, where rs is the Schwarzschild radius (event horizon)
        // This approximation captures the qualitative behavior while being GPU-friendly.

        vec3 toCenter = -normalize(pos);
        float dist = length(pos);
        float rs = uEventHorizon;

        // Enhanced deflection near photon sphere (r = 3M/2 rs)
        // The photon sphere is where light can orbit the black hole.
        // We artificially enhance bending in this region for visual effect.
        float deflection = (rs * rs * rs) / (dist * dist * dist);
        if (dist < uPhotonSphere * 1.5) {
            deflection *= 1.5; // Stronger bending near photon sphere
        }
        deflection = clamp(deflection, 0.0, 0.4);

        // Apply deflection: bend ray direction toward black hole
        rayDir = normalize(rayDir + toCenter * deflection * dt);
        pos += rayDir * dt;
    }

    // Max iterations
    vec3 result = mix(vec3(0.02, 0.0, 0.0), accumulatedDisk, diskAlpha);
    return photonSphereGlow(0.0, closestApproach, result);
}

void main() {
    // Generate ray direction
    float tanHalfFov = tan(radians(uFov * 0.5));

    vec3 rayDir = normalize(
        uCameraFront +
        uCameraRight * (vScreenPos.x * uAspect * tanHalfFov) +
        uCameraUp * (vScreenPos.y * tanHalfFov)
    );

    // Trace ray with all enhancements
    vec3 color = traceRay(uCameraPos, rayDir);

    // Simple tone mapping (Reinhard)
    color = color / (color + vec3(1.0));

    // Gamma correction
    color = pow(color, vec3(1.0/2.2));

    // Subtle vignette
    float vignette = 1.0 - length(vScreenPos) * 0.15;
    color *= vignette;

    FragColor = vec4(color, 1.0);
}
