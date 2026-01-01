#ifndef OPENHO_GAME_FORMULAS_H
#define OPENHO_GAME_FORMULAS_H

#include <cstdint>
#include "rng.h"

// Forward declaration of GameState (defined in game.h)
class GameState;

// ============================================================================
// Game Balance Formulas
// ============================================================================
// This file contains all formula functions for game balance calculations.
// These functions implement the game's core mechanics and can be tuned
// by adjusting their implementations.

namespace GameFormulas
{
	// ========================================================================
	// Ship Design Cost Calculations
	// ========================================================================
	
	/// Calculate the build cost for a ship design based on its technology levels.
	/// This is the cost to build each subsequent ship of this design (after the prototype).
	/// 
	/// @param techRange Technology level for range
	/// @param techSpeed Technology level for speed
	/// @param techWeapons Technology level for weapons
	/// @param techShields Technology level for shields
	/// @param techMiniaturization Technology level for miniaturization
	/// @return The build cost in money
	int64_t calculate_ship_design_build_cost(int32_t tech_range, int32_t tech_speed, 
	                                          int32_t tech_weapons, int32_t tech_shields, 
	                                          int32_t tech_mini);
	
	/// Calculate the prototype cost for a ship design based on its technology levels.
	/// This is the additional cost paid only for the first ship of a new design.
	/// Typically much more expensive than the build cost.
	/// 
	/// @param techRange Technology level for range
	/// @param techSpeed Technology level for speed
	/// @param techWeapons Technology level for weapons
	/// @param techShields Technology level for shields
	/// @param techMiniaturization Technology level for miniaturization
	/// @return The prototype cost in money
	int64_t calculate_ship_design_prototype_cost(int32_t tech_range, int32_t tech_speed, 
	                                              int32_t tech_weapons, int32_t tech_shields, 
	                                              int32_t tech_mini);
	
	/// Calculate the metal cost for a ship design based on its technology levels.
	/// This is the metal cost to build each ship of this design.
	/// 
	/// @param techRange Technology level for range
	/// @param techSpeed Technology level for speed
	/// @param techWeapons Technology level for weapons
	/// @param techShields Technology level for shields
	/// @param techMiniaturization Technology level for miniaturization
	/// @return The metal cost per ship
	int64_t calculate_ship_design_metal_cost(int32_t tech_range, int32_t tech_speed, 
	                                          int32_t tech_weapons, int32_t tech_shields, 
	                                          int32_t tech_mini);
	
	// ========================================================================
	// Player Metrics Calculations
	// ========================================================================
	
	/// Calculate the fleet power for a player based on their ships.
	/// Fleet power represents the combat strength of all a player's ships.
	/// This is a public metric visible to all players.
	/// 
	/// @param playerID The player to calculate fleet power for
	/// @param gameState Reference to the game state (for accessing ships)
	/// @return The fleet power value
	int64_t calculate_player_fleet_power(uint32_t player_id, GameState* game_state);
	
	/// Calculate the victory points for a player based on various factors.
	/// Victory points represent a player's overall progress and strength.
	/// This is a public metric visible to all players.
	/// 
	/// @param playerID The player to calculate victory points for
	/// @param gameState Reference to the game state (for accessing planets, ships, etc.)
	/// @return The victory points value
	int32_t calculate_player_victory_points(uint32_t player_id, GameState* game_state);
	
	// ========================================================================
	// Money and Resources Calculations
	// ========================================================================
	
	/// Calculate interest earned or owed on a player's money savings.
	/// Positive savings earn interest at MONEY_INTEREST_RATE_POSITIVE.
	/// Negative savings (debt) incur interest at MONEY_INTEREST_RATE_DEBT.
	/// 
	/// @param savings The player's current money savings (can be negative for debt)
	/// @return The interest amount to add to savings (positive or negative)
	int64_t calculate_money_interest(int64_t savings);
	
	/// Convert money allocated to research into research points.
	/// This formula determines the efficiency of research spending.
	/// 
	/// @param money_allocated The amount of money allocated to research
	/// @return The number of research points generated from this money
	int64_t convert_money_to_research_points(int64_t money_allocated);
	
	// ========================================================================
	// Player Income Calculations
	// ========================================================================
	
	/// Calculate a player's income from their owned planets.
	/// This is the sum of metal and money production from all planets.
	/// 
	/// @param player_id The ID of the player
	/// @param game_state Pointer to the game state for accessing planet data
	/// @return The total planetary income for this player
	int64_t calculate_planetary_income(uint32_t player_id, GameState* game_state);
	
	/// Calculate interest earned or owed on a player's savings.
	/// Positive interest is earned on savings, negative interest is owed on debt.
	/// 
	/// @param savings The player's current money savings (can be negative for debt)
	/// @return The interest amount (positive or negative)
	int64_t calculate_interest_income(int64_t savings);
	
	/// Calculate windfall income from rare special events.
	/// This is typically zero but can be non-zero during special events.
	/// 
	/// @param player_id The ID of the player
	/// @param game_state Pointer to the game state
	/// @return The windfall income for this player
	int64_t calculate_windfall_income(uint32_t player_id, GameState* game_state);
	
	// ========================================================================
	// Population Growth Calculations
	// ========================================================================
	
	/// Calculate population growth for a planet.
	/// Growth depends on the planet's properties, the player's ideal conditions,
	/// and the current population.
	/// 
	/// @param current_population The planet's current population
	/// @param planet_temperature The planet's current temperature
	/// @param planet_gravity The planet's gravity
	/// @param ideal_temperature The player's ideal temperature
	/// @param ideal_gravity The player's ideal gravity
	/// @return The population growth (can be positive, zero, or negative)
	int64_t calculate_population_growth(
		int64_t current_population,
		double planet_temperature,
		double planet_gravity,
		double ideal_temperature,
		double ideal_gravity
	);
	
	// ========================================================================
	// Technology Advancement Calculations
	// ========================================================================
	
	/// Calculate the cost to advance Range technology from level N to N+1.
	/// Cost increases quadratically with technology level.
	/// 
	/// @param current_level The current technology level
	/// @return The cost in research points to advance to the next level
	int64_t calculate_tech_range_advancement_cost(int32_t current_level);
	
	/// Calculate the cost to advance Speed technology from level N to N+1.
	/// 
	/// @param current_level The current technology level
	/// @return The cost in research points to advance to the next level
	int64_t calculate_tech_speed_advancement_cost(int32_t current_level);
	
	/// Calculate the cost to advance Weapons technology from level N to N+1.
	/// 
	/// @param current_level The current technology level
	/// @return The cost in research points to advance to the next level
	int64_t calculate_tech_weapons_advancement_cost(int32_t current_level);
	
	/// Calculate the cost to advance Shields technology from level N to N+1.
	/// 
	/// @param current_level The current technology level
	/// @return The cost in research points to advance to the next level
	int64_t calculate_tech_shields_advancement_cost(int32_t current_level);
	
	/// Calculate the cost to advance Miniaturization technology from level N to N+1.
	/// 
	/// @param current_level The current technology level
	/// @return The cost in research points to advance to the next level
	int64_t calculate_tech_mini_advancement_cost(int32_t current_level);
	
	/// Calculate the cost to advance Radical technology from level N to N+1.
	/// 
	/// @param current_level The current technology level
	/// @return The cost in research points to advance to the next level
	int64_t calculate_tech_radical_advancement_cost(int32_t current_level);
	
	// ========================================================================
	// Terraforming and Mining Calculations
	// ========================================================================
	
	/// Calculate the temperature change from terraforming spending.
	/// The amount of temperature change depends on the money spent and how far
	/// the current temperature is from the target temperature.
	/// 
	/// @param money_spent The amount of money allocated to terraforming this turn
	/// @param current_temperature The planet's current temperature
	/// @param target_temperature The player's ideal temperature
	/// @return The amount of temperature change (can be positive or negative)
	double calculate_temperature_change(int64_t money_spent, double current_temperature, double target_temperature);
	
	/// Calculate the amount of metal that can be mined from a planet.
	/// The amount of metal extracted depends on the money spent and how much
	/// metal remains on the planet.
	/// 
	/// @param money_spent The amount of money allocated to mining this turn
	/// @param metal_remaining The amount of metal remaining on the planet
	/// @return The amount of metal that can be extracted this turn
	int64_t calculate_metal_mined(int64_t money_spent, int64_t metal_remaining);
	
	// ========================================================================
	// Planet Mechanics Calculations
	// ========================================================================
	
	/// Calculate the warning duration for a planet entering nova state.
	/// Uses a Poisson distribution to determine how many turns the planet
	/// will remain in warning state before being destroyed.
	/// 
	/// @param rng Reference to the deterministic RNG for reproducible results
	/// @return The number of turns until the planet is destroyed (1 to MAX_NOVA_WARNING_TURNS)
	int32_t calculate_planet_nova_warning_duration(DeterministicRNG& rng);
	
	// ========================================================================
	// Planetary Perception Calculations
	// ========================================================================

	/// Calculate how a player perceives a planet's gravity based on their ideal.
	/// The apparent gravity reflects how suitable the planet is for the player,
	/// based on the difference between true gravity and ideal gravity.
	/// 
	/// @param ideal_gravity The player's ideal gravity
	/// @param true_gravity The planet's actual gravity
	/// @return The apparent gravity value as perceived by the player
	double calculate_apparent_gravity(double ideal_gravity, double true_gravity);

	/// Calculate how a player perceives a planet's temperature based on their ideal.
	/// The apparent temperature reflects how suitable the planet is for the player,
	/// based on the difference between true temperature and ideal temperature.
	/// 
	/// @param ideal_temperature The player's ideal temperature
	/// @param true_temperature The planet's actual temperature
	/// @return The apparent temperature value as perceived by the player
	double calculate_apparent_temperature(double ideal_temperature, double true_temperature);

	// ========================================================================
	// Future Formula Functions
	// ========================================================================
	// Add new formula functions here as needed for game mechanics.

	}  // namespace GameFormulas

#endif  // OPENHO_GAME_FORMULAS_H
