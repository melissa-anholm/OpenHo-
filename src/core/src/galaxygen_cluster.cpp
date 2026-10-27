#include "galaxy.h"
#include "game.h"
#include "game_constants.h"
#include "utility.h"
#include <cmath>

std::vector<PlanetCoord> Galaxy::generate_coordinates_cluster(
	const GalaxyGenerationParams& params,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	std::vector<PlanetCoord> coords;

	// Phase 0: Generate random angular offset for cluster orientation
	// This ensures cluster galaxies don't all have the same orientation
	// Store in params for use by select_home_planets_cluster
	const_cast<GalaxyGenerationParams&>(params).cluster_angular_offset = rng.nextDouble() * 360.0;

	// Phase 1: Calculate cluster parameters
	uint32_t n_clusters = params.n_players;
	if (n_clusters < 1) n_clusters = 1;

	// Calculate overall galaxy size
	double gal_size = std::sqrt(double(params.n_planets)) *
	                  (GameConstants::Galaxy_Size_Scale_Base + GameConstants::Galaxy_Size_Scale_Density / params.density);

	// Calculate cluster radius and ring radius based on density
	// At high density (1.0): clusters can overlap slightly (~10%)
	// At low density (0.25): enough space for another cluster between them
	double cluster_radius = gal_size / (2.0 * std::sqrt(double(n_clusters)));

	// Ring radius: distance from center to cluster centers
	// Spacing between clusters on ring: 2π * ring_radius / n_clusters
	// We want spacing = 2 * cluster_radius * spacing_factor
	// spacing_factor ranges from 1.1 (high density, ~10% overlap) to 2.0 (low density, room for another)
	double spacing_factor = 1.1 + (1.0 - params.density) * 0.9;  // Range: [1.1, 2.0]
	double desired_spacing = 2.0 * cluster_radius * spacing_factor;
	double ring_radius = desired_spacing * n_clusters / (2.0 * M_PI);

	// Planets per cluster (roughly equal distribution)
	uint32_t planets_per_cluster = params.n_planets / n_clusters;
	uint32_t remaining_planets = params.n_planets % n_clusters;

	// Phase 2: Generate clusters
	CheckDistanceSpatialGrid grid(GameConstants::min_planet_distance, gal_size * 2.0);

	for (uint32_t cluster_idx = 0; cluster_idx < n_clusters; ++cluster_idx)
	{
		// Calculate cluster center position on ring
		double angle = (2.0 * M_PI * cluster_idx) / n_clusters;
		double cluster_center_x = ring_radius * std::cos(angle);
		double cluster_center_y = ring_radius * std::sin(angle);

		// Determine how many planets in this cluster
		uint32_t planets_in_this_cluster = planets_per_cluster;
		if (cluster_idx < remaining_planets)
		{
			planets_in_this_cluster++;
		}

		// Phase 3: Generate planets within this cluster
		uint32_t planets_placed = 0;
		uint32_t max_attempts = planets_in_this_cluster * 10;  // Generous attempt limit
		uint32_t attempts = 0;

		while (planets_placed < planets_in_this_cluster && attempts < max_attempts)
		{
			// Generate random position within cluster circle
			double angle_offset = rng.nextDouble() * 2.0 * M_PI;
			double radius_offset = rng.nextDouble() * cluster_radius;

			double x_coord = cluster_center_x + radius_offset * std::cos(angle_offset);
			double y_coord = cluster_center_y + radius_offset * std::sin(angle_offset);

			// Check if position is valid
			if (grid.is_position_valid(x_coord, y_coord, GameConstants::min_planet_distance))
			{
				coords.push_back({x_coord, y_coord});
				grid.add_planet(x_coord, y_coord, static_cast<uint32_t>(coords.size()));
				planets_placed++;
			}
			attempts++;
		}
	}

	return coords;
}

std::vector<PlanetCoord> Galaxy::generate_coordinates_cluster(
	const GalaxyGenerationParams& params,
	DeterministicRNG& rng)
{
	std::vector<PlanetCoord> coords;

	// Phase 0: Generate random angular offset for cluster orientation
	const_cast<GalaxyGenerationParams&>(params).cluster_angular_offset = rng.nextDouble() * 360.0;

	// Phase 1: Calculate cluster parameters
	uint32_t n_clusters = params.n_players;
	if (n_clusters < 1) n_clusters = 1;

	// Calculate overall galaxy size
	double gal_size = std::sqrt(double(params.n_planets)) *
	                  (GameConstants::Galaxy_Size_Scale_Base + GameConstants::Galaxy_Size_Scale_Density / params.density);

	// Calculate cluster radius and ring radius based on density
	double cluster_radius = gal_size / (2.0 * std::sqrt(double(n_clusters)));
	double spacing_factor = 1.1 + (1.0 - params.density) * 0.9;
	double desired_spacing = 2.0 * cluster_radius * spacing_factor;
	double ring_radius = desired_spacing * n_clusters / (2.0 * M_PI);

	// Planets per cluster (roughly equal distribution)
	uint32_t planets_per_cluster = params.n_planets / n_clusters;
	uint32_t remaining_planets = params.n_planets % n_clusters;

	// Phase 2: Generate clusters
	CheckDistanceSpatialGrid grid(GameConstants::min_planet_distance, gal_size * 2.0);

	for (uint32_t cluster_idx = 0; cluster_idx < n_clusters; ++cluster_idx)
	{
		// Calculate cluster center position on ring
		double angle = (2.0 * M_PI * cluster_idx) / n_clusters;
		double cluster_center_x = ring_radius * std::cos(angle);
		double cluster_center_y = ring_radius * std::sin(angle);

		// Determine how many planets in this cluster
		uint32_t planets_in_this_cluster = planets_per_cluster;
		if (cluster_idx < remaining_planets)
		{
			planets_in_this_cluster++;
		}

		// Phase 3: Generate planets within this cluster
		uint32_t planets_placed = 0;
		uint32_t max_attempts = planets_in_this_cluster * 10;
		uint32_t attempts = 0;

		while (planets_placed < planets_in_this_cluster && attempts < max_attempts)
		{
			// Generate random position within cluster circle
			double angle_offset = rng.nextDouble() * 2.0 * M_PI;
			double radius_offset = rng.nextDouble() * cluster_radius;

			double x_coord = cluster_center_x + radius_offset * std::cos(angle_offset);
			double y_coord = cluster_center_y + radius_offset * std::sin(angle_offset);

			// Check if position is valid
			if (grid.is_position_valid(x_coord, y_coord, GameConstants::min_planet_distance))
			{
				coords.push_back({x_coord, y_coord});
				grid.add_planet(x_coord, y_coord, static_cast<uint32_t>(coords.size()));
				planets_placed++;
			}
			attempts++;
		}
	}

	return coords;
}
