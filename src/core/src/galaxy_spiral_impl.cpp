// This is a temporary file containing the spiral implementation
// It will be integrated into galaxy.cpp

std::vector<PlanetCoord> Galaxy::generate_coordinates_spiral(
	const GalaxyGenerationParams& params,
	GameState* game_state)
{
	DeterministicRNG& rng = game_state->get_rng();
	
	// Phase 1: Choose randomized parameters
	// Angular span: randomly select from [pi/4, pi]
	double delta_theta = M_PI / 4.0 + rng.nextDouble() * (M_PI - M_PI / 4.0);
	
	// Spiral tightness parameter: a = 100.0 / sqrt(delta_theta)
	double a = 100.0 / std::sqrt(delta_theta);
	
	// Outer/core ratio: randomly select from [2.0, 6.0]
	double ratio = 2.0 + rng.nextDouble() * (6.0 - 2.0);
	
	// Phase 2: Calculate overall size
	// Calculate active area (same formula as random galaxy, but without the 0.85 reduction factor)
	double gal_size = std::sqrt(double(params.n_planets)) * 
	                  (GameConstants::Galaxy_Size_Scale_Base + GameConstants::Galaxy_Size_Scale_Density / params.density);
	double active_area = gal_size * gal_size;
	
	// Solve for core_radius such that total_planets ≈ n_planets
	double core_radius = std::sqrt(active_area / M_PI);
	
	// Refine core_radius iteratively (max 10 iterations)
	for (int iter = 0; iter < 10; ++iter) {
		double overlap = std::max(6.0, 0.15 * core_radius);
		double inner_arm_radius = core_radius - overlap;
		double outer_arm_radius = core_radius * ratio;
		
		// Calculate theta values
		double theta_core = (core_radius / a) * (core_radius / a);
		double theta_outer = delta_theta;
		
		// Calculate arc length using Pythagorean approximation
		double arc_length = fermat_spiral_arc_length(a, theta_core, theta_outer);
		
		// Estimate planets
		double core_planets = (core_radius / GameConstants::min_planet_distance) * 
		                      (core_radius / GameConstants::min_planet_distance);
		double planets_per_arm = arc_length / GameConstants::min_planet_distance;
		double total_planets = core_planets + (params.n_players * planets_per_arm);
		
		// Adjust core_radius
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
	
	// Generate N arms (N = number of players), evenly spaced angularly
	double arm_angle_step = 2.0 * M_PI / params.n_players;
	
	for (uint32_t arm_idx = 0; arm_idx < params.n_players; ++arm_idx) {
		double arm_angle = arm_idx * arm_angle_step;
		
		// Generate candidate points along the spiral arm
		std::vector<PlanetCoord> arm_candidates;
		double angular_step = 0.1;
		
		for (double theta = theta_core; theta <= theta_outer; theta += angular_step) {
			PlanetCoord center = fermat_spiral_point(a, theta, arm_angle);
			
			// Generate points in a band around the spiral
			double band_thickness = 4.0;
			for (double offset = -band_thickness / 2.0; offset <= band_thickness / 2.0; offset += 1.0) {
				// Perpendicular offset from spiral curve
				double angle_perp = arm_angle + theta + M_PI / 2.0;
				PlanetCoord offset_point = {
					center.first + offset * std::cos(angle_perp),
					center.second + offset * std::sin(angle_perp)
				};
				arm_candidates.push_back(offset_point);
			}
		}
		
		// Add candidates that maintain min distance
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
		double seed_angle = rng.nextDouble() * 2.0 * M_PI;
		double seed_r = std::sqrt(rng.nextDouble()) * seed_radius;
		
		// Generate core using Poisson sampling, avoiding arm coordinates
		CircleRegion core_region(core_radius);
		std::vector<PlanetCoord> core_coords = poisson_disk_sampling(
			core_region,
			GameConstants::min_planet_distance,
			params.n_planets,
			rng,
			all_coords);
		
		// Add core coordinates to all_coords
		all_coords.insert(all_coords.end(), core_coords.begin(), core_coords.end());
	}
	
	return all_coords;
}
