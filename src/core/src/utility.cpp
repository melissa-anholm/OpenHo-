#include "utility.h"
#include "rng.h"
#include <cmath>
#include <map>
#include <vector>
#include <algorithm>

// ============================================================================
// Region Implementation: CircleRegion
// ============================================================================

PlanetCoord CircleRegion::random_point(DeterministicRNG& rng) const
{
	// Generate random point uniformly in circle using sqrt weighting for radius
	double angle = rng.nextDouble() * 2.0 * M_PI;
	double radius_factor = std::sqrt(rng.nextDouble());  // sqrt for uniform distribution
	double r = radius_ * radius_factor;
	
	double x = r * std::cos(angle);
	double y = r * std::sin(angle);
	
	return {x, y};
}

// ============================================================================
// Region Implementation: RingRegion
// ============================================================================

PlanetCoord RingRegion::random_point(DeterministicRNG& rng) const
{
	// Generate random point uniformly in annulus
	double angle = rng.nextDouble() * 2.0 * M_PI;
	
	// For uniform distribution in annulus, use inverse transform sampling
	// r = sqrt(r_inner^2 + random * (r_outer^2 - r_inner^2))
	double r_inner_sq = inner_radius_ * inner_radius_;
	double r_outer_sq = outer_radius_ * outer_radius_;
	double r_sq = r_inner_sq + rng.nextDouble() * (r_outer_sq - r_inner_sq);
	double r = std::sqrt(r_sq);
	
	double x = r * std::cos(angle);
	double y = r * std::sin(angle);
	
	return {x, y};
}

// ============================================================================
// Poisson Disk Sampling Implementation
// ============================================================================

std::vector<PlanetCoord> poisson_disk_sampling(
	const Region& region,
	double min_distance,
	uint32_t target_points,
	DeterministicRNG& rng)
{
	std::vector<PlanetCoord> output;
	std::vector<PlanetCoord> active;
	
	// Cell size for spatial grid: min_distance / sqrt(2)
	double cell_size = min_distance / std::sqrt(2.0);
	
	// Spatial grid for efficient neighbor checking
	std::map<std::pair<int, int>, std::vector<PlanetCoord>> grid;
	
	// Lambda to add point to grid
	auto add_to_grid = [&](const PlanetCoord& p) -> void {
		int cell_x = static_cast<int>(p.first / cell_size);
		int cell_y = static_cast<int>(p.second / cell_size);
		grid[{cell_x, cell_y}].push_back(p);
	};
	
	// Lambda to check if position is valid (far enough from all existing points)
	auto is_valid = [&](const PlanetCoord& p) -> bool {
		int cell_x = static_cast<int>(p.first / cell_size);
		int cell_y = static_cast<int>(p.second / cell_size);
		
		// Check this cell and all 8 neighboring cells
		for (int dx = -1; dx <= 1; ++dx) {
			for (int dy = -1; dy <= 1; ++dy) {
				int check_x = cell_x + dx;
				int check_y = cell_y + dy;
				
				auto it = grid.find({check_x, check_y});
				if (it != grid.end()) {
					for (const auto& existing : it->second) {
						double dx_coord = p.first - existing.first;
						double dy_coord = p.second - existing.second;
						double dist = std::sqrt(dx_coord * dx_coord + dy_coord * dy_coord);
						
						if (dist < min_distance) {
							return false;
						}
					}
				}
			}
		}
		return true;
	};
	
	// Start with a random point in the region
	PlanetCoord first = region.random_point(rng);
	if (region.contains(first.first, first.second)) {
		output.push_back(first);
		active.push_back(first);
		add_to_grid(first);
	}
	
	// Number of candidate attempts per active point
	const int k = 30;
	
	// Process active list
	while (!active.empty()) {
		// Pick random active point
		int idx = rng.nextInt32Range(0, active.size() - 1);
		PlanetCoord p = active[idx];
		bool found = false;
		
		// Try k candidate points around this point
		for (int i = 0; i < k; ++i) {
			// Generate random point in annulus around p
			double angle = rng.nextDouble() * 2.0 * M_PI;
			double distance = min_distance + rng.nextDouble() * min_distance;  // [min_distance, 2*min_distance]
			
			double candidate_x = p.first + distance * std::cos(angle);
			double candidate_y = p.second + distance * std::sin(angle);
			PlanetCoord candidate = {candidate_x, candidate_y};
			
			// Check if candidate is in region and valid
			if (region.contains(candidate_x, candidate_y) && is_valid(candidate)) {
				output.push_back(candidate);
				active.push_back(candidate);
				add_to_grid(candidate);
				found = true;
				break;
			}
		}
		
		// If no valid candidates found, remove from active list
		if (!found) {
			active.erase(active.begin() + idx);
		}
	}
	
	return output;
}
