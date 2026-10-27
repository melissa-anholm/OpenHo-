#include "galaxy.h"
#include "game.h"
#include "game_constants.h"
#include "utility.h"
#include <cmath>

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

std::vector<PlanetCoord> Galaxy::generate_coordinates_random(
	const GalaxyGenerationParams& params,
	DeterministicRNG& rng)
{
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
	std::cout << "Attempted to generate a galaxy of size " << original_gal_size << " x " << original_gal_size
	          << " with " << params.n_planets << " planets.\n";
	std::cout << "Actual galaxy generated with size " << gal_size << " x " << gal_size
	          << " with " << planets_placed << " planets.\n";
}
