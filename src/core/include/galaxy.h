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
	uint32_t gal_size;   // Calculated galaxy size (radius) - determines boundaries
	uint32_t n_planets;  // Number of planets to generate (5-500)
	double density;      // Planet distribution density (0.0-1.0, TBD)
	GalaxyShape shape;   // Distribution pattern (random, spiral, circle, ring, cluster, grid)
	uint64_t seed;       // Random seed for generation
	
	// Constructor: calculates gal_size from size_param
	GalaxyGenerationParams(uint32_t size_param, uint32_t planets, double dens, GalaxyShape gal_shape, uint64_t rng_seed)
		: gal_size(static_cast<uint32_t>(std::sqrt(size_param) * 10.0)),
		  n_planets(planets),
		  density(dens),
		  shape(gal_shape),
		  seed(rng_seed)
	{
	}
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
	
	// Constructor to initialize galaxy boundaries and planets
	// Implementation in game.cpp
	Galaxy(const GalaxyGenerationParams& params, class DeterministicRNG* rng);
	
	// Initialize planets in the galaxy based on generation parameters
	// Requires access to RNG, so implementation is in game.cpp
	void initialize_planets(const GalaxyGenerationParams& params, class DeterministicRNG* rng);
	
	// Two separate RNG engines
	// (Implementation details in rng.h)
};

#endif // OPENHO_GALAXY_H
