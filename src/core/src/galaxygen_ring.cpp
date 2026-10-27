#include "galaxy.h"
#include "game.h"
#include "game_constants.h"
#include "utility.h"
#include <cmath>

std::vector<PlanetCoord> Galaxy::generate_coordinates_ring(
	const GalaxyGenerationParams& params,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();

	// Calculate active area (same formula as random galaxy, but without the 0.85 reduction factor)
	double gal_size = std::sqrt(double(params.n_planets)) *
	                  (GameConstants::Galaxy_Size_Scale_Base + GameConstants::Galaxy_Size_Scale_Density / params.density);
	double active_area = gal_size * gal_size;

	// Calculate outer radius range based on inner/outer ratio constraints
	// If R_inner = 0.5 * R_outer: active_area = pi * (R_outer^2 - 0.25*R_outer^2) = 0.75*pi*R_outer^2
	// If R_inner = 0.8 * R_outer: active_area = pi * (R_outer^2 - 0.64*R_outer^2) = 0.36*pi*R_outer^2
	double r_outer_min = std::sqrt(active_area / (0.75 * M_PI));
	double r_outer_max = std::sqrt(active_area / (0.36 * M_PI));

	// Randomly select outer radius within calculated range
	double r_outer = r_outer_min + rng.nextDouble() * (r_outer_max - r_outer_min);

	// Calculate inner radius to achieve correct active area
	// active_area = pi * (r_outer^2 - r_inner^2)
	// r_inner = sqrt(r_outer^2 - active_area/pi)
	double r_inner = std::sqrt(r_outer * r_outer - active_area / M_PI);

	// Create ring region and generate coordinates using Poisson disk sampling
	RingRegion ring(r_inner, r_outer);
	return poisson_disk_sampling(ring, GameConstants::min_planet_distance, params.n_planets, rng);
}

std::vector<PlanetCoord> Galaxy::generate_coordinates_ring(
	const GalaxyGenerationParams& params,
	DeterministicRNG& rng)
{
	// Calculate galaxy radius based on planet count, density, and Poisson disk packing
	// Poisson disk sampling with min_distance d has packing efficiency ~0.65-0.7
	// Each point occupies approximately π * d² area
	// density parameter: 1.0 = tight packing, lower values = more spread out
	double min_distance = GameConstants::min_planet_distance;
	// Poisson disk sampling fills the region with min_distance spacing
	// Factor of 1.5 empirically accounts for packing efficiency
	// Note: density parameter doesn't apply to Poisson disk sampling (it always fills completely)
	double base_area_per_planet = 1.5 * min_distance * min_distance;
	double total_area = params.n_planets * base_area_per_planet;
	double base_radius = std::sqrt(total_area / M_PI);

	// Use Poisson disk sampling to generate evenly distributed coordinates in a ring
	// Ring area = π(R_outer² - R_inner²). With R_inner = 0.6*R_outer, ring area = 0.64*π*R_outer²
	// To achieve target area, scale up: R_outer = base_radius / sqrt(0.64) = base_radius * 1.25
	double outer_radius = base_radius * 1.25;
	double inner_radius = outer_radius * 0.6;
	RingRegion region(inner_radius, outer_radius);
	std::vector<PlanetCoord> coords = poisson_disk_sampling(region, min_distance, params.n_planets, rng);

	return coords;
}
