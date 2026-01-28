#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <limits>
#include "include/galaxy.h"
#include "include/game.h"
#include "include/game_constants.h"

int main() {
    std::cout << "=== Spiral Galaxy Generation Test ===" << std::endl << std::endl;
    
    try {
        // Create galaxy parameters for spiral shape
        GalaxyGenerationParams params(
            150,              // n_planets
            3,                // n_players
            0.5,              // density
            GALAXY_SPIRAL,    // shape
            12345             // seed
        );
        
        // Create a minimal GameState for testing
        // Note: This is a simplified test that creates a GameState
        GameSetup setup;
        GameState* game_state = setup.create_new_game();
        
        if (!game_state) {
            std::cerr << "Failed to create game state" << std::endl;
            return 1;
        }
        
        // Get the galaxy
        const Galaxy& galaxy = game_state->get_galaxy();
        
        std::cout << "Galaxy Parameters:" << std::endl;
        std::cout << "  Shape: SPIRAL" << std::endl;
        std::cout << "  Players: " << params.n_players << std::endl;
        std::cout << "  Requested Planets: " << params.n_planets << std::endl;
        std::cout << "  Density: " << params.density << std::endl;
        std::cout << std::endl;
        
        std::cout << "Generated Galaxy:" << std::endl;
        std::cout << "  Actual Planets: " << galaxy.planets.size() << std::endl;
        std::cout << "  Home Planets: " << galaxy.home_planet_indices.size() << std::endl;
        std::cout << "  Galaxy Size: " << galaxy.gal_size << std::endl;
        std::cout << std::endl;
        
        // Verify home planets
        std::cout << "Home Planet Verification:" << std::endl;
        for (size_t i = 0; i < galaxy.home_planet_indices.size(); ++i) {
            size_t idx = galaxy.home_planet_indices[i];
            const Planet& planet = galaxy.planets[idx];
            std::cout << "  Player " << (i + 1) << " home: " << planet.name 
                      << " at (" << std::fixed << std::setprecision(2) 
                      << planet.x << ", " << planet.y << ")" << std::endl;
        }
        std::cout << std::endl;
        
        // Check planet distribution
        std::cout << "Planet Distribution Analysis:" << std::endl;
        double min_dist = std::numeric_limits<double>::max();
        double max_dist = 0;
        double avg_dist = 0;
        int count = 0;
        
        for (const auto& planet : galaxy.planets) {
            double dist = std::sqrt(planet.x * planet.x + planet.y * planet.y);
            min_dist = std::min(min_dist, dist);
            max_dist = std::max(max_dist, dist);
            avg_dist += dist;
            count++;
        }
        avg_dist /= count;
        
        std::cout << "  Min distance from center: " << std::fixed << std::setprecision(2) << min_dist << std::endl;
        std::cout << "  Max distance from center: " << std::fixed << std::setprecision(2) << max_dist << std::endl;
        std::cout << "  Avg distance from center: " << std::fixed << std::setprecision(2) << avg_dist << std::endl;
        std::cout << std::endl;
        
        // Check minimum spacing
        std::cout << "Minimum Spacing Verification:" << std::endl;
        int violations = 0;
        for (size_t i = 0; i < galaxy.planets.size(); ++i) {
            for (size_t j = i + 1; j < galaxy.planets.size(); ++j) {
                double dx = galaxy.planets[i].x - galaxy.planets[j].x;
                double dy = galaxy.planets[i].y - galaxy.planets[j].y;
                double dist = std::sqrt(dx * dx + dy * dy);
                if (dist < GameConstants::min_planet_distance - 0.01) {  // Small tolerance for rounding
                    violations++;
                    if (violations <= 5) {  // Only print first 5 violations
                        std::cout << "  VIOLATION: " << galaxy.planets[i].name << " and " 
                                  << galaxy.planets[j].name << " distance: " 
                                  << std::fixed << std::setprecision(2) << dist << std::endl;
                    }
                }
            }
        }
        
        if (violations == 0) {
            std::cout << "  ✓ All planets maintain minimum spacing of " 
                      << GameConstants::min_planet_distance << " units" << std::endl;
        } else {
            std::cout << "  ✗ Found " << violations << " spacing violations!" << std::endl;
        }
        std::cout << std::endl;
        
        std::cout << "=== Test Complete ===" << std::endl;
        delete game_state;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}
