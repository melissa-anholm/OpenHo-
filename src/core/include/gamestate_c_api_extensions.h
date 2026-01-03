#ifndef OPENHO_GAMESTATE_C_API_EXTENSIONS_H
#define OPENHO_GAMESTATE_C_API_EXTENSIONS_H

#include "error_codes.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct GameState GameState;

// ============================================================================
// Turn Management & Status
// ============================================================================

/**
 * Mark a player as ready to end their turn
 * Returns: error code
 */
ErrorCode game_mark_player_turn_ready(GameState* game_state, uint32_t player_id);

/**
 * Get turn status for a specific player
 * Returns: 1 if player is ready, 0 if not ready, -1 if invalid player_id
 */
int32_t game_get_player_turn_ready_status(const GameState* game_state, uint32_t player_id);

/**
 * Check if all players are ready to process turn
 * Returns: 1 if all ready, 0 if not all ready
 */
uint32_t game_all_players_ready(const GameState* game_state);

// Note: game_process_turn() already exists in openho_core.h as void game_process_turn(void* game)
// The new validation-aware version will be added as an extension

/**
 * Get current game year
 */
uint32_t game_get_current_year(const GameState* game_state);

// Note: game_get_current_turn() already exists in openho_core.h

// ============================================================================
// Fleet Building Validation & Actions
// ============================================================================

/**
 * Check if a player can build a fleet with given parameters
 * Validates:
 * - Player exists
 * - Design exists and belongs to player
 * - Player owns the planet
 * - Player has sufficient money/metal
 * - Tech levels are available
 * - Ship count is valid
 * Returns: error code (SUCCESS if can build, specific error otherwise)
 */
ErrorCode game_can_player_build_fleet(
	const GameState* game_state,
	uint32_t player_id,
	uint32_t design_id,
	uint32_t ship_count,
	uint32_t planet_id
);

/**
 * Build a fleet for a player (with validation)
 * Should call game_can_player_build_fleet() first, but this will re-validate
 * Returns: fleet_id on success, 0 on error
 */
uint32_t game_player_build_fleet(
	GameState* game_state,
	uint32_t player_id,
	uint32_t design_id,
	uint32_t ship_count,
	uint32_t planet_id
);

// ============================================================================
// Ship Design Validation & Actions
// ============================================================================

/**
 * Check if a player can design a ship with given parameters
 * Validates:
 * - Player exists
 * - Design name is valid and not duplicate
 * - Tech levels are available to player
 * - Tech levels are in valid range
 * - Design limit not reached (max 100)
 * Returns: error code (SUCCESS if can design, specific error otherwise)
 */
ErrorCode game_can_player_design_ship(
	const GameState* game_state,
	uint32_t player_id,
	const char* name,
	uint32_t ship_type,
	int32_t tech_range,
	int32_t tech_speed,
	int32_t tech_weapons,
	int32_t tech_shields,
	int32_t tech_miniaturization
);

/**
 * Design a ship for a player (with validation)
 * Returns: design_id on success, 0 on error
 */
uint32_t game_player_design_ship(
	GameState* game_state,
	uint32_t player_id,
	const char* name,
	uint32_t ship_type,
	int32_t tech_range,
	int32_t tech_speed,
	int32_t tech_weapons,
	int32_t tech_shields,
	int32_t tech_miniaturization
);

// ============================================================================
// Spending Allocation Validation & Actions
// ============================================================================

/**
 * Check if a player can set spending allocation
 * Validates:
 * - Player exists
 * - Fractions are valid (sum to ~1.0, all >= 0.0)
 * Returns: error code (SUCCESS if valid, specific error otherwise)
 */
ErrorCode game_can_player_set_spending_allocation(
	const GameState* game_state,
	uint32_t player_id,
	double savings_fraction,
	double research_fraction,
	double planets_fraction
);

/**
 * Set spending allocation for a player (with validation)
 * Returns: error code
 */
ErrorCode game_player_set_spending_allocation(
	GameState* game_state,
	uint32_t player_id,
	double savings_fraction,
	double research_fraction,
	double planets_fraction
);

// ============================================================================
// Fleet Movement Validation & Actions
// ============================================================================

/**
 * Check if a player can move a fleet to destination
 * Validates:
 * - Player exists
 * - Fleet exists and belongs to player
 * - Destination planet exists
 * - Fleet is not already in transit
 * Returns: error code
 */
ErrorCode game_can_player_move_fleet(
	const GameState* game_state,
	uint32_t player_id,
	uint32_t fleet_id,
	uint32_t destination_planet_id
);

/**
 * Move a fleet for a player (with validation)
 * Returns: error code
 */
ErrorCode game_player_move_fleet(
	GameState* game_state,
	uint32_t player_id,
	uint32_t fleet_id,
	uint32_t destination_planet_id
);

// ============================================================================
// Planet Allocation Validation & Actions
// ============================================================================

/**
 * Check if a player can set planet allocation
 * Validates:
 * - Player exists
 * - Planet exists and is owned by player
 * - Fractions are valid (sum to ~1.0, all >= 0.0)
 * Returns: error code
 */
ErrorCode game_can_player_set_planet_allocation(
	const GameState* game_state,
	uint32_t player_id,
	uint32_t planet_id,
	double mining_fraction,
	double terraforming_fraction
);

/**
 * Set planet allocation for a player (with validation)
 * Returns: error code
 */
ErrorCode game_player_set_planet_allocation(
	GameState* game_state,
	uint32_t player_id,
	uint32_t planet_id,
	double mining_fraction,
	double terraforming_fraction
);

#ifdef __cplusplus
}
#endif

#endif // OPENHO_GAMESTATE_C_API_EXTENSIONS_H
