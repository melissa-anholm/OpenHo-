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
	uint32_t n_planets;  // Number of planets to generate (5-500)
	uint32_t n_players;  // Number of players (determines home planet count)
	double density;      // Planet distribution density (0.0-1.0, TBD)
	GalaxyShape shape;   // Distribution pattern (random, spiral, circle, ring, cluster, grid)
	uint64_t seed;       // Random seed for generation
	
	// Default constructor for testing
	GalaxyGenerationParams()
	{
		seed = 0;
		
		shape = GALAXY_RANDOM;
		n_planets = 100;
		n_players = 1;
		density = 0.5;
	}
	
	// Constructor
	GalaxyGenerationParams(uint32_t planets, uint32_t players, double dens, GalaxyShape gal_shape, uint64_t rng_seed)
	{
		seed = rng_seed;
		
		shape = gal_shape;
		n_planets = planets;
		n_players = players;
		density = dens;
	}
};

// Coordinate pair for planet positions
using PlanetCoord = std::pair<double, double>;  // (x, y)

// Galaxy structure
struct Galaxy
{
	GalaxyCoord gal_size;
	
	// Immutable planet list
	std::vector<Planet> planets;
	
	// Home planet indices (indices into planets vector)
	std::vector<size_t> home_planet_indices;
	
	// Constructor to initialize galaxy boundaries and planets
	// Takes GameState reference for access to RNG and TextAssets
	// Implementation in game.cpp
	Galaxy(const GalaxyGenerationParams& params, class GameState* game_state);
	
	// // Generate randomized planet names (helper method)
	// // Generates n_planets unique names in random order from available_names
	static std::vector<std::string> generate_planet_names(uint32_t n_planets, class GameState* game_state);
	
	// Staged galaxy generation methods
	// Phase 1: Generate all planet coordinates according to galaxy shape
	std::vector<PlanetCoord> generate_planet_coordinates(
		const GalaxyGenerationParams& params,
		class GameState* game_state);
	
	// Phase 2: Select home planet coordinates from all coordinates
	std::vector<PlanetCoord> select_home_planet_coordinates(
		const std::vector<PlanetCoord>& all_coords,
		uint32_t n_home_planets,
		class GameState* game_state);
	
	// Phase 3: Generate planet parameters for all coordinates
	void generate_planet_parameters(
		const std::vector<PlanetCoord>& all_coords,
		const std::vector<PlanetCoord>& home_coords,
		const std::vector<std::string>& planet_names,
		class GameState* game_state);
	
	// Shape-specific coordinate generation methods
	// Each method generates planet coordinates according to its shape pattern
	std::vector<PlanetCoord> generate_coordinates_random(
		const GalaxyGenerationParams& params,
		class GameState* game_state);
	
	std::vector<PlanetCoord> generate_coordinates_spiral(
		const GalaxyGenerationParams& params,
		class GameState* game_state);
	
	std::vector<PlanetCoord> generate_coordinates_circle(
		const GalaxyGenerationParams& params,
		class GameState* game_state);
	
	std::vector<PlanetCoord> generate_coordinates_ring(
		const GalaxyGenerationParams& params,
		class GameState* game_state);
	
	std::vector<PlanetCoord> generate_coordinates_cluster(
		const GalaxyGenerationParams& params,
		class GameState* game_state);
	
	std::vector<PlanetCoord> generate_coordinates_grid(
		const GalaxyGenerationParams& params,
		class GameState* game_state);
	
	// Legacy planet initialization methods (deprecated, kept for reference)
	void initialize_planets_random(
		const GalaxyGenerationParams& params,
		const std::vector<std::string>& planet_names,
		class GameState* game_state);
	
	void initialize_planets_grid(
		const GalaxyGenerationParams& params,
		const std::vector<std::string>& planet_names,
		class GameState* game_state);
};

#endif // OPENHO_GALAXY_H
