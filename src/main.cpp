#include "core/Application.hpp"
#include <iostream>
#include <exception>

int main() {
    try {
        // Create application
        cosmic::core::Application app(1280, 720, "Cosmic Simulator - Black Hole");

        // Initialize
        if (!app.initialize()) {
            std::cerr << "Failed to initialize application\n";
            return -1;
        }

        // Run main loop
        app.run();

        // Cleanup is handled automatically by destructors

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << "\n";
        return -1;
    } catch (...) {
        std::cerr << "Unknown exception caught\n";
        return -1;
    }
}
