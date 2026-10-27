#include "galaxy.h"

std::vector<PlanetCoord> Galaxy::generate_planet_coordinates(
	const GalaxyGenerationParams& params,
	GameState* game_state)
{
	// Dispatch to shape-specific coordinate generation
	switch (params.shape)
	{
		case GALAXY_RANDOM:
			return Galaxy::generate_coordinates_random(params, game_state);
		case GALAXY_SPIRAL:
			return Galaxy::generate_coordinates_spiral(params, game_state);
		case GALAXY_CIRCLE:
			return Galaxy::generate_coordinates_circle(params, game_state);
		case GALAXY_RING:
			return Galaxy::generate_coordinates_ring(params, game_state);
		case GALAXY_CLUSTER:
			return Galaxy::generate_coordinates_cluster(params, game_state);
		case GALAXY_GRID:
			return Galaxy::generate_coordinates_grid(params, game_state);
		default:
			return std::vector<PlanetCoord>();
	}
}
