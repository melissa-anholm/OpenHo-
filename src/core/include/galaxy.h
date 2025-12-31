#ifndef OPENHO_GALAXY_H
#define OPENHO_GALAXY_H

#include "planet.h"
#include "player.h"
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
	uint32_t size;      // Number of planets (5-500)
	double density;     // Planet distribution density (TBD)
	std::string shape;  // "random", "spiral", "circle", "ring", "cluster", "grid"
	uint64_t seed;      // Random seed for generation
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
		// Calculate boundaries based on size
		double radius = std::sqrt(params.size) * 10.0;
		min_x = -radius;
		max_x = radius;
		min_y = -radius;
		max_y = radius;
		
		current_turn = 0;
		// planets and players vectors are default-constructed (empty)
	}
	
	// Two separate RNG engines
	// (Implementation details in rng.h)
};

#endif // OPENHO_GALAXY_H
