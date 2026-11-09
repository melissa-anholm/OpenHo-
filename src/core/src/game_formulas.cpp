#include "game_formulas.h"
#include "game_constants.h"
#include "utility.h"
#include <cmath>
#include <limits>

namespace GameFormulas
{
	// ============================================================================
	// Ship Design Cost Calculations
	// ============================================================================
	namespace
	{
		using IncrementFormula = double (*)(double);

		int64_t round_up_cost(double value)
		{
			// Move one representable step toward zero before ceil() so values
			// such as 600 * pow(0.9, 3), which are mathematically 486 but may
			// be represented as 486.00000000000006, remain 486.
			return static_cast<int64_t>(std::ceil(std::nextafter(value, 0.0)));
		}

			bool is_supported_ship_type(ShipType ship_type)
			{
				return ship_type >= SHIP_SCOUT && ship_type <= SHIP_DREADNOUGHT;
			}

			double prototype_multiplier(ShipType ship_type)
			{
				switch (ship_type)
				{
				case SHIP_COLONY:
				case SHIP_TANKER:
				case SHIP_DREADNOUGHT:
					return GameConstants::Ship_Build_Prototype_Multiplier_Heavy;
				case SHIP_SCOUT:
				case SHIP_FIGHTER:
				case SHIP_SATELLITE:
					return GameConstants::Ship_Build_Prototype_Multiplier_Light;
				default:
					return 0.0;
				}
			}

		double calculate_range_money_increment(double current_level)
		{
			(void)current_level;
			return GameConstants::Ship_Build_Range_Money_Increment;
		}

		double calculate_speed_money_increment(double current_level)
		{
			(void)current_level;
			return GameConstants::Ship_Build_Speed_Money_Increment;
		}

		double calculate_weapons_money_increment(double current_level)
		{
			(void)current_level;
			return GameConstants::Ship_Build_Weapons_Money_Increment;
		}

		double calculate_shields_money_increment(double current_level)
		{
			(void)current_level;
			return GameConstants::Ship_Build_Shields_Money_Increment;
		}

		double calculate_range_metal_increment(double current_level)
		{
			(void)current_level;
			return GameConstants::Ship_Build_Range_Metal_Increment;
		}

		double calculate_speed_metal_increment(double current_level)
		{
			(void)current_level;
			return GameConstants::Ship_Build_Speed_Metal_Increment;
		}

		double calculate_weapons_metal_increment(double current_level)
		{
			(void)current_level;
			return GameConstants::Ship_Build_Weapons_Metal_Increment;
		}

		double calculate_shields_metal_increment(double current_level)
		{
			(void)current_level;
			return GameConstants::Ship_Build_Shields_Metal_Increment;
		}

		double ship_type_multiplier(ShipType ship_type, bool metal)
		{
			const uint32_t index = static_cast<uint32_t>(ship_type);
			return metal ? GameConstants::Ship_Build_Metal_Tech_Multiplier[index]
			             : GameConstants::Ship_Build_Money_Tech_Multiplier[index];
		}

		double stream_multiplier(ShipType ship_type, TechStream stream, bool metal)
		{
			// Satellite speed technology has no cost in either resource.
			if (ship_type == SHIP_SATELLITE && stream == TECH_SPEED)
				return 0.0;
			return ship_type_multiplier(ship_type, metal);
		}

		double sum_stream_cost(int32_t level, ShipType ship_type, TechStream stream,
		                      bool metal, IncrementFormula increment_formula)
		{
			double total = 0.0;
			const double multiplier = stream_multiplier(ship_type, stream, metal);
			for (int32_t transition = 0; transition < level; ++transition)
			{
				// Technology levels are stored as integers at the API boundary, but
				// every formula receives the transition level as a double.
				const double current_level = static_cast<double>(transition);
				total += increment_formula(current_level) * multiplier;
			}
			return total;
		}

		double mini_money_multiplier(int32_t tech_mini)
		{
			if (tech_mini == 0)
				return 1.0;
			return GameConstants::Ship_Build_Mini_Money_Contribution
				* static_cast<double>(tech_mini);
		}

		double mini_metal_multiplier(int32_t tech_mini)
		{
			return std::pow(GameConstants::Ship_Build_Mini_Metal_Factor,
			                static_cast<double>(tech_mini));
		}

		double unrounded_money_cost(ShipType ship_type, int32_t tech_range,
		                           int32_t tech_speed, int32_t tech_weapons,
		                           int32_t tech_shields, int32_t tech_mini)
		{
			const uint32_t index = static_cast<uint32_t>(ship_type);
			double total = GameConstants::Ship_Build_Base_Money[index];
			total += sum_stream_cost(tech_range, ship_type, TECH_RANGE, false,
			                         calculate_range_money_increment);
			total += sum_stream_cost(tech_speed, ship_type, TECH_SPEED, false,
			                         calculate_speed_money_increment);
			total += sum_stream_cost(tech_weapons, ship_type, TECH_WEAPONS, false,
			                         calculate_weapons_money_increment);
			total += sum_stream_cost(tech_shields, ship_type, TECH_SHIELDS, false,
			                         calculate_shields_money_increment);
			return total * mini_money_multiplier(tech_mini);
		}

		double unrounded_metal_cost(ShipType ship_type, int32_t tech_range,
		                           int32_t tech_speed, int32_t tech_weapons,
		                           int32_t tech_shields, int32_t tech_mini)
		{
			const uint32_t index = static_cast<uint32_t>(ship_type);
			double total = GameConstants::Ship_Build_Base_Metal[index];
			total += sum_stream_cost(tech_range, ship_type, TECH_RANGE, true,
			                         calculate_range_metal_increment);
			total += sum_stream_cost(tech_speed, ship_type, TECH_SPEED, true,
			                         calculate_speed_metal_increment);
			total += sum_stream_cost(tech_weapons, ship_type, TECH_WEAPONS, true,
			                         calculate_weapons_metal_increment);
			total += sum_stream_cost(tech_shields, ship_type, TECH_SHIELDS, true,
			                         calculate_shields_metal_increment);
			return total * mini_metal_multiplier(tech_mini);
		}
	}

	bool check_ship_design_levels(ShipType ship_type, int32_t requested_range_level,
	                              int32_t requested_speed_level,
	                              int32_t requested_weapons_level,
	                              int32_t requested_shields_level,
	                              int32_t requested_miniaturization_level)
	{
		if (!is_supported_ship_type(ship_type))
			return false;
		if (requested_range_level < 0 || requested_speed_level < 1 ||
			requested_weapons_level < 1 || requested_shields_level < 1 ||
			requested_miniaturization_level < 0)
			return false;
		if (ship_type == SHIP_SATELLITE && requested_range_level != 0)
			return false;
		return true;
	}

	int32_t ship_design_technology_ceiling(ShipType ship_type, TechStream stream,
	                                       int32_t player_technology_level)
	{
		int32_t offset = 0;
		if (ship_type == SHIP_SATELLITE && stream == TECH_RANGE)
			return 0;
		if (ship_type == SHIP_TANKER &&
			(stream == TECH_RANGE || stream == TECH_SPEED))
			offset = -1;
		if (ship_type == SHIP_SCOUT)
		{
			if (stream == TECH_RANGE)
				offset = 2;
			else if (stream == TECH_WEAPONS || stream == TECH_SHIELDS)
				offset = -1;
		}
		return player_technology_level + offset;
	}

	int64_t calculate_ship_design_build_cost(ShipType ship_type, int32_t tech_range,
	                                          int32_t tech_speed, int32_t tech_weapons,
	                                          int32_t tech_shields, int32_t tech_mini)
	{
		return round_up_cost(unrounded_money_cost(
			ship_type, tech_range, tech_speed, tech_weapons, tech_shields, tech_mini));
	}

	int64_t calculate_ship_design_prototype_cost(ShipType ship_type, int32_t tech_range,
	                                              int32_t tech_speed, int32_t tech_weapons,
	                                              int32_t tech_shields, int32_t tech_mini)
	{
			return round_up_cost(prototype_multiplier(ship_type)
				* unrounded_money_cost(
				ship_type, tech_range, tech_speed, tech_weapons, tech_shields, tech_mini));
	}

	int64_t calculate_ship_design_metal_cost(ShipType ship_type, int32_t tech_range,
	                                          int32_t tech_speed, int32_t tech_weapons,
	                                          int32_t tech_shields, int32_t tech_mini)
	{
		return round_up_cost(unrounded_metal_cost(
			ship_type, tech_range, tech_speed, tech_weapons, tech_shields, tech_mini));
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
		return calculate_money_interest(savings, 1.0);
	}

	int64_t calculate_money_interest(int64_t savings, double positive_rate_modifier)
	{
		if (savings < 0)
		{
			// Negative savings (debt): apply the provisional debt-interest rate.
			double interest = savings * GameConstants::Provisional_Debt_Interest_Rate;
			return static_cast<int64_t>(interest);
		}
		else
		{
			// Positive savings: apply the modifier to the positive-interest rule.
			double interest = GameConstants::Provisional_Positive_Savings_Interest_Coefficient *
				positive_rate_modifier * std::sqrt(static_cast<double>(savings));
			return static_cast<int64_t>(interest);
		}
	}

	int64_t calculate_intentional_debt_limit(
		int64_t summed_planetary_income, uint32_t planet_count)
	{
		// Planet count is part of the policy boundary now, even though the first
		// provisional formula depends only on summed planetary income.
		(void)planet_count;
		if (summed_planetary_income <= 0)
			return 0;

		const double income = static_cast<double>(summed_planetary_income);
		const double rate = GameConstants::Provisional_Debt_Interest_Rate;
		const int32_t turns = GameConstants::Provisional_Intentional_Debt_Repayment_Turns;
		const double repayment_factor = rate == 0.0
			? static_cast<double>(turns)
			: (1.0 - std::pow(1.0 + rate, -turns)) / rate;
		const double limit = income * repayment_factor;
		if (limit >= static_cast<double>(std::numeric_limits<int64_t>::max()))
			return std::numeric_limits<int64_t>::max();
		return static_cast<int64_t>(std::floor(limit));
	}

	int64_t calculate_bankruptcy_debt_limit(
		int64_t summed_planetary_income, uint32_t planet_count)
	{
		// The provisional bankruptcy threshold is five times the debt at which
		// interest equals planetary income. Use at least the reference income so
		// low-income players do not receive an artificially small threshold.
		(void)planet_count;
		const int64_t effective_income = std::max(
			summed_planetary_income,
			GameConstants::Provisional_Bankruptcy_Reference_Income);
		const double debt_limit =
			static_cast<double>(effective_income)
			* static_cast<double>(GameConstants::Provisional_Bankruptcy_Debt_Multiplier)
			/ GameConstants::Provisional_Debt_Interest_Rate;
		if (debt_limit >= static_cast<double>(std::numeric_limits<int64_t>::max()))
			return std::numeric_limits<int64_t>::max();
		return static_cast<int64_t>(std::floor(debt_limit));
	}

	int64_t calculate_available_build_money(
		int64_t current_savings,
		int64_t summed_planetary_income,
		uint32_t planet_count)
	{
		const int64_t intentional_limit = calculate_intentional_debt_limit(
			summed_planetary_income, planet_count);
		const int64_t available_money = current_savings >
			std::numeric_limits<int64_t>::max() - intentional_limit
			? std::numeric_limits<int64_t>::max()
			: current_savings + intentional_limit;
		return available_money > 0 ? available_money : 0;
	}

	bool has_reached_bankruptcy_limit(
		int64_t current_savings,
		int64_t summed_planetary_income,
		uint32_t planet_count)
	{
		return current_savings <= -calculate_bankruptcy_debt_limit(
			summed_planetary_income, planet_count);
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
		// For now: 10% population growth per turn, up to the centralized cap.
		// Note: Parameters (planet_temperature, planet_gravity, ideal_temperature, ideal_gravity) are unused
		
		const int64_t MAX_POPULATION = GameConstants::Maximum_Population;
		
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
		// Terraforming efficiency formula (in Fahrenheit):
		// delta_temperature_fahrenheit = 0.0811 * sqrt(money_spent)
		// This is converted to Kelvin for internal storage
		double delta_fahrenheit = 0.0811 * std::sqrt(static_cast<double>(money_spent));
		
		// Convert Fahrenheit temperature change to Kelvin
		double delta_kelvin = TemperatureUtils::convert_temp_change_F_to_K(delta_fahrenheit);
		
		return delta_kelvin;
	}
	
	int64_t calculate_money_to_terraform(double temperature_change_kelvin)
	{
		// Inverse of calculate_temperature_change()
		// Input: temperature_change in Kelvin
		// Convert Kelvin change to Fahrenheit change
		double delta_fahrenheit = TemperatureUtils::convert_temp_change_K_to_F(temperature_change_kelvin);
		
		// Solve for money: delta_fahrenheit = 0.0811 * sqrt(money)
		// money = (delta_fahrenheit / 0.0811)^2
		double money_needed_double = (delta_fahrenheit / 0.0811) * (delta_fahrenheit / 0.0811);
		int64_t money_needed = static_cast<int64_t>(std::round(money_needed_double));
		
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
namespace GameFormulas
{
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

int32_t calculate_fuel_consumption(double distance)
{
	if (!std::isfinite(distance) || distance <= 0.0)
		return 0;
	if (distance >= static_cast<double>(std::numeric_limits<int32_t>::max()))
		return std::numeric_limits<int32_t>::max();
	return static_cast<int32_t>(std::round(distance));
}

}  // namespace GameFormulas
