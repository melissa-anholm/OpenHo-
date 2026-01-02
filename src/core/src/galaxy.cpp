#include "galaxy.h"
#include "game.h"
#include "text_assets.h"
#include "rng.h"
#include "planet.h"
#include "utility.h"
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

std::vector<std::string> Galaxy::generate_planet_names(
	uint32_t n_planets,
	const std::vector<std::string>& available_names,
	DeterministicRNG& rng)
{
	// Generate randomized planet names using the utility function
	return generate_randomized_subset(available_names, n_planets, rng);
}

void Galaxy::initialize_planets(const GalaxyGenerationParams& params, GameState* game_state)
{
	// Generate planets based on galaxy parameters
	// Uses RNG and TextAssets from GameState
	DeterministicRNG& rng = game_state->get_rng();
	const TextAssets& text_assets = *game_state->text_assets;
	const std::vector<std::string>& available_planet_names = text_assets.get_planet_names();
	
	// Step 1: Generate randomized planet names (no repeats, random order)
	std::vector<std::string> planet_names = generate_planet_names(
		params.n_planets,
		available_planet_names,
		rng);
	
	// Step 2: Create planets with all properties
	for (uint32_t i = 0; i < params.n_planets; ++i)
	{
		uint32_t planet_id = i + 1;  // Start planet IDs from 1
		const std::string& planet_name = planet_names[i];
		
		// Random position within galaxy bounds
		GalaxyCoord x_coord = rng.nextDouble() * gal_size;
		GalaxyCoord y_coord = rng.nextDouble() * gal_size;
		
		// Random properties (using deterministic RNG)
		double true_gravity = 0.3 + rng.nextDouble() * 3.7;  // 0.3 to 4.0
		double true_temperature = -400.0 + rng.nextDouble() * 800.0;  // -400 to 400
		int32_t metal = rng.nextInt32Range(0, 30000);
		
		// Create planet using constructor
		Planet planet(planet_id, planet_name, x_coord, y_coord, true_gravity, true_temperature, metal);
		
		planets.push_back(planet);
	}
}
