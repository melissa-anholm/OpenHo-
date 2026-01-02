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
	
	// // Generate randomized planet names (helper method)
	// // Generates n_planets unique names in random order from available_names
	// static std::vector<std::string> generate_planet_names(
	// 	uint32_t n_planets,
	// 	const std::vector<std::string>& available_names,
	// 	class DeterministicRNG& rng);
	static std::vector<std::string> generate_planet_names(uint32_t n_planets, class GameState* game_state);
	
	
	// Shape-specific planet initialization methods
	// Each method generates planet positions according to its shape pattern
	// and creates Planet objects with the provided names
	void initialize_planets_random(
		const GalaxyGenerationParams& params,
		const std::vector<std::string>& planet_names,
		class GameState* game_state);
	
	void initialize_planets_spiral(
		const GalaxyGenerationParams& params,
		const std::vector<std::string>& planet_names,
		class GameState* game_state);
	
	void initialize_planets_circle(
		const GalaxyGenerationParams& params,
		const std::vector<std::string>& planet_names,
		class GameState* game_state);
	
	void initialize_planets_ring(
		const GalaxyGenerationParams& params,
		const std::vector<std::string>& planet_names,
		class GameState* game_state);
	
	void initialize_planets_cluster(
		const GalaxyGenerationParams& params,
		const std::vector<std::string>& planet_names,
		class GameState* game_state);
	
	void initialize_planets_grid(
		const GalaxyGenerationParams& params,
		const std::vector<std::string>& planet_names,
		class GameState* game_state);
	
	// Two separate RNG engines
	// (Implementation details in rng.h)
};

#endif // OPENHO_GALAXY_H
