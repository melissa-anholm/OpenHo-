#include "galaxy.h"
#include "game.h"
#include "game_constants.h"
#include "utility.h"
#include <cmath>

std::vector<PlanetCoord> Galaxy::generate_coordinates_spiral(
	const GalaxyGenerationParams& params,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();

	// Phase 1: Choose randomized parameters
	double delta_theta = M_PI / 4.0 + rng.nextDouble() * (M_PI - M_PI / 4.0);
	double a = 100.0 / std::sqrt(delta_theta);
	double ratio = 2.0 + rng.nextDouble() * (6.0 - 2.0);

	// Phase 2: Calculate overall size
	double gal_size = std::sqrt(double(params.n_planets)) *
	                  (GameConstants::Galaxy_Size_Scale_Base + GameConstants::Galaxy_Size_Scale_Density / params.density);
	double active_area = gal_size * gal_size;
	double core_radius = std::sqrt(active_area / M_PI);

	// Refine core_radius iteratively
	for (int iter = 0; iter < 10; ++iter) {
		double overlap = std::max(6.0, 0.15 * core_radius);
		double theta_core = (core_radius / a) * (core_radius / a);
		double theta_outer = delta_theta;
		double arc_length = fermat_spiral_arc_length(a, theta_core, theta_outer);
		double core_planets = (core_radius / GameConstants::min_planet_distance) *
		                      (core_radius / GameConstants::min_planet_distance);
		double planets_per_arm = arc_length / GameConstants::min_planet_distance;
		double total_planets = core_planets + (params.n_players * planets_per_arm);

		if (total_planets < params.n_planets * 0.95) {
			core_radius *= 1.05;
		} else if (total_planets > params.n_planets * 1.05) {
			core_radius *= 0.95;
		} else {
			break;
		}
	}

	// Calculate final dimensions
	double overlap = std::max(6.0, 0.15 * core_radius);
	double inner_arm_radius = core_radius - overlap;
	double outer_arm_radius = core_radius * ratio;
	double theta_core = (core_radius / a) * (core_radius / a);
	double theta_outer = delta_theta;

	// Phase 3: Generate spiral arms
	std::vector<PlanetCoord> all_coords;
	double arm_angle_step = 2.0 * M_PI / params.n_players;

	for (uint32_t arm_idx = 0; arm_idx < params.n_players; ++arm_idx) {
		double arm_angle = arm_idx * arm_angle_step;
		std::vector<PlanetCoord> arm_candidates;
		double angular_step = 0.1;

		for (double theta = theta_core; theta <= theta_outer; theta += angular_step) {
			PlanetCoord center = fermat_spiral_point(a, theta, arm_angle);
			double band_thickness = 4.0;
			for (double offset = -band_thickness / 2.0; offset <= band_thickness / 2.0; offset += 1.0) {
				double angle_perp = arm_angle + theta + M_PI / 2.0;
				PlanetCoord offset_point = {
					center.first + offset * std::cos(angle_perp),
					center.second + offset * std::sin(angle_perp)
				};
				arm_candidates.push_back(offset_point);
			}
		}

		for (const auto& candidate : arm_candidates) {
			bool valid = true;
			for (const auto& existing : all_coords) {
				double dx = candidate.first - existing.first;
				double dy = candidate.second - existing.second;
				double dist = std::sqrt(dx * dx + dy * dy);
				if (dist < GameConstants::min_planet_distance) {
					valid = false;
					break;
				}
			}
			if (valid) {
				all_coords.push_back(candidate);
			}
		}
	}

	// Phase 4: Generate central core
	double seed_radius = inner_arm_radius - GameConstants::min_planet_distance;
	if (seed_radius > 0) {
		CircleRegion core_region(core_radius);
		std::vector<PlanetCoord> core_coords = poisson_disk_sampling(
			core_region,
			GameConstants::min_planet_distance,
			params.n_planets,
			rng,
			all_coords);
		all_coords.insert(all_coords.end(), core_coords.begin(), core_coords.end());
	}

	return all_coords;
}

std::vector<PlanetCoord> Galaxy::generate_coordinates_spiral(
	const GalaxyGenerationParams& params,
	DeterministicRNG& rng)
{
	// Phase 1: Choose randomized parameters
	double delta_theta = M_PI / 4.0 + rng.nextDouble() * (M_PI - M_PI / 4.0);
	double a = 100.0 / std::sqrt(delta_theta);
	double ratio = 2.0 + rng.nextDouble() * (6.0 - 2.0);

	// Phase 2: Calculate overall size
	double gal_size = std::sqrt(double(params.n_planets)) *
	                  (GameConstants::Galaxy_Size_Scale_Base + GameConstants::Galaxy_Size_Scale_Density / params.density);
	double active_area = gal_size * gal_size;
	double core_radius = std::sqrt(active_area / M_PI);

	// Refine core_radius iteratively
	for (int iter = 0; iter < 10; ++iter) {
		double overlap = std::max(6.0, 0.15 * core_radius);
		double theta_core = (core_radius / a) * (core_radius / a);
		double theta_outer = delta_theta;
		double arc_length = fermat_spiral_arc_length(a, theta_core, theta_outer);
		double core_planets = (core_radius / GameConstants::min_planet_distance) *
		                      (core_radius / GameConstants::min_planet_distance);
		double planets_per_arm = arc_length / GameConstants::min_planet_distance;
		double total_planets = core_planets + (params.n_players * planets_per_arm);

		if (total_planets < params.n_planets * 0.95) {
			core_radius *= 1.05;
		} else if (total_planets > params.n_planets * 1.05) {
			core_radius *= 0.95;
		} else {
			break;
		}
	}

	// Calculate final dimensions
	double overlap = std::max(6.0, 0.15 * core_radius);
	double inner_arm_radius = core_radius - overlap;
	double outer_arm_radius = core_radius * ratio;
	double theta_core = (core_radius / a) * (core_radius / a);
	double theta_outer = delta_theta;

	// Phase 3: Generate spiral arms
	std::vector<PlanetCoord> all_coords;
	double arm_angle_step = 2.0 * M_PI / params.n_players;

	for (uint32_t arm_idx = 0; arm_idx < params.n_players; ++arm_idx) {
		double arm_angle = arm_idx * arm_angle_step;
		std::vector<PlanetCoord> arm_candidates;
		double angular_step = 0.1;

		for (double theta = theta_core; theta <= theta_outer; theta += angular_step) {
			PlanetCoord center = fermat_spiral_point(a, theta, arm_angle);
			double band_thickness = 4.0;
			for (double offset = -band_thickness / 2.0; offset <= band_thickness / 2.0; offset += 1.0) {
				double angle_perp = arm_angle + theta + M_PI / 2.0;
				PlanetCoord offset_point = {
					center.first + offset * std::cos(angle_perp),
					center.second + offset * std::sin(angle_perp)
				};
				arm_candidates.push_back(offset_point);
			}
		}

		for (const auto& candidate : arm_candidates) {
			bool valid = true;
			for (const auto& existing : all_coords) {
				double dx = candidate.first - existing.first;
				double dy = candidate.second - existing.second;
				double dist = std::sqrt(dx * dx + dy * dy);
				if (dist < GameConstants::min_planet_distance) {
					valid = false;
					break;
				}
			}
			if (valid) {
				all_coords.push_back(candidate);
			}
		}
	}

	// Phase 4: Generate central core
	double seed_radius = inner_arm_radius - GameConstants::min_planet_distance;
	if (seed_radius > 0) {
		CircleRegion core_region(core_radius);
		std::vector<PlanetCoord> core_coords = poisson_disk_sampling(
			core_region,
			GameConstants::min_planet_distance,
			params.n_planets,
			rng,
			all_coords);
		all_coords.insert(all_coords.end(), core_coords.begin(), core_coords.end());
	}

	return all_coords;
}
