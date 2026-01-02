#include "galaxy.h"
#include "game.h"
#include "text_assets.h"
#include "rng.h"
#include "planet.h"
#include "utility.h"
#include <cmath>

// ============================================================================
// Galaxy Constructor
// ============================================================================
Galaxy::Galaxy(const GalaxyGenerationParams& params, GameState* game_state)
{
	// Generate randomized planet names list
	std::vector<std::string> planet_names = generate_planet_names(params.n_planets);
		
	// Dispatch to shape-specific initialization
	switch(params.shape)
	{
		case GALAXY_RANDOM:
			initialize_planets_random(params, planet_names, game_state);
			break;
		case GALAXY_SPIRAL:
			initialize_planets_spiral(params, planet_names, game_state);
			break;
		case GALAXY_CIRCLE:
			initialize_planets_circle(params, planet_names, game_state);
			break;
		case GALAXY_RING:
			initialize_planets_ring(params, planet_names, game_state);
			break;
		case GALAXY_CLUSTER:
			initialize_planets_cluster(params, planet_names, game_state);
			break;
		case GALAXY_GRID:
			initialize_planets_grid(params, planet_names, game_state);
			break;
		default:
			// Fallback to random if unknown shape
			initialize_planets_random(params, planet_names, game_state);
			break;
	}
}

// ============================================================================
// Helper: Generate Randomized Planet Names
// ============================================================================
std::vector<std::string> Galaxy::generate_planet_names(uint32_t n_planets)
{
	DeterministicRNG& rng = game_state->get_rng();
	const TextAssets& text_assets = *game_state->text_assets;
	const std::vector<std::string>& available_names = text_assets.get_planet_names();

	return generate_randomized_subset(available_names, n_planets, rng);
}
// TODO:  add a utility function to make sure that planets don't get too close together.  
// Remember to put the planets back somewhere else afterwards.  


// ============================================================================
// Shape-Specific Initialization Methods
// ============================================================================
void Galaxy::initialize_planets_random(
	const GalaxyGenerationParams& params,
	const std::vector<std::string>& planet_names,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	
	// Calculate size from planets and density.  This will definitely have to be adjusted later.  
	gal_size = (std::sqrt(double(params.n_planets)) * 40.0) * params.density;
	
	for (uint32_t i = 0; i < params.n_planets; ++i)
	{
		uint32_t planet_id = i + 1;  // Start planet IDs from 1
		const std::string& planet_name = planet_names[i];
		
		// Random position within galaxy bounds
		GalaxyCoord x_coord = rng.nextDouble() * gal_size;
		GalaxyCoord y_coord = rng.nextDouble() * gal_size;
		
		// Random properties (using deterministic RNG)
		double true_gravity = GameConstants::min_gravity + rng.nextDouble() * (GameConstants::max_gravity - GameConstants::min_gravity);
		double true_temperature = GameConstants::min_temp + rng.nextDouble() * (GameConstants::max_temp - GameConstants::min_temp);
		int32_t metal = rng.nextInt32Range(GameConstants::min_metal, GameConstants::max_metal);
		
		// Create planet using constructor
		Planet planet(planet_id, planet_name, x_coord, y_coord, true_gravity, true_temperature, metal);
		
		planets.push_back(planet);
	}
}
void Galaxy::initialize_planets_grid(
	const GalaxyGenerationParams& params,
	const std::vector<std::string>& planet_names,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	
	double planet_spacing = 4.0 + 2.0/(params.density) ;  // this is probably fine...
	int rows_num = int(std::sqrt(double(params.n_planets));
	uint32_t true_n_planets = rows_num*rows_num;  // same size or smaller, so we won't run out of names.
	
	int k=0;
	for (uint32_t i = 0; i < rows_num; j++)
	{
		for(int j=0; j<row_nums; j++)
		{
			GalaxyCoord x_coord = planet_spacing * i;
			GalaxyCoord y_coord = planet_spacing * j;
			
			uint32_t planet_id = k + 1;
			const std::string& planet_name = planet_names[k];
			k++;
			
			// Random properties (using deterministic RNG)
			double true_gravity = GameConstants::min_gravity + rng.nextDouble() * (GameConstants::max_gravity - GameConstants::min_gravity);
			double true_temperature = GameConstants::min_temp + rng.nextDouble() * (GameConstants::max_temp - GameConstants::min_temp);
			int32_t metal = rng.nextInt32Range(GameConstants::min_metal, GameConstants::max_metal);
			
			Planet planet(planet_id, planet_name, x_coord, y_coord, true_gravity, true_temperature, metal);
			planets.push_back(planet);
		}
	}
}





void Galaxy::initialize_planets_spiral(
	const GalaxyGenerationParams& params,
	const std::vector<std::string>& planet_names,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	
	// Placeholder: Spiral distribution
	// TODO: Implement spiral galaxy distribution algorithm
	for (uint32_t i = 0; i < params.n_planets; ++i)
	{
		uint32_t planet_id = i + 1;
		const std::string& planet_name = planet_names[i];
		
		// For now, use random distribution (will be replaced with spiral algorithm)
		GalaxyCoord x_coord = rng.nextDouble() * gal_size;
		GalaxyCoord y_coord = rng.nextDouble() * gal_size;
		
		// Random properties (using deterministic RNG)
		double true_gravity = GameConstants::min_gravity + rng.nextDouble() * (GameConstants::max_gravity - GameConstants::min_gravity);
		double true_temperature = GameConstants::min_temp + rng.nextDouble() * (GameConstants::max_temp - GameConstants::min_temp);
		int32_t metal = rng.nextInt32Range(GameConstants::min_metal, GameConstants::max_metal);
		
		Planet planet(planet_id, planet_name, x_coord, y_coord, true_gravity, true_temperature, metal);
		planets.push_back(planet);
	}
}

void Galaxy::initialize_planets_circle(
	const GalaxyGenerationParams& params,
	const std::vector<std::string>& planet_names,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	
	// Placeholder: Circle distribution
	// TODO: Implement circle galaxy distribution algorithm
	for (uint32_t i = 0; i < params.n_planets; ++i)
	{
		uint32_t planet_id = i + 1;
		const std::string& planet_name = planet_names[i];
		
		// For now, use random distribution (will be replaced with circle algorithm)
		GalaxyCoord x_coord = rng.nextDouble() * gal_size;
		GalaxyCoord y_coord = rng.nextDouble() * gal_size;
		
		// Random properties (using deterministic RNG)
		double true_gravity = GameConstants::min_gravity + rng.nextDouble() * (GameConstants::max_gravity - GameConstants::min_gravity);
		double true_temperature = GameConstants::min_temp + rng.nextDouble() * (GameConstants::max_temp - GameConstants::min_temp);
		int32_t metal = rng.nextInt32Range(GameConstants::min_metal, GameConstants::max_metal);
		
		Planet planet(planet_id, planet_name, x_coord, y_coord, true_gravity, true_temperature, metal);
		planets.push_back(planet);
	}
}

void Galaxy::initialize_planets_ring(
	const GalaxyGenerationParams& params,
	const std::vector<std::string>& planet_names,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	
	// Placeholder: Ring distribution
	// TODO: Implement ring galaxy distribution algorithm
	for (uint32_t i = 0; i < params.n_planets; ++i)
	{
		uint32_t planet_id = i + 1;
		const std::string& planet_name = planet_names[i];
		
		// For now, use random distribution (will be replaced with ring algorithm)
		GalaxyCoord x_coord = rng.nextDouble() * gal_size;
		GalaxyCoord y_coord = rng.nextDouble() * gal_size;
		
		// Random properties (using deterministic RNG)
		double true_gravity = GameConstants::min_gravity + rng.nextDouble() * (GameConstants::max_gravity - GameConstants::min_gravity);
		double true_temperature = GameConstants::min_temp + rng.nextDouble() * (GameConstants::max_temp - GameConstants::min_temp);
		int32_t metal = rng.nextInt32Range(GameConstants::min_metal, GameConstants::max_metal);
		
		Planet planet(planet_id, planet_name, x_coord, y_coord, true_gravity, true_temperature, metal);
		planets.push_back(planet);
	}
}

void Galaxy::initialize_planets_cluster(
	const GalaxyGenerationParams& params,
	const std::vector<std::string>& planet_names,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	
	// Placeholder: Cluster distribution
	// TODO: Implement cluster galaxy distribution algorithm
	for (uint32_t i = 0; i < params.n_planets; ++i)
	{
		uint32_t planet_id = i + 1;
		const std::string& planet_name = planet_names[i];
		
		// For now, use random distribution (will be replaced with cluster algorithm)
		GalaxyCoord x_coord = rng.nextDouble() * gal_size;
		GalaxyCoord y_coord = rng.nextDouble() * gal_size;
		
		// Random properties (using deterministic RNG)
		double true_gravity = GameConstants::min_gravity + rng.nextDouble() * (GameConstants::max_gravity - GameConstants::min_gravity);
		double true_temperature = GameConstants::min_temp + rng.nextDouble() * (GameConstants::max_temp - GameConstants::min_temp);
		int32_t metal = rng.nextInt32Range(GameConstants::min_metal, GameConstants::max_metal);
		
		Planet planet(planet_id, planet_name, x_coord, y_coord, true_gravity, true_temperature, metal);
		planets.push_back(planet);
	}
}

