#include "galaxy.h"
#include "game.h"
#include "text_assets.h"
#include "rng.h"
#include "planet.h"
#include <string>

// ============================================================================
// Galaxy Constructor
// ============================================================================

Galaxy::Galaxy(const GalaxyGenerationParams& params, GameState* game_state)
{
	// Calculate size from planets and density.  Later, also use the shape parameter here too.
	gal_size = (std::sqrt( double(params.n_planets) ) * 20.0) * params.density;
	
	// planets vector is default-constructed (empty)
	
	// Initialize planets
	initialize_planets(params, game_state);
}

// ============================================================================
// Galaxy Initialization
// ============================================================================
void Galaxy::initialize_planets(const GalaxyGenerationParams& params, GameState* game_state)
{
	// Generate planets based on galaxy parameters
	// Uses RNG and TextAssets from GameState
	DeterministicRNG& rng = game_state->get_rng();
	const TextAssets& text_assets = *game_state->text_assets;
	const std::vector<std::string>& available_planet_names = text_assets.get_planet_names();
	
	// For now, create planets according to n_planets parameter
	for (uint32_t i = 0; i < params.n_planets; ++i)
	{
		uint32_t planet_id = i + 1;  // Start planet IDs from 1
		
		// Get planet name from TextAssets (cycle through available names if needed)
		std::string planet_name;
		if (i < available_planet_names.size())
		{
			planet_name = available_planet_names[i];
		}
		else
		{
			// Fallback if we run out of names
			planet_name = "Planet_" + std::to_string(planet_id);
		}
		
		// Random position within galaxy bounds
		GalaxyCoord x_coord = rng.nextDouble() * gal_size;
		GalaxyCoord y_coord = rng.nextDouble() * gal_size;
		
		// Random properties (using deterministic RNG)
		double true_gravity = 0.3 + rng.nextDouble() * 3.7;  // 0.3 to 4.0
		double true_temperature = -400.0 + rng.nextDouble() * 800.0;  // -50 to 150
		int32_t metal = rng.nextInt32Range(0, 30000);
		
		// Create planet using constructor
		Planet planet(planet_id, planet_name, x_coord, y_coord, true_gravity, true_temperature, metal);
		
		planets.push_back(planet);
	}
}
