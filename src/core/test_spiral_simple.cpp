#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <limits>
#include "include/galaxy.h"
#include "include/rng.h"
#include <cmath>
#include "include/game_constants.h"

int main() {
    std::cout << "=== Spiral Galaxy Coordinate Generation Test ===" << std::endl << std::endl;
    
    try {
        // Create RNG for testing
        DeterministicRNG rng(12345, 54321);
        
        // Create galaxy parameters for spiral shape
        GalaxyGenerationParams params(
            150,              // n_planets
            3,                // n_players
            0.5,              // density
            GALAXY_SPIRAL,    // shape
            12345             // seed
        );
        
        std::cout << "Test Parameters:" << std::endl;
        std::cout << "  Shape: SPIRAL" << std::endl;
        std::cout << "  Players: " << params.n_players << std::endl;
        std::cout << "  Requested Planets: " << params.n_planets << std::endl;
        std::cout << "  Density: " << params.density << std::endl;
        std::cout << std::endl;
        
        // Test spiral helper functions
        std::cout << "Spiral Helper Functions Test:" << std::endl;
        
        // Test with different angular spans
        double test_angles[] = {M_PI / 4, M_PI / 2, 3 * M_PI / 4, M_PI};
        
        for (double delta_theta : test_angles) {
            double a = 100.0 / std::sqrt(delta_theta);
            
            // Calculate arc length
            double core_radius = 20.0;
            double ratio = 3.0;
            double outer_radius = core_radius * ratio;
            
            double theta_core = (core_radius / a) * (core_radius / a);
            double theta_outer = delta_theta;
            
            double r_core = core_radius;
            double r_outer = outer_radius;
            double r_avg = (r_core + r_outer) / 2.0;
            
            double arc_length = std::sqrt(
                (r_outer - r_core) * (r_outer - r_core) + 
                (r_avg * (theta_outer - theta_core)) * (r_avg * (theta_outer - theta_core))
            );
            
            std::cout << "  Angular span: " << std::fixed << std::setprecision(4) << delta_theta 
                      << " rad (" << (delta_theta * 180 / M_PI) << "°)" << std::endl;
            std::cout << "    Tightness (a): " << std::setprecision(4) << a << std::endl;
            std::cout << "    Arc length: " << std::setprecision(2) << arc_length << " units" << std::endl;
            std::cout << "    Estimated planets per arm: " << (int)(arc_length / GameConstants::min_planet_distance) << std::endl;
            std::cout << std::endl;
        }
        
        // Test Fermat spiral point generation
        std::cout << "Fermat Spiral Point Generation:" << std::endl;
        double test_a = 100.0 / std::sqrt(M_PI / 2);
        double test_theta = M_PI / 4;
        
        std::cout << "  Testing with a = " << std::fixed << std::setprecision(4) << test_a << std::endl;
        std::cout << "  Theta values and corresponding radii:" << std::endl;
        
        for (int i = 0; i <= 4; ++i) {
            double theta = (i / 4.0) * (M_PI / 2);
            double r = test_a * std::sqrt(theta);
            std::cout << "    θ = " << std::setprecision(4) << theta 
                      << " → r = " << std::setprecision(4) << r << std::endl;
        }
        std::cout << std::endl;
        
        // Test parameter selection ranges
        std::cout << "Parameter Selection Ranges:" << std::endl;
        
        // Angular span range
        std::cout << "  Angular span: [" << std::setprecision(4) << (M_PI / 4) 
                  << ", " << M_PI << "] radians" << std::endl;
        std::cout << "    In degrees: [45°, 180°]" << std::endl;
        
        // Outer/core ratio range
        std::cout << "  Outer/core ratio: [2.0, 6.0]" << std::endl;
        
        // Overlap calculation
        std::cout << "  Overlap calculation for different core radii:" << std::endl;
        for (double core_r : {10.0, 20.0, 50.0, 100.0}) {
            double overlap = std::max(6.0, 0.15 * core_r);
            std::cout << "    Core radius " << std::setprecision(0) << core_r 
                      << " → overlap = " << std::setprecision(2) << overlap << std::endl;
        }
        std::cout << std::endl;
        
        std::cout << "=== Test Complete ===" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}
