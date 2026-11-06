#ifndef OPENHO_UTILITY_H
#define OPENHO_UTILITY_H

#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>
#include <map>
#include <cmath>
#include <utility>
#include <cctype>
#include <stdexcept>
#include <unordered_set>
#include <type_traits>
#include "rng.h"

// Forward declarations
class Planet;

// Type definitions
using PlanetCoord = std::pair<double, double>;  // (x, y)

// ============================================================================
// Temperature Conversion Utilities
// ============================================================================
// All internal game temperatures are stored in Kelvin.
// These utilities convert between Kelvin and other temperature scales.

namespace TemperatureUtils
{
	// Physical constants
	constexpr double ABSOLUTE_ZERO_K = 0.0;
	constexpr double OFFSET_K_TO_C = 273.15;
	constexpr double FAHRENHEIT_FACTOR = 9.0 / 5.0;
	constexpr double OFFSET_C_TO_F = 32.0;
	constexpr double OFFSET_K_TO_F = 459.67;  // 273.15 * 9/5 - 32
	
	// Best perceived temperature (ideal temperature for colonization)
	// This is the temperature at which planets are most desirable for colonization
	// Value: 295.37223 K (approximately 72°F / 22.22°C)
	constexpr double BEST_PERCEIVED_TEMPERATURE_K = 295.37223;
	constexpr double BEST_PERCEIVED_TEMPERATURE_C = 22.22223;  // 295.37223 - 273.15
	constexpr double BEST_PERCEIVED_TEMPERATURE_F = 72.0;
	
	// ========================================================================
	// Absolute Temperature Conversions
	// These functions convert absolute temperatures between scales
	// ========================================================================
	
	inline double convert_temp_absolute_K_to_C(double temp_k)
	{
		return temp_k - OFFSET_K_TO_C;
	}
	
	inline double convert_temp_absolute_K_to_F(double temp_k)
	{
		return temp_k * FAHRENHEIT_FACTOR - OFFSET_K_TO_F;
	}
	
	inline double convert_temp_absolute_C_to_K(double temp_c)
	{
		return temp_c + OFFSET_K_TO_C;
	}
	
	inline double convert_temp_absolute_C_to_F(double temp_c)
	{
		return temp_c * FAHRENHEIT_FACTOR + OFFSET_C_TO_F;
	}
	
	inline double convert_temp_absolute_F_to_C(double temp_f)
	{
		return (temp_f - OFFSET_C_TO_F) / FAHRENHEIT_FACTOR;
	}
	
	inline double convert_temp_absolute_F_to_K(double temp_f)
	{
		return (temp_f + OFFSET_K_TO_F) / FAHRENHEIT_FACTOR;
	}
	
	// ========================================================================
	// Temperature Change Conversions
	// These functions convert temperature CHANGES (deltas) between scales
	// Note: Offsets don't apply to changes, only scaling factors do
	// ========================================================================
	
	inline double convert_temp_change_C_to_K(double delta_c)
	{
		// Celsius and Kelvin have the same scale, so changes are identical
		return delta_c;
	}
	
	inline double convert_temp_change_K_to_C(double delta_k)
	{
		// Celsius and Kelvin have the same scale, so changes are identical
		return delta_k;
	}
	
	inline double convert_temp_change_F_to_C(double delta_f)
	{
		return delta_f / FAHRENHEIT_FACTOR;
	}
	
	inline double convert_temp_change_C_to_F(double delta_c)
	{
		return delta_c * FAHRENHEIT_FACTOR;
	}
	
	inline double convert_temp_change_F_to_K(double delta_f)
	{
		// F -> C (divide by 9/5), then C -> K (no change)
		return delta_f / FAHRENHEIT_FACTOR;
	}
	
	inline double convert_temp_change_K_to_F(double delta_k)
	{
		// K -> C (no change), then C -> F (multiply by 9/5)
		return delta_k * FAHRENHEIT_FACTOR;
	}
}  // namespace TemperatureUtils

// ============================================================================
// Randomized Subset Generation
// ============================================================================

/**
 * Generate a randomized subset of N items from a source list.
 * 
 * If n_items <= source_list.size():
 *   Returns n_items unique items in random order (no repeats)
 * 
 * If n_items > source_list.size():
 *   Selects additional names randomly from the source list. Repeated names
 *   receive the smallest available numeric suffix (" 2", " 3", etc.). A
 *   source name ending in a numeral is never selected for suffixing.
 * 
 * @tparam T Type of items in the list (typically std::string)
 * @param source_list The list to select from
 * @param n_items Number of items to generate
 * @param rng Deterministic RNG for reproducible randomization
 * @return Vector of n_items in randomized order
 */
template<typename T>
std::vector<T> generate_randomized_subset(
	const std::vector<T>& source_list,
	uint32_t n_items,
	class DeterministicRNG& rng);

// ============================================================================
// Template Implementation
// ============================================================================

template<typename T>
std::vector<T> generate_randomized_subset(
	const std::vector<T>& source_list,
	uint32_t n_items,
	class DeterministicRNG& rng)
{
	std::vector<T> result;
	
	if (source_list.empty() || n_items == 0)
		{ return result; }

	if constexpr (std::is_same_v<T, std::string>)
	{
		std::vector<std::string> unique_source;
		std::unordered_set<std::string> seen_source;
		for (const auto& item : source_list)
		{
			if (seen_source.insert(item).second)
				unique_source.push_back(item);
		}

		std::vector<uint32_t> indices(unique_source.size());
		for (uint32_t i = 0; i < indices.size(); ++i)
			indices[i] = i;
		for (uint32_t i = static_cast<uint32_t>(indices.size()); i > 1; --i)
		{
			const uint32_t j = rng.nextUInt32Range(0, i - 1);
			std::swap(indices[i - 1], indices[j]);
		}

		const uint32_t initial_count = std::min<uint32_t>(n_items, unique_source.size());
		std::unordered_set<std::string> used_names;
		for (uint32_t i = 0; i < initial_count; ++i)
		{
			result.push_back(unique_source[indices[i]]);
			used_names.insert(result.back());
		}

		std::vector<std::string> repeatable_names;
		for (const auto& name : unique_source)
		{
			if (!name.empty() && std::isdigit(static_cast<unsigned char>(name.back())) == 0)
				repeatable_names.push_back(name);
		}
		if (result.size() < n_items && repeatable_names.empty())
			throw std::runtime_error("not enough suffixable planet names");

		while (result.size() < n_items)
		{
			const auto& base = repeatable_names[rng.nextUInt32Range(
				0, static_cast<uint32_t>(repeatable_names.size() - 1))];
			uint32_t suffix = 2;
			std::string candidate;
			do
			{
				candidate = base + " " + std::to_string(suffix++);
			} while (used_names.find(candidate) != used_names.end());
			result.push_back(candidate);
			used_names.insert(candidate);
		}
		return result;
	}

	// Non-string callers retain the original unique shuffled-subset semantics.
	std::vector<uint32_t> indices(source_list.size());
	for (uint32_t i = 0; i < indices.size(); ++i)
		indices[i] = i;
	for (uint32_t i = static_cast<uint32_t>(indices.size()); i > 1; --i)
	{
		const uint32_t j = rng.nextUInt32Range(0, i - 1);
		std::swap(indices[i - 1], indices[j]);
	}
	for (uint32_t i = 0; i < std::min<uint32_t>(n_items, indices.size()); ++i)
		result.push_back(source_list[indices[i]]);
	return result;
}

// ============================================================================
// Spatial Grid for Distance Checking
// ============================================================================

/**
 * Spatial grid data structure for efficient distance-based collision detection.
 * Divides space into cells to avoid O(N^2) distance checking.
 * Used during planet generation to ensure minimum spacing between planets.
 */
class CheckDistanceSpatialGrid
{
public:
	/**
	 * Initialize the spatial grid.
	 * @param cell_size Size of each grid cell (should be >= min_distance)
	 * @param max_coord Maximum coordinate value (galaxy size)
	 */
	CheckDistanceSpatialGrid(double cell_size, double max_coord)
		: cell_size_(cell_size), max_coord_(max_coord)
	{
	}
	
	/**
	 * Add a planet to the grid.
	 * @param x X coordinate
	 * @param y Y coordinate
	 * @param planet_id ID of the planet (for reference)
	 */
	void add_planet(double x, double y, uint32_t planet_id)
	{
		int cell_x = static_cast<int>(x / cell_size_);
		int cell_y = static_cast<int>(y / cell_size_);
		cells_[{cell_x, cell_y}].push_back({x, y, planet_id});
	}
	
	/**
	 * Check if a position is valid (far enough from all existing planets).
	 * @param x X coordinate to check
	 * @param y Y coordinate to check
	 * @param min_distance Minimum required distance from other planets
	 * @return true if position is valid, false otherwise
	 */
	bool is_position_valid(double x, double y, double min_distance) const
	{
		int cell_x = static_cast<int>(x / cell_size_);
		int cell_y = static_cast<int>(y / cell_size_);
		
		// Check this cell and all neighboring cells
		for (int dx = -1; dx <= 1; ++dx)
		{
			for (int dy = -1; dy <= 1; ++dy)
			{
				int check_x = cell_x + dx;
				int check_y = cell_y + dy;
				
				auto it = cells_.find({check_x, check_y});
				if (it != cells_.end())
				{
					for (const auto& planet : it->second)
					{
						double dx_coord = x - planet.x;
						double dy_coord = y - planet.y;
						double distance = std::sqrt(dx_coord * dx_coord + dy_coord * dy_coord);
						
						if (distance < min_distance)
							return false;
					}
				}
			}
		}
		
		return true;
	}
	
	/**
	 * Get the number of planets in the grid.
	 * @return Number of planets added so far
	 */
	uint32_t planet_count() const
	{
		uint32_t count = 0;
		for (const auto& cell : cells_)
		{
			count += cell.second.size();
		}
		return count;
	}
	
private:
	struct PlanetRecord
	{
		double x;
		double y;
		uint32_t planet_id;
	};
	
	double cell_size_;
	double max_coord_;
	std::map<std::pair<int, int>, std::vector<PlanetRecord>> cells_;
};

// ============================================================================
// Poisson Disk Sampling for Uniform Planet Distribution
// ============================================================================

/**
 * Abstract base class for regions used in Poisson disk sampling.
 * Defines the interface that all region types must implement.
 */
class Region
{
public:
	virtual ~Region() = default;
	
	/**
	 * Check if a point is within this region.
	 * @param x X coordinate
	 * @param y Y coordinate
	 * @return true if point is in region, false otherwise
	 */
	virtual bool contains(double x, double y) const = 0;
	
	/**
	 * Generate a random point uniformly within this region.
	 * @param rng Reference to deterministic RNG
	 * @return Random point in region
	 */
	virtual PlanetCoord random_point(class DeterministicRNG& rng) const = 0;
};

/**
 * Circular region for Poisson disk sampling.
 */
class CircleRegion : public Region
{
public:
	/**
	 * Initialize a circular region.
	 * @param radius Radius of the circle
	 */
	explicit CircleRegion(double radius)
		: radius_(radius)
	{
	}
	
	bool contains(double x, double y) const override
	{
		double dist_sq = x * x + y * y;
		return dist_sq <= radius_ * radius_;
	}
	
	PlanetCoord random_point(DeterministicRNG& rng) const override;
	
	double get_radius() const { return radius_; }
	
private:
	double radius_;
};

/**
 * Annular (ring) region for Poisson disk sampling.
 */
class RingRegion : public Region
{
public:
	/**
	 * Initialize a ring region.
	 * @param inner_radius Inner radius of the ring
	 * @param outer_radius Outer radius of the ring
	 */
	RingRegion(double inner_radius, double outer_radius)
		: inner_radius_(inner_radius), outer_radius_(outer_radius)
	{
	}
	
	bool contains(double x, double y) const override
	{
		double dist_sq = x * x + y * y;
		double dist = std::sqrt(dist_sq);
		return dist >= inner_radius_ && dist <= outer_radius_;
	}
	
	PlanetCoord random_point(DeterministicRNG& rng) const override;
	
	double get_inner_radius() const { return inner_radius_; }
	double get_outer_radius() const { return outer_radius_; }
	
private:
	double inner_radius_;
	double outer_radius_;
};

/**
 * Poisson disk sampling algorithm for uniform planet distribution.
 * Generates points that maintain a minimum distance from each other.
 * 
 * @param region The region in which to sample points
 * @param min_distance Minimum required distance between points
 * @param target_points Target number of points to generate (approximately)
 * @param rng Reference to deterministic RNG
 * @param existing_coords Optional existing coordinates to avoid (default: none)
 * @return Vector of generated planet coordinates
 */
std::vector<PlanetCoord> poisson_disk_sampling(
	const Region& region,
	double min_distance,
	uint32_t target_points,
	class DeterministicRNG& rng,
	const std::vector<PlanetCoord>& existing_coords = {});

// ============================================================================
// Spiral Galaxy Helper Functions
// ============================================================================

/**
 * Calculate the arc length of a Fermat spiral segment using Pythagorean approximation.
 * For Fermat's spiral r = a*sqrt(theta), the arc length is approximated as:
 * arc_length ≈ sqrt((r_outer - r_inner)^2 + (r_avg * delta_theta)^2)
 * 
 * @param a Spiral tightness parameter
 * @param theta_inner Starting angle (radians)
 * @param theta_outer Ending angle (radians)
 * @return Approximate arc length along the spiral
 */
double fermat_spiral_arc_length(
	double a,
	double theta_inner,
	double theta_outer);

/**
 * Generate a point on a Fermat spiral at a given angle.
 * 
 * @param a Spiral tightness parameter
 * @param theta Angle (radians)
 * @param arm_angle The angular offset of this arm (radians)
 * @return (x, y) coordinates on the spiral
 */
PlanetCoord fermat_spiral_point(
	double a,
	double theta,
	double arm_angle);

#endif  // OPENHO_UTILITY_H
