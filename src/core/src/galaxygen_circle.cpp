#include "galaxy.h"
#include "game.h"
#include "game_constants.h"
#include "utility.h"
#include <cmath>

std::vector<PlanetCoord> Galaxy::generate_coordinates_circle(
	const GalaxyGenerationParams& params,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();

	// Calculate active area (same formula as random galaxy, but without the 0.85 reduction factor)
	// active_area = gal_size^2 where gal_size = sqrt(n_planets) * (5.0 + 6.4/density)
	double gal_size = std::sqrt(double(params.n_planets)) *
	                  (GameConstants::Galaxy_Size_Scale_Base + GameConstants::Galaxy_Size_Scale_Density / params.density);
	double active_area = gal_size * gal_size;

	// Calculate circle radius from active area: area = pi * r^2
	double radius = std::sqrt(active_area / M_PI);

	// Create circle region and generate coordinates using Poisson disk sampling
	CircleRegion circle(radius);
	return poisson_disk_sampling(circle, GameConstants::min_planet_distance, params.n_planets, rng);
}

std::vector<PlanetCoord> Galaxy::generate_coordinates_circle(
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

	// Use Poisson disk sampling to generate evenly distributed coordinates
	CircleRegion region(base_radius);
	std::vector<PlanetCoord> coords = poisson_disk_sampling(region, min_distance, params.n_planets, rng);

	return coords;
}
