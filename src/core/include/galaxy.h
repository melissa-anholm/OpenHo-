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
//	uint32_t gal_size;   // Calculated galaxy size (radius) - determines boundaries
	uint32_t n_planets;  // Number of planets to generate (5-500)
	double density;      // Planet distribution density (0.0-1.0, TBD)
	GalaxyShape shape;   // Distribution pattern (random, spiral, circle, ring, cluster, grid)
	uint64_t seed;       // Random seed for generation
	
	// Default constructor for testing
	GalaxyGenerationParams()
	{
		seed = 0;
		
		shape = GALAXY_RANDOM;
		n_planets = 100;
		density = 0.5;
	//	gal_size = 100;
	}
	
	// Constructor
	GalaxyGenerationParams(uint32_t planets, double dens, GalaxyShape gal_shape, uint64_t rng_seed)
	{
		seed = rng_seed;
		
		shape = gal_shape;
		n_planets = planets;
		density = dens;
	}
};

// Galaxy structure
struct Galaxy
{
	// Boundaries
	// GalaxyCoord min_x;
	// GalaxyCoord max_x;
	// GalaxyCoord min_y;
	// GalaxyCoord max_y;
	GalaxyCoord gal_size;
	
	// Immutable planet list
	std::vector<Planet> planets;
	
	// Constructor to initialize galaxy boundaries and planets
	// Takes GameState reference for access to RNG and TextAssets
	// Implementation in game.cpp
	Galaxy(const GalaxyGenerationParams& params, class GameState* game_state);
	
	// Initialize planets in the galaxy based on generation parameters
	// Requires access to RNG and TextAssets, so implementation is in game.cpp
	void initialize_planets(const GalaxyGenerationParams& params, class GameState* game_state);
	
	// Two separate RNG engines
	// (Implementation details in rng.h)
};

#endif // OPENHO_GALAXY_H
