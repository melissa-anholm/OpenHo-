#include "galaxy.h"
#include "game.h"
#include "game_constants.h"
#include <cmath>

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

std::vector<PlanetCoord> Galaxy::generate_coordinates_grid(
	const GalaxyGenerationParams& params,
	DeterministicRNG& rng)
{
	(void)rng;  // Not used for grid generation

	std::vector<PlanetCoord> coords;

	// Calculate grid dimensions
	uint32_t grid_size = static_cast<uint32_t>(std::ceil(std::sqrt(double(params.n_planets))));
	double spacing = GameConstants::min_planet_distance * 1.5;  // Slightly more than minimum

	// Center the grid at (0, 0)
	double offset_x = -(grid_size - 1) * spacing / 2.0;
	double offset_y = -(grid_size - 1) * spacing / 2.0;

	// Generate grid coordinates
	for (uint32_t i = 0; i < params.n_planets; ++i)
	{
		uint32_t row = i / grid_size;
		uint32_t col = i % grid_size;

		double x = offset_x + col * spacing;
		double y = offset_y + row * spacing;

		coords.push_back({x, y});
	}

	return coords;
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
