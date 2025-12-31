#ifndef OPENHO_GALAXY_H
#define OPENHO_GALAXY_H

#include "planet.h"
#include "player.h"
#include "enums.h"
#include <cstdint>
#include <cmath>
#include <vector>
#include <string>

// ============================================================================
// Data Structures
// ============================================================================

// Galaxy generation parameters
struct GalaxyGenerationParams
{
	uint32_t size;       // Galaxy size parameter (determines boundaries)
	uint32_t n_planets;  // Number of planets to generate (5-500)
	double density;      // Planet distribution density (0.0-1.0, TBD)
	GalaxyShape shape;   // Distribution pattern (random, spiral, circle, ring, cluster, grid)
	uint64_t seed;       // Random seed for generation
};

// Galaxy structure
struct Galaxy
{
	// Boundaries
	GalaxyCoord min_x;
	GalaxyCoord max_x;
	GalaxyCoord min_y;
	GalaxyCoord max_y;
	
	// Immutable planet list
	std::vector<Planet> planets;
	
	// Current turn number
	uint32_t current_turn;
	
	// Constructor to initialize galaxy boundaries based on generation parameters
	Galaxy(const GalaxyGenerationParams& params)
	{
		// Calculate boundaries based on galaxy size
		double radius = std::sqrt(params.size) * 10.0;
		min_x = -radius;
		max_x = radius;
		min_y = -radius;
		max_y = radius;
		
		current_turn = 0;
		// planets and players vectors are default-constructed (empty)
	}
	
	// Initialize planets in the galaxy based on generation parameters
	// Requires access to RNG, so implementation is in game.cpp
	void initialize_planets(const GalaxyGenerationParams& params, class DeterministicRNG* rng);
	
	// Two separate RNG engines
	// (Implementation details in rng.h)
};

#endif // OPENHO_GALAXY_H
