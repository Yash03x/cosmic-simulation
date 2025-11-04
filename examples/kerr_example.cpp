/**
 * Simple Example: Computing Black Hole Properties
 *
 * This program shows how to use the Kerr metric to compute
 * properties of a spinning black hole.
 *
 * No physics background needed - just run and see the results!
 */

#include <iostream>
#include <iomanip>
#include "physics/Kerr.hpp"
#include "physics/Schwarzschild.hpp"

int main() {
    std::cout << "=== Black Hole Property Calculator ===\n\n";

    // Create a non-rotating black hole (like Schwarzschild)
    std::cout << "1. Non-Rotating Black Hole (spin = 0)\n";
    std::cout << "   Mass: 1 solar mass\n";
    cosmic::physics::Schwarzschild nonRotating(1.0);

    double r_horizon = nonRotating.eventHorizonRadius();
    double r_photon = nonRotating.photonSphereRadius();
    double r_isco = nonRotating.iscoRadius();

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "   - Event Horizon:  " << r_horizon << "M (point of no return)\n";
    std::cout << "   - Photon Sphere:  " << r_photon << "M (where light can orbit)\n";
    std::cout << "   - ISCO:           " << r_isco << "M (closest stable orbit)\n\n";

    // Create a slowly spinning black hole
    std::cout << "2. Slowly Spinning Black Hole (spin = 0.5)\n";
    std::cout << "   Mass: 1 solar mass, Spin: 0.5 (50% of maximum)\n";
    cosmic::physics::Kerr slowSpin(1.0, 0.5);

    r_horizon = slowSpin.eventHorizonRadius();
    r_photon = slowSpin.photonSphereRadius();
    r_isco = slowSpin.iscoRadius();

    std::cout << "   - Event Horizon:  " << r_horizon << "M\n";
    std::cout << "   - Photon Sphere:  " << r_photon << "M\n";
    std::cout << "   - ISCO:           " << r_isco << "M\n";
    std::cout << "   ➜ Notice: ISCO moved closer! Spinning allows tighter orbits.\n\n";

    // Create a rapidly spinning black hole (like the one in Interstellar!)
    std::cout << "3. Rapidly Spinning Black Hole (spin = 0.998)\n";
    std::cout << "   Mass: 1 solar mass, Spin: 0.998 (99.8% of maximum)\n";
    std::cout << "   (This is similar to Gargantua from Interstellar)\n";
    cosmic::physics::Kerr fastSpin(1.0, 0.998);

    r_horizon = fastSpin.eventHorizonRadius();
    r_photon = fastSpin.photonSphereRadius();
    r_isco = fastSpin.iscoRadius();

    std::cout << "   - Event Horizon:  " << r_horizon << "M\n";
    std::cout << "   - Photon Sphere:  " << r_photon << "M\n";
    std::cout << "   - ISCO:           " << r_isco << "M\n";
    std::cout << "   ➜ Amazing! ISCO is only " << r_isco << "M - matter can orbit 5x closer!\n\n";

    // Show frame dragging effect
    std::cout << "4. Frame Dragging (Spacetime Rotation)\n";
    std::cout << "   At radius = 5M, above the equator:\n";

    double omega_slow = slowSpin.frameDraggingOmega(5.0, cosmic::constants::HALF_PI);
    double omega_fast = fastSpin.frameDraggingOmega(5.0, cosmic::constants::HALF_PI);

    std::cout << std::setprecision(6);
    std::cout << "   - Slow spin (0.5):  ω = " << omega_slow << " rad/s\n";
    std::cout << "   - Fast spin (0.998): ω = " << omega_fast << " rad/s\n";
    std::cout << "   ➜ The spinning black hole DRAGS spacetime around with it!\n\n";

    // Show ergosphere
    std::cout << "5. The Ergosphere (Energy Extraction Region)\n";
    std::cout << "   Fast-spinning black hole at equator:\n";

    double r_ergo_eq = fastSpin.ergosphereRadius(cosmic::constants::HALF_PI);
    std::cout << std::setprecision(2);
    std::cout << "   - Ergosphere radius: " << r_ergo_eq << "M\n";
    std::cout << "   - Event horizon:     " << r_horizon << "M\n";
    std::cout << "   ➜ Between these radii, you can extract energy from the black hole!\n";
    std::cout << "     (This is the Penrose Process - theoretical free energy!)\n\n";

    std::cout << "=== Summary ===\n";
    std::cout << "Spinning black holes are VERY different from non-rotating ones:\n";
    std::cout << "• Matter can orbit much closer (better for accretion disks)\n";
    std::cout << "• They drag spacetime around (frame dragging)\n";
    std::cout << "• They have an ergosphere where energy extraction is possible\n";
    std::cout << "• They're more efficient at converting mass to energy\n\n";

    std::cout << "Try changing the spin values in this code to see how things change!\n";

    return 0;
}
