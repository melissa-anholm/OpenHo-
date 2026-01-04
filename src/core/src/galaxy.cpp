#include "galaxy.h"
#include "game.h"
#include "text_assets.h"
#include "utility.h"
#include "game_constants.h"
#include <cmath>
#include <iostream>
#include <set>

Galaxy::Galaxy(const GalaxyGenerationParams& params, GameState* game_state)
{
	// Staged galaxy generation:
	// Phase 1: Generate all planet coordinates
	std::vector<PlanetCoord> all_coords = generate_planet_coordinates(params, game_state);
	
	// Phase 2: Select home planet coordinates
	uint32_t n_home_planets = game_state->get_players().size();
	std::vector<PlanetCoord> home_coords = select_home_planet_coordinates(all_coords, n_home_planets, game_state);
	
	// Phase 3: Generate planet names
	const std::vector<std::string> planet_names = generate_planet_names(all_coords.size(), game_state);
	
	// Phase 4: Generate planet parameters
	generate_planet_parameters(all_coords, home_coords, planet_names, game_state);
	
	// Calculate galaxy size from coordinates
	if (!all_coords.empty())
	{
		GalaxyCoord max_dist = 0;
		for (const auto& coord : all_coords)
		{
			GalaxyCoord dist = std::sqrt(coord.first * coord.first + coord.second * coord.second);
			if (dist > max_dist) max_dist = dist;
		}
		gal_size = max_dist * 2.0;
	}
	else
	{
		gal_size = 100.0;
	}
}

std::vector<std::string> Galaxy::generate_planet_names(
	uint32_t n_planets,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	
	return generate_randomized_subset(game_state->text_assets->get_planet_name_list(), n_planets, rng);
}

void Galaxy::initialize_planets_random(
	const GalaxyGenerationParams& params,
	const std::vector<std::string>& planet_names_unused,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	
	// Generate randomized planet names
	const std::vector<std::string> planet_names = generate_planet_names(params.n_planets, game_state);
	
	// Calculate galaxy size using density-aware formula with reduction factor
	// Formula: gal_size = sqrt(n_planets) * (base + density_factor / density) * (1 - reduction_factor)
	gal_size = std::sqrt(double(params.n_planets)) * 
	           (GameConstants::Galaxy_Size_Scale_Base + GameConstants::Galaxy_Size_Scale_Density / params.density) *
	           (1.0 - GameConstants::Galaxy_Size_Reduction_Factor);
	
	GalaxyCoord original_gal_size = gal_size;
	
	// Initialize spatial grid for distance checking
	// Use larger grid to account for potential expansion
	CheckDistanceSpatialGrid grid(GameConstants::min_planet_distance, gal_size * 2.0);
	
	// Galaxy is centered at (0, 0)
	GalaxyCoord half_size = gal_size / 2.0;
	GalaxyCoord min_x = -half_size;
	GalaxyCoord max_x = half_size;
	GalaxyCoord min_y = -half_size;
	GalaxyCoord max_y = half_size;
	
	uint32_t attempts_per_expansion = static_cast<uint32_t>(std::floor(std::sqrt(double(params.n_planets)) / 2.0));
	uint32_t max_attempts_per_planet = params.n_planets;
	uint32_t planets_skipped = 0;
	uint32_t planets_placed = 0;
	
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
		
		while (attempt < max_attempts_per_planet && !placed)
		{
			// Check if we need to expand the galaxy boundaries
			if (attempt > 0 && attempt % attempts_per_expansion == 0)
			{
				// Expand boundaries by 5% around center (0, 0)
				GalaxyCoord expansion_factor = 1.0 + GameConstants::Galaxy_Expansion_Factor;
				min_x *= expansion_factor;
				max_x *= expansion_factor;
				min_y *= expansion_factor;
				max_y *= expansion_factor;
				
				// Update gal_size to reflect new bounds
				gal_size = max_x - min_x;
			}
			
			// Generate random position within current galaxy bounds
			GalaxyCoord x_coord = min_x + rng.nextDouble() * (max_x - min_x);
			GalaxyCoord y_coord = min_y + rng.nextDouble() * (max_y - min_y);
			
			// Check if position is valid (far enough from other planets)
			if (grid.is_position_valid(x_coord, y_coord, GameConstants::min_planet_distance))
			{
				// Create planet and add to grid
				Planet planet(planet_id, planet_name, x_coord, y_coord, true_gravity, true_temperature, metal);
				planets.push_back(planet);
				grid.add_planet(x_coord, y_coord, planet_id);
				placed = true;
				planets_placed++;
			}
			
			attempt++;
		}
		
		if (!placed)
		{
			planets_skipped++;
		}
	}
	
	// Output summary
	std::cout << "Attempted to generate a galaxy of size " << original_gal_size << " x " << original_gal_size 
	          << " with " << params.n_planets << " planets.\n";
	std::cout << "Actual galaxy generated with size " << gal_size << " x " << gal_size 
	          << " with " << planets_placed << " planets.\n";
}

	void Galaxy::initialize_planets_spiral(
		const GalaxyGenerationParams& params,
		const std::vector<std::string>& planet_names,
		GameState* game_state)
	{
		DeterministicRNG& rng = game_state->get_rng();
		
		// Calculate size from planets and density
		gal_size = std::sqrt(double(params.n_planets)) * 
		           (GameConstants::Galaxy_Size_Scale_Base + GameConstants::Galaxy_Size_Scale_Density / params.density);
		
		// Initialize spatial grid for distance checking
		CheckDistanceSpatialGrid grid(GameConstants::min_planet_distance, gal_size);
		
		uint32_t planets_skipped = 0;
		
		// Placeholder: Spiral distribution
		// TODO: Implement spiral galaxy distribution algorithm
		// IMPORTANT: When implementing spiral shape, rework the "min planets" and "homeworld assignment" strategy
		// The current strategy (designed for RANDOM/GRID) may not work well for spiral distributions.
		// Consider integrating suitable home planet availability into this initialization routine.
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
		}
		
		if (!placed)
		{
			planets_skipped++;
		}
	}
}

void Galaxy::initialize_planets_circle(
	const GalaxyGenerationParams& params,
	const std::vector<std::string>& planet_names,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	
	// Calculate size from planets and density
	gal_size = std::sqrt(double(params.n_planets)) * 
	           (GameConstants::Galaxy_Size_Scale_Base + GameConstants::Galaxy_Size_Scale_Density / params.density);
	
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
		}
		
		if (!placed)
		{
			planets_skipped++;
		}
	}
}

void Galaxy::initialize_planets_ring(
	const GalaxyGenerationParams& params,
	const std::vector<std::string>& planet_names,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	
	// Calculate size from planets and density
	gal_size = std::sqrt(double(params.n_planets)) * 
	           (GameConstants::Galaxy_Size_Scale_Base + GameConstants::Galaxy_Size_Scale_Density / params.density);
	
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
		}
		
		if (!placed)
		{
			planets_skipped++;
		}
	}
}

void Galaxy::initialize_planets_cluster(
	const GalaxyGenerationParams& params,
	const std::vector<std::string>& planet_names,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	
	// Calculate size from planets and density
	gal_size = std::sqrt(double(params.n_planets)) * 
	           (GameConstants::Galaxy_Size_Scale_Base + GameConstants::Galaxy_Size_Scale_Density / params.density);
	
	// Initialize spatial grid for distance checking
	CheckDistanceSpatialGrid grid(GameConstants::min_planet_distance, gal_size);
	
	uint32_t planets_skipped = 0;
	
		// Placeholder: Cluster distribution
		// TODO: Implement cluster galaxy distribution algorithm
		// IMPORTANT: When implementing cluster shape, rework the "min planets" and "homeworld assignment" strategy
		// The current strategy (designed for RANDOM/GRID) may not work well for cluster distributions.
		// Consider integrating suitable home planet availability into this initialization routine.
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
		}
		
		if (!placed)
		{
			planets_skipped++;
		}
	}
}

void Galaxy::initialize_planets_grid(
	const GalaxyGenerationParams& params,
	const std::vector<std::string>& planet_names,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	
	double planet_spacing = 4.0 + 2.0/(params.density);  // this is probably fine...
	
	uint32_t cols_num = static_cast<uint32_t>(std::ceil(std::sqrt(double(params.n_planets))));
	uint32_t rows_num = static_cast<uint32_t>(std::ceil(double(params.n_planets) / cols_num));
	
	gal_size = std::max(cols_num, rows_num) * planet_spacing;
	
	uint32_t planet_idx = 0;
	for (uint32_t row = 0; row < rows_num && planet_idx < params.n_planets; ++row)
	{
		for (uint32_t col = 0; col < cols_num && planet_idx < params.n_planets; ++col)
		{
			uint32_t planet_id = planet_idx + 1;
			const std::string& planet_name = planet_names[planet_idx];
			
			// Random properties
			double true_gravity = GameConstants::min_gravity + rng.nextDouble() * (GameConstants::max_gravity - GameConstants::min_gravity);
			double true_temperature = GameConstants::min_temp + rng.nextDouble() * (GameConstants::max_temp - GameConstants::min_temp);
			int32_t metal = rng.nextInt32Range(GameConstants::min_metal, GameConstants::max_metal);
			
			// Grid positions
			GalaxyCoord x_coord = col * planet_spacing;
			GalaxyCoord y_coord = row * planet_spacing;
			
			Planet planet(planet_id, planet_name, x_coord, y_coord, true_gravity, true_temperature, metal);
			planets.push_back(planet);
			
			planet_idx++;
		}
	}
}


// ============================================================================
// Staged Galaxy Generation Methods
// ============================================================================

std::vector<PlanetCoord> Galaxy::generate_planet_coordinates(
	const GalaxyGenerationParams& params,
	GameState* game_state)
{
	// Dispatch to shape-specific coordinate generation
	switch (params.shape)
	{
		case GALAXY_RANDOM:
			return generate_coordinates_random(params, game_state);
		case GALAXY_SPIRAL:
			return generate_coordinates_spiral(params, game_state);
		case GALAXY_CIRCLE:
			return generate_coordinates_circle(params, game_state);
		case GALAXY_RING:
			return generate_coordinates_ring(params, game_state);
		case GALAXY_CLUSTER:
			return generate_coordinates_cluster(params, game_state);
		case GALAXY_GRID:
			return generate_coordinates_grid(params, game_state);
		default:
			return std::vector<PlanetCoord>();
	}
}

std::vector<PlanetCoord> Galaxy::select_home_planet_coordinates(
	const std::vector<PlanetCoord>& all_coords,
	uint32_t n_home_planets,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	
	// Create a mutable copy for shuffling
	std::vector<PlanetCoord> shuffled = all_coords;
	
	// Fisher-Yates shuffle
	for (size_t i = shuffled.size() - 1; i > 0; --i)
	{
		size_t j = rng.nextInt32Range(0, i);
		std::swap(shuffled[i], shuffled[j]);
	}
	
	// Take first n_home_planets as home planets
	std::vector<PlanetCoord> home_coords(shuffled.begin(), shuffled.begin() + std::min((size_t)n_home_planets, shuffled.size()));
	return home_coords;
}

void Galaxy::generate_planet_parameters(
	const std::vector<PlanetCoord>& all_coords,
	const std::vector<PlanetCoord>& home_coords,
	const std::vector<std::string>& planet_names,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	
	// Create a set of home coordinates for quick lookup
	std::set<PlanetCoord> home_coord_set(home_coords.begin(), home_coords.end());
	
	// Generate planets for all coordinates
	uint32_t planet_id = 1;
	for (size_t i = 0; i < all_coords.size() && i < planet_names.size(); ++i)
	{
		const auto& coord = all_coords[i];
		bool is_home_planet = home_coord_set.count(coord) > 0;
		
		// Generate parameters
		double true_gravity;
		double true_temperature;
		
		if (is_home_planet)
		{
			// Home planets have constrained gravity
			true_gravity = GameConstants::Starting_Planet_Min_Gravity + 
						  rng.nextDouble() * (GameConstants::Starting_Planet_Max_Gravity - GameConstants::Starting_Planet_Min_Gravity);
			true_temperature = GameConstants::min_temp + rng.nextDouble() * (GameConstants::max_temp - GameConstants::min_temp);
		}
		else
		{
			// Regular planets have full range
			true_gravity = GameConstants::min_gravity + rng.nextDouble() * (GameConstants::max_gravity - GameConstants::min_gravity);
			true_temperature = GameConstants::min_temp + rng.nextDouble() * (GameConstants::max_temp - GameConstants::min_temp);
		}
		
		int32_t metal = rng.nextInt32Range(GameConstants::min_metal, GameConstants::max_metal);
		
		// Create planet
		Planet planet(planet_id++, planet_names[i], coord.first, coord.second, true_gravity, true_temperature, metal);
		planets.push_back(planet);
		
		// Track home planet indices
		if (is_home_planet)
		{
			home_planet_indices.push_back(planets.size() - 1);
		}
	}
}

// ============================================================================
// Shape-Specific Coordinate Generation Methods
// ============================================================================

std::vector<PlanetCoord> Galaxy::generate_coordinates_random(
	const GalaxyGenerationParams& params,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	std::vector<PlanetCoord> coords;
	
	// Calculate galaxy size using density-aware formula with reduction factor
	GalaxyCoord gal_size = std::sqrt(double(params.n_planets)) * 
	           (GameConstants::Galaxy_Size_Scale_Base + GameConstants::Galaxy_Size_Scale_Density / params.density) *
	           (1.0 - GameConstants::Galaxy_Size_Reduction_Factor);
	
	// Initialize spatial grid for distance checking
	CheckDistanceSpatialGrid grid(GameConstants::min_planet_distance, gal_size * 2.0);
	
	// Galaxy is centered at (0, 0)
	GalaxyCoord half_size = gal_size / 2.0;
	GalaxyCoord min_x = -half_size;
	GalaxyCoord max_x = half_size;
	GalaxyCoord min_y = -half_size;
	GalaxyCoord max_y = half_size;
	
	uint32_t attempts_per_expansion = static_cast<uint32_t>(std::floor(std::sqrt(double(params.n_planets)) / 2.0));
	uint32_t max_attempts_per_planet = params.n_planets;
	
	for (uint32_t i = 0; i < params.n_planets; ++i)
	{
		uint32_t attempt = 0;
		bool placed = false;
		
		while (attempt < max_attempts_per_planet && !placed)
		{
			// Check if we need to expand the galaxy boundaries
			if (attempt > 0 && attempt % attempts_per_expansion == 0)
			{
				// Expand boundaries by 5% around center (0, 0)
				GalaxyCoord expansion_factor = 1.0 + GameConstants::Galaxy_Expansion_Factor;
				min_x *= expansion_factor;
				max_x *= expansion_factor;
				min_y *= expansion_factor;
				max_y *= expansion_factor;
			}
			
			// Generate random position within current galaxy bounds
			GalaxyCoord x_coord = min_x + rng.nextDouble() * (max_x - min_x);
			GalaxyCoord y_coord = min_y + rng.nextDouble() * (max_y - min_y);
			
			// Check if position is valid (far enough from other planets)
			if (grid.is_position_valid(x_coord, y_coord, GameConstants::min_planet_distance))
			{
				coords.push_back({x_coord, y_coord});
				grid.add_planet(x_coord, y_coord, i + 1);
				placed = true;
			}
			
			attempt++;
		}
	}
	
	return coords;
}

std::vector<PlanetCoord> Galaxy::generate_coordinates_spiral(
	const GalaxyGenerationParams& params,
	GameState* game_state)
{
	// TODO: Implement spiral coordinate generation
	return std::vector<PlanetCoord>();
}

std::vector<PlanetCoord> Galaxy::generate_coordinates_circle(
	const GalaxyGenerationParams& params,
	GameState* game_state)
{
	// TODO: Implement circle coordinate generation
	return std::vector<PlanetCoord>();
}

std::vector<PlanetCoord> Galaxy::generate_coordinates_ring(
	const GalaxyGenerationParams& params,
	GameState* game_state)
{
	// TODO: Implement ring coordinate generation
	return std::vector<PlanetCoord>();
}

std::vector<PlanetCoord> Galaxy::generate_coordinates_cluster(
	const GalaxyGenerationParams& params,
	GameState* game_state)
{
	// TODO: Implement cluster coordinate generation
	return std::vector<PlanetCoord>();
}

std::vector<PlanetCoord> Galaxy::generate_coordinates_grid(
	const GalaxyGenerationParams& params,
	GameState* game_state)
{
	std::vector<PlanetCoord> coords;
	
	double planet_spacing = 4.0 + 2.0/(params.density);
	
	uint32_t cols_num = static_cast<uint32_t>(std::ceil(std::sqrt(double(params.n_planets))));
	uint32_t rows_num = static_cast<uint32_t>(std::ceil(double(params.n_planets) / cols_num));
	
	uint32_t planet_idx = 0;
	for (uint32_t row = 0; row < rows_num && planet_idx < params.n_planets; ++row)
	{
		for (uint32_t col = 0; col < cols_num && planet_idx < params.n_planets; ++col)
		{
			GalaxyCoord x_coord = col * planet_spacing;
			GalaxyCoord y_coord = row * planet_spacing;
			
			coords.push_back({x_coord, y_coord});
			planet_idx++;
		}
	}
	
	return coords;
}
