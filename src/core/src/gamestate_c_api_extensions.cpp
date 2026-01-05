#include "gamestate_c_api_extensions.h"
#include "game.h"
#include "player.h"

// ============================================================================
// Turn Management & Status
// ============================================================================

ErrorCode game_mark_player_turn_ready(GameState* game_state, uint32_t player_id)
{
	if (!game_state)
		return ErrorCode::INVALID_PARAMETER;
	
	// TODO: Implement turn ready tracking
	return ErrorCode::SUCCESS;
}

int32_t game_get_player_turn_ready_status(const GameState* game_state, uint32_t player_id)
{
	if (!game_state)
		return -1;
	
	// TODO: Implement turn ready status query
	return 0;
}

uint32_t game_all_players_ready(const GameState* game_state)
{
	if (!game_state)
		return 0;
	
	// TODO: Implement check for all players ready
	return 0;
}

// Note: game_process_turn() is implemented in c_api.cpp
// Note: game_get_current_turn() is implemented in c_api.cpp

uint32_t game_get_current_year(const GameState* game_state)
{
	if (!game_state)
		return 0;
	
	// TODO: Add getter to GameState if not present
	return 0;
}

// ============================================================================
// Fleet Building Validation & Actions
// ============================================================================

ErrorCode game_can_player_build_fleet(
	const GameState* game_state,
	uint32_t player_id,
	uint32_t design_id,
	uint32_t ship_count,
	uint32_t planet_id)
{
	if (!game_state)
		return ErrorCode::INVALID_PARAMETER;
	
	return game_state->check_player_build_fleet(player_id, design_id, ship_count, planet_id);
}

uint32_t game_player_build_fleet(
	GameState* game_state,
	uint32_t player_id,
	uint32_t design_id,
	uint32_t ship_count,
	uint32_t planet_id)
{
	if (!game_state)
		return 0;
	
	// Validate first
	if (game_state->check_player_build_fleet(player_id, design_id, ship_count, planet_id) != ErrorCode::SUCCESS)
		return 0;
	
	// Get player and delegate to Player::create_fleet
	GameState* mutable_game_state = const_cast<GameState*>(game_state);
	Player* player = mutable_game_state->get_player(player_id);
	if (!player)
		return 0;
	
	return player->create_fleet(design_id, ship_count, planet_id);
}

// ============================================================================
// Ship Design Validation & Actions
// ============================================================================

ErrorCode game_can_player_design_ship(
	const GameState* game_state,
	uint32_t player_id,
	const char* name,
	uint32_t ship_type,
	int32_t tech_range,
	int32_t tech_speed,
	int32_t tech_weapons,
	int32_t tech_shields,
	int32_t tech_miniaturization)
{
	if (!game_state || !name)
		return ErrorCode::INVALID_PARAMETER;
	
	return game_state->check_player_design_ship(player_id, std::string(name), static_cast<ShipType>(ship_type), tech_range, tech_speed, tech_weapons, tech_shields, tech_miniaturization);
}

uint32_t game_player_design_ship(
	GameState* game_state,
	uint32_t player_id,
	const char* name,
	uint32_t ship_type,
	int32_t tech_range,
	int32_t tech_speed,
	int32_t tech_weapons,
	int32_t tech_shields,
	int32_t tech_miniaturization)
{
	if (!game_state || !name)
		return 0;
	
	// Validate first
	if (game_state->check_player_design_ship(player_id, std::string(name), static_cast<ShipType>(ship_type), tech_range, tech_speed, tech_weapons, tech_shields, tech_miniaturization) != ErrorCode::SUCCESS)
		return 0;
	
	// Delegate to GameState
	return game_state->create_ship_design(player_id, std::string(name), static_cast<ShipType>(ship_type), tech_range, tech_speed, tech_weapons, tech_shields, tech_miniaturization);
}

// ============================================================================
// Spending Allocation Validation & Actions
// ============================================================================

ErrorCode game_can_player_set_spending_allocation(
	const GameState* game_state,
	uint32_t player_id,
	double savings_fraction,
	double research_fraction,
	double planets_fraction)
{
	if (!game_state)
		return ErrorCode::INVALID_PARAMETER;
	
	return game_state->check_player_set_spending_allocation(player_id, savings_fraction, research_fraction, planets_fraction);
}

ErrorCode game_player_set_spending_allocation(
	GameState* game_state,
	uint32_t player_id,
	double savings_fraction,
	double research_fraction,
	double planets_fraction)
{
	if (!game_state)
		return ErrorCode::INVALID_PARAMETER;
	
	// Validate first
	if (game_state->check_player_set_spending_allocation(player_id, savings_fraction, research_fraction, planets_fraction) != ErrorCode::SUCCESS)
		return ErrorCode::INVALID_ALLOCATION;
	
	// Delegate to GameState - note: research allocation struct needs to be populated
	Player::MoneyAllocation alloc;
	alloc.savings_fraction = savings_fraction;
	alloc.research_fraction = research_fraction;
	alloc.planets_fraction = planets_fraction;
	game_state->set_money_allocation(player_id, alloc);
	return ErrorCode::SUCCESS;
}

// ============================================================================
// Fleet Movement Validation & Actions
// ============================================================================

ErrorCode game_can_player_move_fleet(
	const GameState* game_state,
	uint32_t player_id,
	uint32_t fleet_id,
	uint32_t destination_planet_id)
{
	if (!game_state)
		return ErrorCode::INVALID_PARAMETER;
	
	return game_state->check_player_move_fleet(player_id, fleet_id, destination_planet_id);
}

ErrorCode game_player_move_fleet(
	GameState* game_state,
	uint32_t player_id,
	uint32_t fleet_id,
	uint32_t destination_planet_id)
{
	if (!game_state)
		return ErrorCode::INVALID_PARAMETER;
	
	// Validate first
	if (game_state->check_player_move_fleet(player_id, fleet_id, destination_planet_id) != ErrorCode::SUCCESS)
		return ErrorCode::VALIDATION_FAILED;
	
	// Delegate to GameState
	game_state->move_fleet(player_id, fleet_id, destination_planet_id);
	return ErrorCode::SUCCESS;
}

// ============================================================================
// Planet Allocation Validation & Actions
// ============================================================================

ErrorCode game_can_player_set_planet_allocation(
	const GameState* game_state,
	uint32_t player_id,
	uint32_t planet_id,
	double mining_fraction,
	double terraforming_fraction)
{
	if (!game_state)
		return ErrorCode::INVALID_PARAMETER;
	
	return game_state->check_player_set_planet_allocation(player_id, planet_id, mining_fraction, terraforming_fraction);
}

ErrorCode game_player_set_planet_allocation(
	GameState* game_state,
	uint32_t player_id,
	uint32_t planet_id,
	double mining_fraction,
	double terraforming_fraction)
{
	if (!game_state)
		return ErrorCode::INVALID_PARAMETER;
	
	// Validate first
	if (game_state->check_player_set_planet_allocation(player_id, planet_id, mining_fraction, terraforming_fraction) != ErrorCode::SUCCESS)
		return ErrorCode::INVALID_ALLOCATION;
	
	// TODO: Implement actual allocation setting in GameState
	return ErrorCode::SUCCESS;
}
