#include "galaxy.h"
#include "game.h"
#include "utility.h"
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

std::vector<PlanetCoord> Galaxy::select_home_planets_random(
	const std::vector<PlanetCoord>& all_coords,
	uint32_t n_home_planets,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();

	// Validate that we have enough coordinates for home planets
	if (all_coords.size() < n_home_planets)
	{
		throw std::runtime_error("Insufficient planet coordinates (" + std::to_string(all_coords.size()) +
		                         ") for " + std::to_string(n_home_planets) + " home planets.");
	}

	// Create a mutable copy for shuffling
	std::vector<PlanetCoord> shuffled = all_coords;

	// Fisher-Yates shuffle
	for (size_t i = shuffled.size() - 1; i > 0; --i)
	{
		size_t j = rng.nextInt32Range(0, i);
		std::swap(shuffled[i], shuffled[j]);
	}

	// Take first n_home_planets as home planets
	std::vector<PlanetCoord> home_coords(shuffled.begin(), shuffled.begin() + n_home_planets);
	return home_coords;
}

std::vector<PlanetCoord> Galaxy::select_home_planets_spiral(
	const std::vector<PlanetCoord>& all_coords,
	uint32_t n_home_planets,
	double a,
	double delta_theta,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();

	// Validate that we have enough coordinates for home planets
	if (all_coords.size() < n_home_planets)
	{
		throw std::runtime_error("Insufficient planet coordinates (" + std::to_string(all_coords.size()) +
		                         ") for " + std::to_string(n_home_planets) + " home planets.");
	}

	std::vector<PlanetCoord> home_coords;

	// For each spiral arm, find the planet closest to the end of the arm
	double arm_angle_step = 2.0 * M_PI / n_home_planets;

	for (uint32_t arm_idx = 0; arm_idx < n_home_planets; ++arm_idx)
	{
		double arm_angle = arm_idx * arm_angle_step;

		// The end of the arm is at theta = delta_theta
		PlanetCoord arm_end = fermat_spiral_point(a, delta_theta, arm_angle);

		// Find the closest planet to this arm end
		double min_distance = std::numeric_limits<double>::max();
		PlanetCoord closest_planet = all_coords[0];

		for (const auto& coord : all_coords)
		{
			double dx = coord.first - arm_end.first;
			double dy = coord.second - arm_end.second;
			double dist = std::sqrt(dx * dx + dy * dy);

			if (dist < min_distance)
			{
				min_distance = dist;
				closest_planet = coord;
			}
		}

		home_coords.push_back(closest_planet);
	}

	return home_coords;
}

std::vector<PlanetCoord> Galaxy::select_home_planets_cluster(
	const std::vector<PlanetCoord>& all_coords,
	const GalaxyGenerationParams& params,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	uint32_t n_home_planets = params.n_players;
	double angular_offset = params.cluster_angular_offset;

	// Validate that we have enough coordinates for home planets
	if (all_coords.size() < n_home_planets)
	{
		throw std::runtime_error("Insufficient planet coordinates (" + std::to_string(all_coords.size()) +
		                         ") for " + std::to_string(n_home_planets) + " home planets.");
	}

	std::vector<PlanetCoord> home_coords;

	// Convert angular offset from degrees to radians
	double offset_rad = angular_offset * M_PI / 180.0;

	// Wedge angle in radians
	double wedge_angle = 2.0 * M_PI / n_home_planets;

	// For each wedge, find planets within it and select one at random
	for (uint32_t wedge_idx = 0; wedge_idx < n_home_planets; ++wedge_idx)
	{
		// Calculate wedge boundaries
		double wedge_start = offset_rad + wedge_idx * wedge_angle;
		double wedge_end = wedge_start + wedge_angle;

		// Find all planets within this wedge
		std::vector<PlanetCoord> planets_in_wedge;
		for (const auto& coord : all_coords)
		{
			double angle = std::atan2(coord.second, coord.first);

			// Normalize angle to [0, 2π)
			if (angle < 0) angle += 2.0 * M_PI;

			// Check if angle is within wedge (handling wraparound)
			bool in_wedge = false;
			if (wedge_end <= 2.0 * M_PI)
			{
				// Normal case: wedge doesn't wrap around
				in_wedge = (angle >= wedge_start && angle < wedge_end);
			}
			else
			{
				// Wedge wraps around 0 radians
				double wrapped_end = wedge_end - 2.0 * M_PI;
				in_wedge = (angle >= wedge_start || angle < wrapped_end);
			}

			if (in_wedge)
			{
				planets_in_wedge.push_back(coord);
			}
		}

		// Select one random planet from this wedge
		if (!planets_in_wedge.empty())
		{
			uint32_t random_idx = rng.nextInt32Range(0, planets_in_wedge.size() - 1);
			home_coords.push_back(planets_in_wedge[random_idx]);
		}
	}

	return home_coords;
}
