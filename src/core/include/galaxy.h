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
	
	// Distance matrix: [from_planet_id][to_planet_id] = distance
	// Computed once at initialization, never updated
	// Stores Euclidean distance rounded to nearest integer as double
	std::vector<std::vector<double>> distance_matrix;
	
	// Temporary member variable for cluster galaxy generation
	// Stores the random angular offset used to orient cluster arrangements
	// Only used during construction, cleared after home planets are selected
	double cluster_angular_offset = 0.0;
	
	// Constructor to initialize galaxy boundaries and planets
	// Takes GameState reference for access to RNG and TextAssets
	// Implementation in game.cpp
	Galaxy(const GalaxyGenerationParams& params, class GameState* game_state);
	
	// Compute distance matrix after all planets are created
	// Called from constructor after generate_planet_parameters()
	void compute_distance_matrix();
	
	// Get distance between two planets
	// Returns Euclidean distance rounded to nearest integer
	// Throws std::out_of_range if planet IDs are invalid
	double get_distance(uint32_t from_id, uint32_t to_id) const;
	
	// // Generate randomized planet names (helper method)
	// // Generates n_planets unique names in random order from available_names
	static std::vector<std::string> generate_planet_names(uint32_t n_planets, class GameState* game_state);
	
	// Staged galaxy generation methods
	// Phase 1: Generate all planet coordinates according to galaxy shape
	std::vector<PlanetCoord> generate_planet_coordinates(
		const GalaxyGenerationParams& params,
		class GameState* game_state);
	
	// Phase 2: Select home planet coordinates from all coordinates (random selection)
	std::vector<PlanetCoord> select_home_planets_random(
		const std::vector<PlanetCoord>& all_coords,
		uint32_t n_home_planets,
		class GameState* game_state);
	
	// Phase 2b: Select home planet coordinates for spiral galaxies
	std::vector<PlanetCoord> select_home_planets_spiral(
		const std::vector<PlanetCoord>& all_coords,
		uint32_t n_home_planets,
		double a,
		double delta_theta,
		class GameState* game_state);
	
	// Phase 2c: Select home planet coordinates for cluster galaxies
	// Divides galaxy into N equal wedges starting from angular_offset
	// Selects one random planet from each wedge as a home planet
	std::vector<PlanetCoord> select_home_planets_cluster(
		const std::vector<PlanetCoord>& all_coords,
		uint32_t n_home_planets,
		double angular_offset,
		class GameState* game_state);
	
	// Phase 3: Generate planet parameters for all coordinates
	void generate_planet_parameters(
		const std::vector<PlanetCoord>& all_coords,
		const std::vector<PlanetCoord>& home_coords,
		const std::vector<std::string>& planet_names,
		class GameState* game_state);
	
	// Shape-specific coordinate generation methods
	// Each method generates planet coordinates according to its shape pattern
	static std::vector<PlanetCoord> generate_coordinates_random(
		const GalaxyGenerationParams& params,
		class GameState* game_state);
	
	static std::vector<PlanetCoord> generate_coordinates_spiral(
		const GalaxyGenerationParams& params,
		class GameState* game_state);
	
	static std::vector<PlanetCoord> generate_coordinates_circle(
		const GalaxyGenerationParams& params,
		class GameState* game_state);
	
	static std::vector<PlanetCoord> generate_coordinates_ring(
		const GalaxyGenerationParams& params,
		class GameState* game_state);
	
	static std::vector<PlanetCoord> generate_coordinates_cluster(
		const GalaxyGenerationParams& params,
		class GameState* game_state,
		Galaxy* galaxy = nullptr);
	
	static std::vector<PlanetCoord> generate_coordinates_grid(
		const GalaxyGenerationParams& params,
		class GameState* game_state);
	
	// Overloaded versions that take RNG directly (for Python wrapper)
	static std::vector<PlanetCoord> generate_coordinates_random(
		const GalaxyGenerationParams& params,
		class DeterministicRNG& rng);
	
	static std::vector<PlanetCoord> generate_coordinates_spiral(
		const GalaxyGenerationParams& params,
		class DeterministicRNG& rng);
	
	static std::vector<PlanetCoord> generate_coordinates_circle(
		const GalaxyGenerationParams& params,
		class DeterministicRNG& rng);
	
	static std::vector<PlanetCoord> generate_coordinates_ring(
		const GalaxyGenerationParams& params,
		class DeterministicRNG& rng);
	
	static std::vector<PlanetCoord> generate_coordinates_cluster(
		const GalaxyGenerationParams& params,
		class DeterministicRNG& rng);
	
	static std::vector<PlanetCoord> generate_coordinates_grid(
		const GalaxyGenerationParams& params,
		class DeterministicRNG& rng);
	
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

// ============================================================================
// Galaxy Generation Failure Logging
// ============================================================================

/**
 * Log galaxy generation failure to a file.
 * Creates a detailed report of what was requested vs. what was generated.
 * 
 * @param params The galaxy generation parameters that were requested
 * @param generated_coords The coordinates that were successfully generated
 * @param log_filename Path to write the log file to
 */
void log_galaxy_generation_failure(
	const GalaxyGenerationParams& params,
	const std::vector<PlanetCoord>& generated_coords,
	const std::string& log_filename);

#endif // OPENHO_GALAXY_H
