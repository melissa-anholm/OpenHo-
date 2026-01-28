#include "galaxy.h"
#include "game.h"
#include "text_assets.h"
#include "utility.h"
#include "game_constants.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include <set>
#include <limits>

Galaxy::Galaxy(const GalaxyGenerationParams& params, GameState* game_state)
{
	// Staged galaxy generation:
	// Phase 1: Generate all planet coordinates
	std::vector<PlanetCoord> all_coords = generate_planet_coordinates(params, game_state);
	
	// Phase 2: Select home planet coordinates based on galaxy shape
	std::vector<PlanetCoord> home_coords;
	if (params.shape == GALAXY_SPIRAL)
	{
		// For spiral galaxies, we need the spiral parameters
		// We'll need to pass them through - for now, use a workaround
		// TODO: Refactor to pass spiral parameters through params struct
		home_coords = select_home_planets_random(all_coords, params.n_players, game_state);
	}
	else
	{
		home_coords = select_home_planets_random(all_coords, params.n_players, game_state);
	}
	
	// Phase 3: Generate planet names
	const std::vector<std::string> planet_names = generate_planet_names(all_coords.size(), game_state);
	
	// Phase 4: Generate planet parameters
	generate_planet_parameters(all_coords, home_coords, planet_names, game_state);
	
	// Phase 5: Compute distance matrix
	compute_distance_matrix();
	
	// Calculate galaxy size from coordinates
	if (!all_coords.empty())
	{
		GalaxyCoord max_dist = 0;
		for (const auto& coord : all_coords)
		{
			GalaxyCoord dist = std::sqrt(coord.first * coord.first + coord.second * coord.second);
			if (dist > max_dist) max_dist = dist;
		}
		gal_size = max_dist * 2.0;
	}
	else
	{
		gal_size = 100.0;
	}
}

std::vector<std::string> Galaxy::generate_planet_names(
	uint32_t n_planets,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	
	return generate_randomized_subset(game_state->text_assets->get_planet_name_list(), n_planets, rng);
}

// ============================================================================
// Staged Galaxy Generation Methods
// ============================================================================
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

std::vector<PlanetCoord> Galaxy::select_home_planets_random(
	const std::vector<PlanetCoord>& all_coords,
	uint32_t n_home_planets,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	
	// Validate that we have enough coordinates for home planets
	if (all_coords.size() < n_home_planets)
	{
		throw std::runtime_error("Insufficient planet coordinates (" + std::to_string(all_coords.size()) + 
		                         ") for " + std::to_string(n_home_planets) + " home planets.");
	}
	
	// Create a mutable copy for shuffling
	std::vector<PlanetCoord> shuffled = all_coords;
	
	// Fisher-Yates shuffle
	for (size_t i = shuffled.size() - 1; i > 0; --i)
	{
		size_t j = rng.nextInt32Range(0, i);
		std::swap(shuffled[i], shuffled[j]);
	}
	
	// Take first n_home_planets as home planets
	std::vector<PlanetCoord> home_coords(shuffled.begin(), shuffled.begin() + n_home_planets);
	return home_coords;
}

void Galaxy::generate_planet_parameters(
	const std::vector<PlanetCoord>& all_coords,
	const std::vector<PlanetCoord>& home_coords,
	const std::vector<std::string>& planet_names,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	
	// Create a set of home coordinates for quick lookup
	std::set<PlanetCoord> home_coord_set(home_coords.begin(), home_coords.end());
	
	// Generate planets for all coordinates
	uint32_t planet_id = 1;
	for (size_t i = 0; i < all_coords.size() && i < planet_names.size(); ++i)
	{
		const auto& coord = all_coords[i];
		bool is_home_planet = home_coord_set.count(coord) > 0;
		
		// Generate parameters
		double true_gravity;
		double true_temperature;
		
		if (is_home_planet)
		{
			// Home planets have constrained gravity
			true_gravity = GameConstants::Starting_Planet_Min_Gravity + 
						  rng.nextDouble() * (GameConstants::Starting_Planet_Max_Gravity - GameConstants::Starting_Planet_Min_Gravity);
			true_temperature = GameConstants::min_temp + rng.nextDouble() * (GameConstants::max_temp - GameConstants::min_temp);
		}
		else
		{
			// Regular planets have full range
			true_gravity = GameConstants::min_gravity + rng.nextDouble() * (GameConstants::max_gravity - GameConstants::min_gravity);
			true_temperature = GameConstants::min_temp + rng.nextDouble() * (GameConstants::max_temp - GameConstants::min_temp);
		}
		
		int32_t metal = rng.nextInt32Range(GameConstants::min_metal, GameConstants::max_metal);
		
		// Create planet
		Planet planet(planet_id++, planet_names[i], coord.first, coord.second, true_gravity, true_temperature, metal);
		planets.push_back(planet);
		
		// Track home planet indices
		if (is_home_planet)
		{
			home_planet_indices.push_back(i);
		}
	}
}

// ============================================================================
// Shape-Specific Coordinate Generation Methods
// ============================================================================
std::vector<PlanetCoord> Galaxy::generate_coordinates_random(
	const GalaxyGenerationParams& params,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	std::vector<PlanetCoord> coords;
	
	// Calculate galaxy size using density-aware formula with reduction factor
	GalaxyCoord gal_size = std::sqrt(double(params.n_planets)) * 
	           (GameConstants::Galaxy_Size_Scale_Base + GameConstants::Galaxy_Size_Scale_Density / params.density) *
	           (1.0 - GameConstants::Galaxy_Size_Reduction_Factor);
	
	// Initialize spatial grid for distance checking
	CheckDistanceSpatialGrid grid(GameConstants::min_planet_distance, gal_size * 2.0);
	
	// Galaxy is centered at (0, 0)
	GalaxyCoord half_size = gal_size / 2.0;
	GalaxyCoord min_x = -half_size;
	GalaxyCoord max_x = half_size;
	GalaxyCoord min_y = -half_size;
	GalaxyCoord max_y = half_size;
	
	uint32_t attempts_per_expansion = static_cast<uint32_t>(std::floor(std::sqrt(double(params.n_planets)) / 2.0));
	uint32_t max_attempts_per_planet = params.n_planets;
	
	for (uint32_t i = 0; i < params.n_planets; ++i)
	{
		uint32_t attempt = 0;
		bool placed = false;
		
		while (attempt < max_attempts_per_planet && !placed)
		{
			// Check if we need to expand the galaxy boundaries
			if (attempt > 0 && attempt % attempts_per_expansion == 0)
			{
				// Expand boundaries by 5% around center (0, 0)
				GalaxyCoord expansion_factor = 1.0 + GameConstants::Galaxy_Expansion_Factor;
				min_x *= expansion_factor;
				max_x *= expansion_factor;
				min_y *= expansion_factor;
				max_y *= expansion_factor;
			}
			// Generate random position within current galaxy bounds
			GalaxyCoord x_coord = min_x + rng.nextDouble() * (max_x - min_x);
			GalaxyCoord y_coord = min_y + rng.nextDouble() * (max_y - min_y);
			
			// Check if position is valid (far enough from other planets)
			if (grid.is_position_valid(x_coord, y_coord, GameConstants::min_planet_distance))
			{
				coords.push_back({x_coord, y_coord});
				grid.add_planet(x_coord, y_coord, i + 1);
				placed = true;
			}
			attempt++;
		}
	}
	
	return coords;
}

std::vector<PlanetCoord> Galaxy::generate_coordinates_spiral(
	const GalaxyGenerationParams& params,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	
	// Phase 1: Choose randomized parameters
	double delta_theta = M_PI / 4.0 + rng.nextDouble() * (M_PI - M_PI / 4.0);
	double a = 100.0 / std::sqrt(delta_theta);
	double ratio = 2.0 + rng.nextDouble() * (6.0 - 2.0);
	
	// Phase 2: Calculate overall size
	double gal_size = std::sqrt(double(params.n_planets)) * 
	                  (GameConstants::Galaxy_Size_Scale_Base + GameConstants::Galaxy_Size_Scale_Density / params.density);
	double active_area = gal_size * gal_size;
	double core_radius = std::sqrt(active_area / M_PI);
	
	// Refine core_radius iteratively
	for (int iter = 0; iter < 10; ++iter) {
		double overlap = std::max(6.0, 0.15 * core_radius);
		double theta_core = (core_radius / a) * (core_radius / a);
		double theta_outer = delta_theta;
		double arc_length = fermat_spiral_arc_length(a, theta_core, theta_outer);
		double core_planets = (core_radius / GameConstants::min_planet_distance) * 
		                      (core_radius / GameConstants::min_planet_distance);
		double planets_per_arm = arc_length / GameConstants::min_planet_distance;
		double total_planets = core_planets + (params.n_players * planets_per_arm);
		
		if (total_planets < params.n_planets * 0.95) {
			core_radius *= 1.05;
		} else if (total_planets > params.n_planets * 1.05) {
			core_radius *= 0.95;
		} else {
			break;
		}
	}
	
	// Calculate final dimensions
	double overlap = std::max(6.0, 0.15 * core_radius);
	double inner_arm_radius = core_radius - overlap;
	double outer_arm_radius = core_radius * ratio;
	double theta_core = (core_radius / a) * (core_radius / a);
	double theta_outer = delta_theta;
	
	// Phase 3: Generate spiral arms
	std::vector<PlanetCoord> all_coords;
	double arm_angle_step = 2.0 * M_PI / params.n_players;
	
	for (uint32_t arm_idx = 0; arm_idx < params.n_players; ++arm_idx) {
		double arm_angle = arm_idx * arm_angle_step;
		std::vector<PlanetCoord> arm_candidates;
		double angular_step = 0.1;
		
		for (double theta = theta_core; theta <= theta_outer; theta += angular_step) {
			PlanetCoord center = fermat_spiral_point(a, theta, arm_angle);
			double band_thickness = 4.0;
			for (double offset = -band_thickness / 2.0; offset <= band_thickness / 2.0; offset += 1.0) {
				double angle_perp = arm_angle + theta + M_PI / 2.0;
				PlanetCoord offset_point = {
					center.first + offset * std::cos(angle_perp),
					center.second + offset * std::sin(angle_perp)
				};
				arm_candidates.push_back(offset_point);
			}
		}
		
		for (const auto& candidate : arm_candidates) {
			bool valid = true;
			for (const auto& existing : all_coords) {
				double dx = candidate.first - existing.first;
				double dy = candidate.second - existing.second;
				double dist = std::sqrt(dx * dx + dy * dy);
				if (dist < GameConstants::min_planet_distance) {
					valid = false;
					break;
				}
			}
			if (valid) {
				all_coords.push_back(candidate);
			}
		}
	}
	
	// Phase 4: Generate central core
	double seed_radius = inner_arm_radius - GameConstants::min_planet_distance;
	if (seed_radius > 0) {
		CircleRegion core_region(core_radius);
		std::vector<PlanetCoord> core_coords = poisson_disk_sampling(
			core_region,
			GameConstants::min_planet_distance,
			params.n_planets,
			rng,
			all_coords);
		all_coords.insert(all_coords.end(), core_coords.begin(), core_coords.end());
	}
	
	return all_coords;
}

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

std::vector<PlanetCoord> Galaxy::generate_coordinates_ring(
	const GalaxyGenerationParams& params,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	
	// Calculate active area (same formula as random galaxy, but without the 0.85 reduction factor)
	double gal_size = std::sqrt(double(params.n_planets)) * 
	                  (GameConstants::Galaxy_Size_Scale_Base + GameConstants::Galaxy_Size_Scale_Density / params.density);
	double active_area = gal_size * gal_size;
	
	// Calculate outer radius range based on inner/outer ratio constraints
	// If R_inner = 0.5 * R_outer: active_area = pi * (R_outer^2 - 0.25*R_outer^2) = 0.75*pi*R_outer^2
	// If R_inner = 0.8 * R_outer: active_area = pi * (R_outer^2 - 0.64*R_outer^2) = 0.36*pi*R_outer^2
	double r_outer_min = std::sqrt(active_area / (0.75 * M_PI));
	double r_outer_max = std::sqrt(active_area / (0.36 * M_PI));
	
	// Randomly select outer radius within calculated range
	double r_outer = r_outer_min + rng.nextDouble() * (r_outer_max - r_outer_min);
	
	// Calculate inner radius to achieve correct active area
	// active_area = pi * (r_outer^2 - r_inner^2)
	// r_inner = sqrt(r_outer^2 - active_area/pi)
	double r_inner = std::sqrt(r_outer * r_outer - active_area / M_PI);
	
	// Create ring region and generate coordinates using Poisson disk sampling
	RingRegion ring(r_inner, r_outer);
	return poisson_disk_sampling(ring, GameConstants::min_planet_distance, params.n_planets, rng);
}

std::vector<PlanetCoord> Galaxy::generate_coordinates_cluster(
	const GalaxyGenerationParams& params,
	GameState* game_state)
{
	// TODO: Implement cluster coordinate generation
	return std::vector<PlanetCoord>();
}

std::vector<PlanetCoord> Galaxy::generate_coordinates_grid(
	const GalaxyGenerationParams& params,
	GameState* game_state)
{
	std::vector<PlanetCoord> coords;
	
	double planet_spacing = 4.0 + 2.0/(params.density);
	
	uint32_t cols_num = static_cast<uint32_t>(std::ceil(std::sqrt(double(params.n_planets))));
	uint32_t rows_num = static_cast<uint32_t>(std::ceil(double(params.n_planets) / cols_num));
	
	uint32_t planet_idx = 0;
	for (uint32_t row = 0; row < rows_num && planet_idx < params.n_planets; ++row)
	{
		for (uint32_t col = 0; col < cols_num && planet_idx < params.n_planets; ++col)
		{
			GalaxyCoord x_coord = col * planet_spacing;
			GalaxyCoord y_coord = row * planet_spacing;
			
			coords.push_back({x_coord, y_coord});
			planet_idx++;
		}
	}
	
	return coords;
}


// ============================================================================
// Legacy Methods
// Probably soon-to-be deleted.
void Galaxy::initialize_planets_random(
	const GalaxyGenerationParams& params,
	const std::vector<std::string>& planet_names_unused,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	
	// Generate randomized planet names
	const std::vector<std::string> planet_names = generate_planet_names(params.n_planets, game_state);
	
	// Calculate galaxy size using density-aware formula with reduction factor
	// Formula: gal_size = sqrt(n_planets) * (base + density_factor / density) * (1 - reduction_factor)
	gal_size = std::sqrt(double(params.n_planets)) * 
	           (GameConstants::Galaxy_Size_Scale_Base + GameConstants::Galaxy_Size_Scale_Density / params.density) *
	           (1.0 - GameConstants::Galaxy_Size_Reduction_Factor);
	
	GalaxyCoord original_gal_size = gal_size;
	
	// Initialize spatial grid for distance checking
	// Use larger grid to account for potential expansion
	CheckDistanceSpatialGrid grid(GameConstants::min_planet_distance, gal_size * 2.0);
	
	// Galaxy is centered at (0, 0)
	GalaxyCoord half_size = gal_size / 2.0;
	GalaxyCoord min_x = -half_size;
	GalaxyCoord max_x = half_size;
	GalaxyCoord min_y = -half_size;
	GalaxyCoord max_y = half_size;
	
	uint32_t attempts_per_expansion = static_cast<uint32_t>(std::floor(std::sqrt(double(params.n_planets)) / 2.0));
	uint32_t max_attempts_per_planet = params.n_planets;
	uint32_t planets_skipped = 0;
	uint32_t planets_placed = 0;
	
	for (uint32_t i = 0; i < params.n_planets; ++i)
	{
		uint32_t planet_id = i + 1;  // Start planet IDs from 1
		const std::string& planet_name = planet_names[i];
		
		// Random properties (generated once, reused if position needs to be regenerated)
		double true_gravity = GameConstants::min_gravity + rng.nextDouble() * (GameConstants::max_gravity - GameConstants::min_gravity);
		double true_temperature = GameConstants::min_temp + rng.nextDouble() * (GameConstants::max_temp - GameConstants::min_temp);
		int32_t metal = rng.nextInt32Range(GameConstants::min_metal, GameConstants::max_metal);
		
		// Try to find a valid position for this planet
		uint32_t attempt = 0;
		bool placed = false;
		
		while (attempt < max_attempts_per_planet && !placed)
		{
			// Check if we need to expand the galaxy boundaries
			if (attempt > 0 && attempt % attempts_per_expansion == 0)
			{
				// Expand boundaries by 5% around center (0, 0)
				GalaxyCoord expansion_factor = 1.0 + GameConstants::Galaxy_Expansion_Factor;
				min_x *= expansion_factor;
				max_x *= expansion_factor;
				min_y *= expansion_factor;
				max_y *= expansion_factor;
				
				// Update gal_size to reflect new bounds
				gal_size = max_x - min_x;
			}
			// Generate random position within current galaxy bounds
			GalaxyCoord x_coord = min_x + rng.nextDouble() * (max_x - min_x);
			GalaxyCoord y_coord = min_y + rng.nextDouble() * (max_y - min_y);
			
			// Check if position is valid (far enough from other planets)
			if (grid.is_position_valid(x_coord, y_coord, GameConstants::min_planet_distance))
			{
				// Create planet and add to grid
				Planet planet(planet_id, planet_name, x_coord, y_coord, true_gravity, true_temperature, metal);
				planets.push_back(planet);
				grid.add_planet(x_coord, y_coord, planet_id);
				placed = true;
				planets_placed++;
			}
			attempt++;
		}
		
		if (!placed)
		{
			planets_skipped++;
		}
	}
	std::cout << "Attempted to generate a galaxy of size " << original_gal_size << " x " << original_gal_size 
	          << " with " << params.n_planets << " planets.\n";
	std::cout << "Actual galaxy generated with size " << gal_size << " x " << gal_size 
	          << " with " << planets_placed << " planets.\n";
}

void Galaxy::initialize_planets_grid(
	const GalaxyGenerationParams& params,
	const std::vector<std::string>& planet_names,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	
	double planet_spacing = 4.0 + 2.0/(params.density);  // this is probably fine...
	
	uint32_t cols_num = static_cast<uint32_t>(std::ceil(std::sqrt(double(params.n_planets))));
	uint32_t rows_num = static_cast<uint32_t>(std::ceil(double(params.n_planets) / cols_num));
	
	gal_size = std::max(cols_num, rows_num) * planet_spacing;
	
	uint32_t planet_idx = 0;
	for (uint32_t row = 0; row < rows_num && planet_idx < params.n_planets; ++row)
	{
		for (uint32_t col = 0; col < cols_num && planet_idx < params.n_planets; ++col)
		{
			uint32_t planet_id = planet_idx + 1;
			const std::string& planet_name = planet_names[planet_idx];
			
			// Random properties
			double true_gravity = GameConstants::min_gravity + rng.nextDouble() * (GameConstants::max_gravity - GameConstants::min_gravity);
			double true_temperature = GameConstants::min_temp + rng.nextDouble() * (GameConstants::max_temp - GameConstants::min_temp);
			int32_t metal = rng.nextInt32Range(GameConstants::min_metal, GameConstants::max_metal);
			
			// Grid positions
			GalaxyCoord x_coord = col * planet_spacing;
			GalaxyCoord y_coord = row * planet_spacing;
			
			Planet planet(planet_id, planet_name, x_coord, y_coord, true_gravity, true_temperature, metal);
			planets.push_back(planet);
			
			planet_idx++;
		}
	}
}

// ============================================================================
// Distance Matrix Methods
// ============================================================================
void Galaxy::compute_distance_matrix()
{
	size_t n = planets.size();
	distance_matrix.resize(n);
	
	for (size_t i = 0; i < n; ++i)
	{
		distance_matrix[i].resize(n);
		for (size_t j = 0; j < n; ++j)
		{
			const Planet& from = planets[i];
			const Planet& to = planets[j];
			
			// Calculate Euclidean distance
			double dx = to.x - from.x;
			double dy = to.y - from.y;
			double euclidean_dist = std::sqrt(dx * dx + dy * dy);
			
			// Round to nearest integer, store as double
			distance_matrix[i][j] = std::round(euclidean_dist);
		}
	}
}

double Galaxy::get_distance(uint32_t from_id, uint32_t to_id) const
{
	return distance_matrix.at(from_id).at(to_id);
}

// ============================================================================
// Galaxy Generation Failure Logging
// ============================================================================

void log_galaxy_generation_failure(
	const GalaxyGenerationParams& params,
	const std::vector<PlanetCoord>& generated_coords,
	const std::string& log_filename)
{
	std::ofstream log_file(log_filename);
	if (!log_file.is_open())
	{
		std::cerr << "ERROR: Could not open log file: " << log_filename << std::endl;
		return;
	}
	
	// Write header
	log_file << "===============================================================================\n";
	log_file << "GALAXY GENERATION FAILURE REPORT\n";
	log_file << "===============================================================================\n\n";
	
	// Write summary
	log_file << "SUMMARY\n";
	log_file << "-------\n";
	log_file << "Requested planets: " << params.n_planets << "\n";
	log_file << "Generated planets: " << generated_coords.size() << "\n";
	log_file << "Shortfall: " << (params.n_planets - generated_coords.size()) << " planets\n";
	log_file << "Success rate: " << (100.0 * generated_coords.size() / params.n_planets) << "%\n\n";
	
	// Write parameters
	log_file << "GALAXY GENERATION PARAMETERS\n";
	log_file << "----------------------------\n";
	log_file << "Number of planets requested: " << params.n_planets << "\n";
	log_file << "Number of players: " << params.n_players << "\n";
	log_file << "Density parameter: " << params.density << "\n";
	
	// Convert shape enum to string
	std::string shape_name;
	switch (params.shape)
	{
		case GALAXY_RANDOM: shape_name = "RANDOM"; break;
		case GALAXY_SPIRAL: shape_name = "SPIRAL"; break;
		case GALAXY_CIRCLE: shape_name = "CIRCLE"; break;
		case GALAXY_RING: shape_name = "RING"; break;
		case GALAXY_CLUSTER: shape_name = "CLUSTER"; break;
		case GALAXY_GRID: shape_name = "GRID"; break;
		default: shape_name = "UNKNOWN"; break;
	}
	log_file << "Galaxy shape: " << shape_name << "\n";
	log_file << "Random seed: " << params.seed << "\n\n";
	
	// Write generated coordinates
	log_file << "GENERATED PLANET COORDINATES\n";
	log_file << "----------------------------\n";
	log_file << "Total generated: " << generated_coords.size() << "\n\n";
	
	for (size_t i = 0; i < generated_coords.size(); ++i)
	{
		log_file << "Planet " << (i + 1) << ": (" 
		         << generated_coords[i].first << ", " 
		         << generated_coords[i].second << ")\n";
	}
	
	log_file << "\n===============================================================================\n";
	log_file << "END OF REPORT\n";
	log_file << "===============================================================================\n";
	
	log_file.close();
	
	std::cerr << "Galaxy generation failure logged to: " << log_filename << std::endl;
}

std::vector<PlanetCoord> Galaxy::select_home_planets_spiral(
	const std::vector<PlanetCoord>& all_coords,
	uint32_t n_home_planets,
	double a,
	double delta_theta,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	
	// Validate that we have enough coordinates for home planets
	if (all_coords.size() < n_home_planets)
	{
		throw std::runtime_error("Insufficient planet coordinates (" + std::to_string(all_coords.size()) + 
		                         ") for " + std::to_string(n_home_planets) + " home planets.");
	}
	
	std::vector<PlanetCoord> home_coords;
	
	// For each spiral arm, find the planet closest to the end of the arm
	double arm_angle_step = 2.0 * M_PI / n_home_planets;
	
	for (uint32_t arm_idx = 0; arm_idx < n_home_planets; ++arm_idx)
	{
		double arm_angle = arm_idx * arm_angle_step;
		
		// The end of the arm is at theta = delta_theta
		PlanetCoord arm_end = fermat_spiral_point(a, delta_theta, arm_angle);
		
		// Find the closest planet to this arm end
		double min_distance = std::numeric_limits<double>::max();
		PlanetCoord closest_planet = all_coords[0];
		
		for (const auto& coord : all_coords)
		{
			double dx = coord.first - arm_end.first;
			double dy = coord.second - arm_end.second;
			double dist = std::sqrt(dx * dx + dy * dy);
			
			if (dist < min_distance)
			{
				min_distance = dist;
				closest_planet = coord;
			}
		}
		
		home_coords.push_back(closest_planet);
	}
	
	return home_coords;
}

// ============================================================================
// Overloaded coordinate generation methods that take RNG directly
// These are wrappers for the Python bindings to avoid needing GameState
// ============================================================================

std::vector<PlanetCoord> Galaxy::generate_coordinates_random(
	const GalaxyGenerationParams& params,
	DeterministicRNG& rng)
{
	std::vector<PlanetCoord> coords;
	
	// Calculate galaxy size using density-aware formula with reduction factor
	GalaxyCoord gal_size = std::sqrt(double(params.n_planets)) * 
	           (GameConstants::Galaxy_Size_Scale_Base + GameConstants::Galaxy_Size_Scale_Density / params.density) *
	           (1.0 - GameConstants::Galaxy_Size_Reduction_Factor);
	
	// Initialize spatial grid for distance checking
	CheckDistanceSpatialGrid grid(GameConstants::min_planet_distance, gal_size * 2.0);
	
	// Galaxy is centered at (0, 0)
	GalaxyCoord half_size = gal_size / 2.0;
	GalaxyCoord min_x = -half_size;
	GalaxyCoord max_x = half_size;
	GalaxyCoord min_y = -half_size;
	GalaxyCoord max_y = half_size;
	
	uint32_t attempts_per_expansion = static_cast<uint32_t>(std::floor(std::sqrt(double(params.n_planets)) / 2.0));
	uint32_t max_attempts_per_planet = params.n_planets;
	
	for (uint32_t i = 0; i < params.n_planets; ++i)
	{
		uint32_t attempt = 0;
		bool placed = false;
		
		while (attempt < max_attempts_per_planet && !placed)
		{
			// Check if we need to expand the galaxy boundaries
			if (attempt > 0 && attempt % attempts_per_expansion == 0)
			{
				// Expand boundaries by 5% around center (0, 0)
				GalaxyCoord expansion_factor = 1.0 + GameConstants::Galaxy_Expansion_Factor;
				min_x *= expansion_factor;
				max_x *= expansion_factor;
				min_y *= expansion_factor;
				max_y *= expansion_factor;
			}
			// Generate random position within current galaxy bounds
			GalaxyCoord x_coord = min_x + rng.nextDouble() * (max_x - min_x);
			GalaxyCoord y_coord = min_y + rng.nextDouble() * (max_y - min_y);
			
			// Check if position is valid (far enough from other planets)
			if (grid.is_position_valid(x_coord, y_coord, GameConstants::min_planet_distance))
			{
				coords.push_back({x_coord, y_coord});
				grid.add_planet(x_coord, y_coord, i + 1);
				placed = true;
			}
			attempt++;
		}
	}
	
	return coords;
}

std::vector<PlanetCoord> Galaxy::generate_coordinates_spiral(
	const GalaxyGenerationParams& params,
	DeterministicRNG& rng)
{
	// Phase 1: Choose randomized parameters
	double delta_theta = M_PI / 4.0 + rng.nextDouble() * (M_PI - M_PI / 4.0);
	double a = 100.0 / std::sqrt(delta_theta);
	double ratio = 2.0 + rng.nextDouble() * (6.0 - 2.0);
	
	// Phase 2: Calculate overall size
	double gal_size = std::sqrt(double(params.n_planets)) * 
	                  (GameConstants::Galaxy_Size_Scale_Base + GameConstants::Galaxy_Size_Scale_Density / params.density);
	double active_area = gal_size * gal_size;
	double core_radius = std::sqrt(active_area / M_PI);
	
	// Refine core_radius iteratively
	for (int iter = 0; iter < 10; ++iter) {
		double overlap = std::max(6.0, 0.15 * core_radius);
		double theta_core = (core_radius / a) * (core_radius / a);
		double theta_outer = delta_theta;
		double arc_length = fermat_spiral_arc_length(a, theta_core, theta_outer);
		double core_planets = (core_radius / GameConstants::min_planet_distance) * 
		                      (core_radius / GameConstants::min_planet_distance);
		double planets_per_arm = arc_length / GameConstants::min_planet_distance;
		double total_planets = core_planets + (params.n_players * planets_per_arm);
		
		if (total_planets < params.n_planets * 0.95) {
			core_radius *= 1.05;
		} else if (total_planets > params.n_planets * 1.05) {
			core_radius *= 0.95;
		} else {
			break;
		}
	}
	
	// Calculate final dimensions
	double overlap = std::max(6.0, 0.15 * core_radius);
	double inner_arm_radius = core_radius - overlap;
	double outer_arm_radius = core_radius * ratio;
	double theta_core = (core_radius / a) * (core_radius / a);
	double theta_outer = delta_theta;
	
	// Phase 3: Generate spiral arms
	std::vector<PlanetCoord> all_coords;
	double arm_angle_step = 2.0 * M_PI / params.n_players;
	
	for (uint32_t arm_idx = 0; arm_idx < params.n_players; ++arm_idx) {
		double arm_angle = arm_idx * arm_angle_step;
		std::vector<PlanetCoord> arm_candidates;
		double angular_step = 0.1;
		
		for (double theta = theta_core; theta <= theta_outer; theta += angular_step) {
			PlanetCoord center = fermat_spiral_point(a, theta, arm_angle);
			double band_thickness = 4.0;
			for (double offset = -band_thickness / 2.0; offset <= band_thickness / 2.0; offset += 1.0) {
				double angle_perp = arm_angle + theta + M_PI / 2.0;
				PlanetCoord offset_point = {
					center.first + offset * std::cos(angle_perp),
					center.second + offset * std::sin(angle_perp)
				};
				arm_candidates.push_back(offset_point);
			}
		}
		
		for (const auto& candidate : arm_candidates) {
			bool valid = true;
			for (const auto& existing : all_coords) {
				double dx = candidate.first - existing.first;
				double dy = candidate.second - existing.second;
				double dist = std::sqrt(dx * dx + dy * dy);
				if (dist < GameConstants::min_planet_distance) {
					valid = false;
					break;
				}
			}
			if (valid) {
				all_coords.push_back(candidate);
			}
		}
	}
	
	// Phase 4: Generate central core
	double seed_radius = inner_arm_radius - GameConstants::min_planet_distance;
	if (seed_radius > 0) {
		CircleRegion core_region(core_radius);
		std::vector<PlanetCoord> core_coords = poisson_disk_sampling(
			core_region,
			GameConstants::min_planet_distance,
			params.n_planets,
			rng,
			all_coords);
		all_coords.insert(all_coords.end(), core_coords.begin(), core_coords.end());
	}
	
	return all_coords;
}

std::vector<PlanetCoord> Galaxy::generate_coordinates_circle(
	const GalaxyGenerationParams& params,
	DeterministicRNG& rng)
{
	// Calculate galaxy radius based on planet count and density
	double base_radius = std::sqrt(double(params.n_planets) / params.density) * 10.0;
	
	// Use Poisson disk sampling to generate evenly distributed coordinates
	CircleRegion region(base_radius);
	
	double min_distance = GameConstants::min_planet_distance;
	std::vector<PlanetCoord> coords = poisson_disk_sampling(region, min_distance, params.n_planets, rng);
	
	return coords;
}

std::vector<PlanetCoord> Galaxy::generate_coordinates_ring(
	const GalaxyGenerationParams& params,
	DeterministicRNG& rng)
{
	// Calculate galaxy radius based on planet count and density
	double base_radius = std::sqrt(double(params.n_planets) / params.density) * 10.0;
	
	// Use Poisson disk sampling to generate evenly distributed coordinates in a ring
	double inner_radius = base_radius * 0.6;  // Inner radius is 60% of outer
	RingRegion region(inner_radius, base_radius);
	
	double min_distance = GameConstants::min_planet_distance;
	std::vector<PlanetCoord> coords = poisson_disk_sampling(region, min_distance, params.n_planets, rng);
	
	return coords;
}

std::vector<PlanetCoord> Galaxy::generate_coordinates_cluster(
	const GalaxyGenerationParams& params,
	DeterministicRNG& rng)
{
	// Not implemented yet
	(void)params;
	(void)rng;
	return {};
}

std::vector<PlanetCoord> Galaxy::generate_coordinates_grid(
	const GalaxyGenerationParams& params,
	DeterministicRNG& rng)
{
	(void)rng;  // Not used for grid generation
	
	std::vector<PlanetCoord> coords;
	
	// Calculate grid dimensions
	uint32_t grid_size = static_cast<uint32_t>(std::ceil(std::sqrt(double(params.n_planets))));
	double spacing = GameConstants::min_planet_distance * 1.5;  // Slightly more than minimum
	
	// Center the grid at (0, 0)
	double offset_x = -(grid_size - 1) * spacing / 2.0;
	double offset_y = -(grid_size - 1) * spacing / 2.0;
	
	// Generate grid coordinates
	for (uint32_t i = 0; i < params.n_planets; ++i)
	{
		uint32_t row = i / grid_size;
		uint32_t col = i % grid_size;
		
		double x = offset_x + col * spacing;
		double y = offset_y + row * spacing;
		
		coords.push_back({x, y});
	}
	
	return coords;
}
