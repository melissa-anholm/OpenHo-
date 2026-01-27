#include "game_formulas.h"
#include "game_constants.h"
#include "game.h"
#include <cmath>

namespace GameFormulas
{
	// ============================================================================
	// Ship Design Cost Calculations
	// ============================================================================
	int64_t calculate_ship_design_build_cost(int32_t tech_range, int32_t tech_speed, 
	                                          int32_t tech_weapons, int32_t tech_shields, 
	                                          int32_t tech_mini)
	{
		// Placeholder: return 1
		// Real formula will be implemented later
		return 1;
	}
	
	int64_t calculate_ship_design_prototype_cost(int32_t tech_range, int32_t tech_speed, 
	                                              int32_t tech_weapons, int32_t tech_shields, 
	                                              int32_t tech_mini)
	{
		// Placeholder: return 1
		// Real formula will be implemented later
		return 1;
	}
	
	int64_t calculate_ship_design_metal_cost(int32_t tech_range, int32_t tech_speed, 
	                                          int32_t tech_weapons, int32_t tech_shields, 
	                                          int32_t tech_mini)
	{
		// Placeholder: return 1
		// Real formula will be implemented later
		return 1;
	}
	
	// ============================================================================
	// Player Metrics Calculations
	// ============================================================================
	int64_t calculate_player_fleet_power(uint32_t player_id, GameState* game_state)
	{
		// Placeholder: return 1
		// Real formula will be implemented later
		return 1;
	}
	
	int32_t calculate_player_victory_points(uint32_t player_id, GameState* game_state)
	{
		// Placeholder: return 1
		// Real formula will be implemented later
		return 1;
	}
	
	// ============================================================================
	// Money and Resources Calculations
	// ============================================================================
	int64_t calculate_money_interest(int64_t savings)
	{
		if (savings < 0)
		{
			// Negative savings (debt): apply 15% interest on debt
			double interest = savings * 0.15;
			return static_cast<int64_t>(interest);
		}
		else
		{
			// Positive savings: apply 10 * sqrt(savings) interest
			double interest = 10.0 * std::sqrt(static_cast<double>(savings));
			return static_cast<int64_t>(interest);
		}
	}
	
	int64_t convert_money_to_research_points(int64_t money_allocated)
	{
		// Placeholder: one-to-one conversion
		// Real formula will be implemented later
		return money_allocated;
	}
	
	// ============================================================================
	// Technology Advancement Calculations
	// ============================================================================
	int64_t calculate_tech_range_advancement_cost(int32_t current_level)
	{
		// Quadratic cost formula: cost = (level + 1)^2 * base_multiplier
		int64_t base_multiplier = 100;
		int64_t next_level = current_level + 1;
		return next_level * next_level * base_multiplier;
	}
	
	int64_t calculate_tech_speed_advancement_cost(int32_t current_level)
	{
		// Quadratic cost formula: cost = (level + 1)^2 * base_multiplier
		int64_t base_multiplier = 100;
		int64_t next_level = current_level + 1;
		return next_level * next_level * base_multiplier;
	}
	
	int64_t calculate_tech_weapons_advancement_cost(int32_t current_level)
	{
		// Quadratic cost formula: cost = (level + 1)^2 * base_multiplier
		int64_t base_multiplier = 100;
		int64_t next_level = current_level + 1;
		return next_level * next_level * base_multiplier;
	}
	
	int64_t calculate_tech_shields_advancement_cost(int32_t current_level)
	{
		// Quadratic cost formula: cost = (level + 1)^2 * base_multiplier
		int64_t base_multiplier = 100;
		int64_t next_level = current_level + 1;
		return next_level * next_level * base_multiplier;
	}
	
	int64_t calculate_tech_mini_advancement_cost(int32_t current_level)
	{
		// Quadratic cost formula: cost = (level + 1)^2 * base_multiplier
		int64_t base_multiplier = 100;
		int64_t next_level = current_level + 1;
		return next_level * next_level * base_multiplier;
	}
	
	int64_t calculate_tech_radical_advancement_cost(int32_t current_level)
	{
		// Quadratic cost formula: cost = (level + 1)^2 * base_multiplier
		int64_t base_multiplier = 100;
		int64_t next_level = current_level + 1;
		return next_level * next_level * base_multiplier;
	}
	
	// ============================================================================
	// Player Income Calculations
	// ============================================================================
	int64_t calculate_planetary_income(uint32_t player_id, GameState* game_state)
	{
		// Placeholder: return 0
		// Real implementation will sum income from all planets owned by the player
		return 0;
	}
	
	int64_t calculate_interest_income(int64_t savings)
	{
		// Calculate interest on current savings
		return calculate_money_interest(savings);
	}
	
	int64_t calculate_windfall_income(uint32_t player_id, GameState* game_state)
	{
		// Placeholder: return 0
		// Real implementation will handle rare special events
		return 0;
	}
	
	// ============================================================================
	// Population Growth Calculations
	// ============================================================================
	int64_t calculate_population_growth(
		int64_t current_population,
		double planet_temperature,
		double planet_gravity,
		double ideal_temperature,
		double ideal_gravity )
	{
		// Placeholder: simple population growth
		// TODO: Real formula will depend on planet conditions, player ideals, infrastructure, etc.
		// For now: 10% population growth per turn, up to max of 1,000,000
		// Note: Parameters (planet_temperature, planet_gravity, ideal_temperature, ideal_gravity) are unused
		
		const int64_t MAX_POPULATION = 1000000;
		
		// If already at max, no growth
		if (current_population >= MAX_POPULATION)
			return 0;
		
		// Calculate 10% growth
		int64_t growth = static_cast<int64_t>(current_population * 0.10);
		
		// Ensure we don't exceed max population
		int64_t new_population = current_population + growth;
		if (new_population > MAX_POPULATION)
			return MAX_POPULATION - current_population;
		
		return growth;
	}
	
	// ============================================================================
	// Terraforming and Mining Calculations
	// ============================================================================
	double calculate_temperature_change(int64_t money_spent)
	{
		// Terraforming efficiency: temperature_change = 0.01 * money_spent
		// Direction and overshoot prevention are handled by the caller
		double change = money_spent * 0.01;
		return change;
	}
	
	int64_t calculate_money_to_terraform(double temperature_change)
	{
		// Inverse of calculate_temperature_change()
		// money_needed = temperature_change / 0.01
		int64_t money_needed = static_cast<int64_t>(std::round(temperature_change / 0.01));
		return money_needed;
	}
	
	int64_t calculate_metal_mined(int64_t money_spent)
	{
		// Mining efficiency: metal extracted = 20 * sqrt(money spent)
		double metal_extracted = 20.0 * std::sqrt(static_cast<double>(money_spent));
		return static_cast<int64_t>(metal_extracted);
	}
	
	int64_t calculate_money_to_mine(int64_t metal_collected)
	{
		// Inverse of mining efficiency: money needed = (metal_collected / 20)^2
		// Derived from: metal = 20 * sqrt(money)
		// Solving for money: money = (metal / 20)^2
		double metal_d = static_cast<double>(metal_collected);
		double money_needed = (metal_d / 20.0) * (metal_d / 20.0);
		return static_cast<int64_t>(money_needed);
	}
	
	// ============================================================================
	// Planet Mechanics Calculations
	// ============================================================================
	int32_t calculate_planet_nova_warning_duration(DeterministicRNG& rng)
	{
		// Placeholder: return 1
		// Real formula using Poisson distribution will be implemented later
		return 1;
	}
	
}  // namespace GameFormulas

// ============================================================================
// Planetary Perception Calculations
// ============================================================================
double calculate_apparent_gravity(double ideal_gravity, double true_gravity)
{
	// Linear gravity perception formula
	// Line passes through (0, 0) and (ideal_gravity, best_perceived_gravity)
	// Slope = best_perceived_gravity / ideal_gravity
	// Formula: perceived = (best_perceived_gravity / ideal_gravity) * true_gravity
	
	const double slope = GameConstants::best_perceived_gravity / ideal_gravity;
	double perceived_grav = slope * true_gravity;
	
	return perceived_grav;
}

double calculate_apparent_temperature(double ideal_temperature, double true_temperature)
{
	// Calculate perceived temperature based on ideal and true temperature
	// All temperatures are in Kelvin
	// Formula: perceived = ideal + (true - ideal) * compression_factor
	// Compression factor approaches 0 near absolute zero (0K)
	
	// Linear temperature perception formula
	// Line passes through (0, 0) and (ideal_temperature, best_perceived_temperature_K)
	// Slope = best_perceived_temperature_K / ideal_temperature
	// Formula: perceived = (best_perceived_temperature_K / ideal_temperature) * true_temperature
	
	const double slope = GameConstants::best_perceived_temperature_K / ideal_temperature;
	double perceived_temp = slope * true_temperature;
	
	return perceived_temp;
}
