#ifndef OPENHO_GALAXY_H
#define OPENHO_GALAXY_H

#include "planet.h"
#include "ship.h"
#include "player.h"
#include <cstdint>
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
	
	// Mutable ship list
	std::vector<Ship> ships;
	
	// Players
	std::vector<Player> players;
	
	// Current turn number
	uint32_t current_turn;
	
	// Two separate RNG engines
	// (Implementation details in rng.h)
};

#endif // OPENHO_GALAXY_H
