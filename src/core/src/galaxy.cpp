#include "galaxy.h"
#include "game.h"
#include "text_assets.h"
#include "utility.h"
#include "game_constants.h"
#include <cmath>
#include <set>

Galaxy::Galaxy(const GalaxyGenerationParams& params, GameState* game_state)
{
	// Staged galaxy generation:
	// Phase 1: Generate all planet coordinates
	std::vector<PlanetCoord> all_coords = generate_planet_coordinates(params, game_state);

	// Phase 2: Select home planet coordinates based on galaxy shape
	std::vector<PlanetCoord> home_coords;
	if (params.shape == GALAXY_CLUSTER)
	{
		// For cluster galaxies, angular offset is stored in params
		home_coords = select_home_planets_cluster(all_coords, params, game_state);
	}
	else if (params.shape == GALAXY_SPIRAL)
	{
		// For spiral galaxies, we need the spiral parameters
		// We'll need to pass them through - for now, use a workaround
		// TODO: Refactor to pass spiral parameters through params struct
		home_coords = select_home_planets_random(all_coords, params.n_players, game_state);
	}
	else
	{
		home_coords = select_home_planets_random(all_coords, params.n_players, game_state);
	}

	// Phase 3: Generate planet names
	const std::vector<std::string> planet_names = generate_planet_names(all_coords.size(), game_state);

	// Phase 4: Generate planet parameters
	generate_planet_parameters(all_coords, home_coords, planet_names, game_state);

	// Phase 5: Compute distance matrix
	compute_distance_matrix();

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
			home_planet_indices.push_back(i);
		}
	}
}

void Galaxy::compute_distance_matrix()
{
	size_t n = planets.size();
	distance_matrix.resize(n);

	for (size_t i = 0; i < n; ++i)
	{
		distance_matrix[i].resize(n);
		for (size_t j = 0; j < n; ++j)
		{
			const Planet& from = planets[i];
			const Planet& to = planets[j];

			// Calculate Euclidean distance
			double dx = to.x - from.x;
			double dy = to.y - from.y;
			double euclidean_dist = std::sqrt(dx * dx + dy * dy);

			// Round to nearest integer, store as double
			distance_matrix[i][j] = std::round(euclidean_dist);
		}
	}
}

double Galaxy::get_distance(uint32_t from_id, uint32_t to_id) const
{
	return distance_matrix.at(from_id).at(to_id);
}
