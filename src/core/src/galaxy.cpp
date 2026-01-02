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
	// Use pre-calculated gal_size from params
	min_x = -params.gal_size;
	max_x = params.gal_size;
	min_y = -params.gal_size;
	max_y = params.gal_size;
	
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
		GalaxyCoord x = min_x + rng.nextDouble() * (max_x - min_x);
		GalaxyCoord y = min_y + rng.nextDouble() * (max_y - min_y);
		
		// Random properties (using deterministic RNG)
		double true_gravity = 0.5 + rng.nextDouble() * 1.5;  // 0.5 to 2.0
		double true_temperature = -50.0 + rng.nextDouble() * 200.0;  // -50 to 150
		int32_t metal = 100 + rng.nextInt32Range(0, 500);
		
		// Create planet using constructor
		Planet planet(planet_id, planet_name, x, y, true_gravity, true_temperature, metal);
		
		planets.push_back(planet);
	}
}
