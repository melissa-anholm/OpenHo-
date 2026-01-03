#include "galaxy.h"
#include "game.h"
#include "text_assets.h"
#include "rng.h"
#include "planet.h"
#include "utility.h"
#include "game_constants.h"
#include <cmath>
#include <iostream>

// ============================================================================
// Galaxy Constructor
// ============================================================================
Galaxy::Galaxy(const GalaxyGenerationParams& params, GameState* game_state)
{
	// Generate randomized planet names list
	std::vector<std::string> planet_names = generate_planet_names(params.n_planets, game_state);
		
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
std::vector<std::string> Galaxy::generate_planet_names(uint32_t n_planets, GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	const TextAssets& text_assets = *game_state->text_assets;
	const std::vector<std::string>& available_names = text_assets.get_planet_names();

	return generate_randomized_subset(available_names, n_planets, rng);
}

// ============================================================================
// Shape-Specific Initialization Methods
// ============================================================================
void Galaxy::initialize_planets_random(
	const GalaxyGenerationParams& params,
	const std::vector<std::string>& planet_names,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	
	// Calculate galaxy size using density-aware formula
	// Formula: gal_size = sqrt(n_planets) * (base + density_factor / density)
	gal_size = std::sqrt(double(params.n_planets)) * 
	           (GameConstants::Galaxy_Size_Scale_Base + GameConstants::Galaxy_Size_Scale_Density / params.density);
	
	// Initialize spatial grid for distance checking
	CheckDistanceSpatialGrid grid(GameConstants::min_planet_distance, gal_size);
	
	uint32_t planets_skipped = 0;
	
	for (uint32_t i = 0; i < params.n_planets; ++i)
	{
		uint32_t planet_id = i + 1;  // Start planet IDs from 1
		const std::string& planet_name = planet_names[i];
		
		// Random properties (generated once, reused if position needs to be regenerated)
		double true_gravity = GameConstants::min_gravity + rng.nextDouble() * (GameConstants::max_gravity - GameConstants::min_gravity);
		double true_temperature = GameConstants::min_temp + rng.nextDouble() * (GameConstants::max_temp - GameConstants::min_temp);
		int32_t metal = rng.nextInt32Range(GameConstants::min_metal, GameConstants::max_metal);
		
		// Try to find a valid position for this planet
		uint32_t attempt = 0;
		bool placed = false;
		
		while (attempt < GameConstants::Planet_Placement_Max_Attempts && !placed)
		{
			// Generate random position within galaxy bounds
			GalaxyCoord x_coord = rng.nextDouble() * gal_size;
			GalaxyCoord y_coord = rng.nextDouble() * gal_size;
			
			// Check if position is valid (far enough from other planets)
			if (grid.is_position_valid(x_coord, y_coord, GameConstants::min_planet_distance))
			{
				// Create planet and add to grid
				Planet planet(planet_id, planet_name, x_coord, y_coord, true_gravity, true_temperature, metal);
				planets.push_back(planet);
				grid.add_planet(x_coord, y_coord, planet_id);
				placed = true;
			}
			
			attempt++;
			
			// Debug output if exceeds threshold
			if (attempt > GameConstants::Planet_Placement_Debug_Threshold && attempt == GameConstants::Planet_Placement_Debug_Threshold + 1)
			{
				std::cerr << "Warning: Planet " << planet_id << " (" << planet_name 
				          << ") took " << attempt << " attempts to place\n";
			}
		}
		
		if (!placed)
		{
			planets_skipped++;
			std::cerr << "Failed to place planet " << planet_id << " (" << planet_name 
			          << ") after " << GameConstants::Planet_Placement_Max_Attempts << " attempts\n";
		}
	}
	
	if (planets_skipped > 0)
	{
		std::cerr << "Skipped " << planets_skipped << " planets due to placement failures\n";
	}
}

void Galaxy::initialize_planets_grid(
	const GalaxyGenerationParams& params,
	const std::vector<std::string>& planet_names,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	
	double planet_spacing = 4.0 + 2.0/(params.density);  // this is probably fine...
	int rows_num = int(std::sqrt(double(params.n_planets)));
	
	int k=0;
	for (uint32_t i = 0; i < (uint32_t)rows_num; i++)
	{
		for(int j=0; j<rows_num; j++)
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
	
	// Set gal_size based on grid
	gal_size = planet_spacing * rows_num;
}

void Galaxy::initialize_planets_spiral(
	const GalaxyGenerationParams& params,
	const std::vector<std::string>& planet_names,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	
	// Calculate size from planets and density
	gal_size = (std::sqrt(double(params.n_planets)) * 40.0) * params.density;
	
	// Initialize spatial grid for distance checking
	CheckDistanceSpatialGrid grid(GameConstants::min_planet_distance, gal_size);
	
	uint32_t planets_skipped = 0;
	
	// Placeholder: Spiral distribution
	// TODO: Implement spiral galaxy distribution algorithm
	for (uint32_t i = 0; i < params.n_planets; ++i)
	{
		uint32_t planet_id = i + 1;
		const std::string& planet_name = planet_names[i];
		
		// Random properties
		double true_gravity = GameConstants::min_gravity + rng.nextDouble() * (GameConstants::max_gravity - GameConstants::min_gravity);
		double true_temperature = GameConstants::min_temp + rng.nextDouble() * (GameConstants::max_temp - GameConstants::min_temp);
		int32_t metal = rng.nextInt32Range(GameConstants::min_metal, GameConstants::max_metal);
		
		// For now, use random distribution (will be replaced with spiral algorithm)
		uint32_t attempt = 0;
		bool placed = false;
		
		while (attempt < GameConstants::Planet_Placement_Max_Attempts && !placed)
		{
			GalaxyCoord x_coord = rng.nextDouble() * gal_size;
			GalaxyCoord y_coord = rng.nextDouble() * gal_size;
			
			if (grid.is_position_valid(x_coord, y_coord, GameConstants::min_planet_distance))
			{
				Planet planet(planet_id, planet_name, x_coord, y_coord, true_gravity, true_temperature, metal);
				planets.push_back(planet);
				grid.add_planet(x_coord, y_coord, planet_id);
				placed = true;
			}
			
			attempt++;
			
			if (attempt > GameConstants::Planet_Placement_Debug_Threshold && attempt == GameConstants::Planet_Placement_Debug_Threshold + 1)
			{
				std::cerr << "Warning: Planet " << planet_id << " (" << planet_name 
				          << ") took " << attempt << " attempts to place\n";
			}
		}
		
		if (!placed)
		{
			planets_skipped++;
			std::cerr << "Failed to place planet " << planet_id << " (" << planet_name 
			          << ") after " << GameConstants::Planet_Placement_Max_Attempts << " attempts\n";
		}
	}
	
	if (planets_skipped > 0)
	{
		std::cerr << "Skipped " << planets_skipped << " planets due to placement failures\n";
	}
}

void Galaxy::initialize_planets_circle(
	const GalaxyGenerationParams& params,
	const std::vector<std::string>& planet_names,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	
	// Calculate size from planets and density
	gal_size = (std::sqrt(double(params.n_planets)) * 40.0) * params.density;
	
	// Initialize spatial grid for distance checking
	CheckDistanceSpatialGrid grid(GameConstants::min_planet_distance, gal_size);
	
	uint32_t planets_skipped = 0;
	
	// Placeholder: Circle distribution
	// TODO: Implement circle galaxy distribution algorithm
	// FUTURE: Use Poisson disk sampling (Bridson's algorithm) for uniform distribution
	// See Documentation/PoissonDiskSamplingOverview.md for details
	// This will eliminate placement failures and ensure well-spaced planets
	for (uint32_t i = 0; i < params.n_planets; ++i)
	{
		uint32_t planet_id = i + 1;
		const std::string& planet_name = planet_names[i];
		
		// Random properties
		double true_gravity = GameConstants::min_gravity + rng.nextDouble() * (GameConstants::max_gravity - GameConstants::min_gravity);
		double true_temperature = GameConstants::min_temp + rng.nextDouble() * (GameConstants::max_temp - GameConstants::min_temp);
		int32_t metal = rng.nextInt32Range(GameConstants::min_metal, GameConstants::max_metal);
		
		// For now, use random distribution (will be replaced with circle algorithm)
		uint32_t attempt = 0;
		bool placed = false;
		
		while (attempt < GameConstants::Planet_Placement_Max_Attempts && !placed)
		{
			GalaxyCoord x_coord = rng.nextDouble() * gal_size;
			GalaxyCoord y_coord = rng.nextDouble() * gal_size;
			
			if (grid.is_position_valid(x_coord, y_coord, GameConstants::min_planet_distance))
			{
				Planet planet(planet_id, planet_name, x_coord, y_coord, true_gravity, true_temperature, metal);
				planets.push_back(planet);
				grid.add_planet(x_coord, y_coord, planet_id);
				placed = true;
			}
			
			attempt++;
			
			if (attempt > GameConstants::Planet_Placement_Debug_Threshold && attempt == GameConstants::Planet_Placement_Debug_Threshold + 1)
			{
				std::cerr << "Warning: Planet " << planet_id << " (" << planet_name 
				          << ") took " << attempt << " attempts to place\n";
			}
		}
		
		if (!placed)
		{
			planets_skipped++;
			std::cerr << "Failed to place planet " << planet_id << " (" << planet_name 
			          << ") after " << GameConstants::Planet_Placement_Max_Attempts << " attempts\n";
		}
	}
	
	if (planets_skipped > 0)
	{
		std::cerr << "Skipped " << planets_skipped << " planets due to placement failures\n";
	}
}

void Galaxy::initialize_planets_ring(
	const GalaxyGenerationParams& params,
	const std::vector<std::string>& planet_names,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	
	// Calculate size from planets and density
	gal_size = (std::sqrt(double(params.n_planets)) * 40.0) * params.density;
	
	// Initialize spatial grid for distance checking
	CheckDistanceSpatialGrid grid(GameConstants::min_planet_distance, gal_size);
	
	uint32_t planets_skipped = 0;
	
	// Placeholder: Ring distribution
	// TODO: Implement ring galaxy distribution algorithm
	for (uint32_t i = 0; i < params.n_planets; ++i)
	{
		uint32_t planet_id = i + 1;
		const std::string& planet_name = planet_names[i];
		
		// Random properties
		double true_gravity = GameConstants::min_gravity + rng.nextDouble() * (GameConstants::max_gravity - GameConstants::min_gravity);
		double true_temperature = GameConstants::min_temp + rng.nextDouble() * (GameConstants::max_temp - GameConstants::min_temp);
		int32_t metal = rng.nextInt32Range(GameConstants::min_metal, GameConstants::max_metal);
		
		// For now, use random distribution (will be replaced with ring algorithm)
		uint32_t attempt = 0;
		bool placed = false;
		
		while (attempt < GameConstants::Planet_Placement_Max_Attempts && !placed)
		{
			GalaxyCoord x_coord = rng.nextDouble() * gal_size;
			GalaxyCoord y_coord = rng.nextDouble() * gal_size;
			
			if (grid.is_position_valid(x_coord, y_coord, GameConstants::min_planet_distance))
			{
				Planet planet(planet_id, planet_name, x_coord, y_coord, true_gravity, true_temperature, metal);
				planets.push_back(planet);
				grid.add_planet(x_coord, y_coord, planet_id);
				placed = true;
			}
			
			attempt++;
			
			if (attempt > GameConstants::Planet_Placement_Debug_Threshold && attempt == GameConstants::Planet_Placement_Debug_Threshold + 1)
			{
				std::cerr << "Warning: Planet " << planet_id << " (" << planet_name 
				          << ") took " << attempt << " attempts to place\n";
			}
		}
		
		if (!placed)
		{
			planets_skipped++;
			std::cerr << "Failed to place planet " << planet_id << " (" << planet_name 
			          << ") after " << GameConstants::Planet_Placement_Max_Attempts << " attempts\n";
		}
	}
	
	if (planets_skipped > 0)
	{
		std::cerr << "Skipped " << planets_skipped << " planets due to placement failures\n";
	}
}

void Galaxy::initialize_planets_cluster(
	const GalaxyGenerationParams& params,
	const std::vector<std::string>& planet_names,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	
	// Calculate size from planets and density
	gal_size = (std::sqrt(double(params.n_planets)) * 40.0) * params.density;
	
	// Initialize spatial grid for distance checking
	CheckDistanceSpatialGrid grid(GameConstants::min_planet_distance, gal_size);
	
	uint32_t planets_skipped = 0;
	
	// Placeholder: Cluster distribution
	// TODO: Implement cluster galaxy distribution algorithm
	for (uint32_t i = 0; i < params.n_planets; ++i)
	{
		uint32_t planet_id = i + 1;
		const std::string& planet_name = planet_names[i];
		
		// Random properties
		double true_gravity = GameConstants::min_gravity + rng.nextDouble() * (GameConstants::max_gravity - GameConstants::min_gravity);
		double true_temperature = GameConstants::min_temp + rng.nextDouble() * (GameConstants::max_temp - GameConstants::min_temp);
		int32_t metal = rng.nextInt32Range(GameConstants::min_metal, GameConstants::max_metal);
		
		// For now, use random distribution (will be replaced with cluster algorithm)
		uint32_t attempt = 0;
		bool placed = false;
		
		while (attempt < GameConstants::Planet_Placement_Max_Attempts && !placed)
		{
			GalaxyCoord x_coord = rng.nextDouble() * gal_size;
			GalaxyCoord y_coord = rng.nextDouble() * gal_size;
			
			if (grid.is_position_valid(x_coord, y_coord, GameConstants::min_planet_distance))
			{
				Planet planet(planet_id, planet_name, x_coord, y_coord, true_gravity, true_temperature, metal);
				planets.push_back(planet);
				grid.add_planet(x_coord, y_coord, planet_id);
				placed = true;
			}
			
			attempt++;
			
			if (attempt > GameConstants::Planet_Placement_Debug_Threshold && attempt == GameConstants::Planet_Placement_Debug_Threshold + 1)
			{
				std::cerr << "Warning: Planet " << planet_id << " (" << planet_name 
				          << ") took " << attempt << " attempts to place\n";
			}
		}
		
		if (!placed)
		{
			planets_skipped++;
			std::cerr << "Failed to place planet " << planet_id << " (" << planet_name 
			          << ") after " << GameConstants::Planet_Placement_Max_Attempts << " attempts\n";
		}
	}
	
	if (planets_skipped > 0)
	{
		std::cerr << "Skipped " << planets_skipped << " planets due to placement failures\n";
	}
}
