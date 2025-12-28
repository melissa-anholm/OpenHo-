#ifndef OPENHO_GAME_FORMULAS_H
#define OPENHO_GAME_FORMULAS_H

#include <cstdint>
#include "rng.h"

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
	                                          int32_t tech_miniaturization);
	
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
	                                              int32_t tech_miniaturization);
	
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
	                                          int32_t tech_miniaturization);
	
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
	int64_t calculate_player_fleet_power(uint32_t player_id, class GameState* game_state);
	
	/// Calculate the victory points for a player based on various factors.
	/// Victory points represent a player's overall progress and strength.
	/// This is a public metric visible to all players.
	/// 
	/// @param playerID The player to calculate victory points for
	/// @param gameState Reference to the game state (for accessing planets, ships, etc.)
	/// @return The victory points value
	int32_t calculate_player_victory_points(uint32_t player_id, class GameState* game_state);
	
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
	int64_t calculate_tech_miniaturization_advancement_cost(int32_t current_level);
	
	/// Calculate the cost to advance Radical technology from level N to N+1.
	/// 
	/// @param current_level The current technology level
	/// @return The cost in research points to advance to the next level
	int64_t calculate_tech_radical_advancement_cost(int32_t current_level);
	
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
	// Future Formula Functions
	// ========================================================================
	// Add new formula functions here as needed for game mechanics.
	
}  // namespace GameFormulas

#endif  // OPENHO_GAME_FORMULAS_H
