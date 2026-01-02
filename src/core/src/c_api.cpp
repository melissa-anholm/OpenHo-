#include "game.h"
#include "openho_core.h"
#include "game_setup.h"
#include <cstring>
#include <unordered_set>

// ============================================================================
// C API Wrapper Functions for Objective-C++ Bridging
// ============================================================================

// ============================================================================
// Game Initialization and Management
// ============================================================================

void* game_create()
{
	try
	{
		GameSetup setup;
		GameState* game = setup.create_new_game();
		return static_cast<void*>(game);
	}
	catch (const std::exception& e)
	{
		// In production, log the error
		return nullptr;
	}
}

void game_destroy(void* game)
{
	if (!game)
		return;
	
	GameState* gameState = static_cast<GameState*>(game);
	delete gameState;
}

// ============================================================================
// Game State Queries
// ============================================================================

uint32_t game_get_current_turn(void* game)
{
	if (!game)
		{ return 0; }
	
	GameState* gameState = static_cast<GameState*>(game);
	return gameState->get_current_turn();
}

uint32_t game_get_num_players(void* game)
{
	if (!game)
		{ return 0; }
	
	GameState* gameState = static_cast<GameState*>(game);
	return gameState->get_num_players();
}

uint32_t game_get_num_planets(void* game)
{
	if (!game)
		{ return 0; }
	
	GameState* gameState = static_cast<GameState*>(game);
	return static_cast<uint32_t>(gameState->get_galaxy().planets.size());
}

uint32_t game_get_num_fleets(void* game)
{
	if (!game)
		{ return 0; }
	
	GameState* gameState = static_cast<GameState*>(game);
	
	uint32_t total_fleets = 0;
	for (const auto& player : gameState->get_players())
	{
		total_fleets += static_cast<uint32_t>(player.get_fleets().size());
	}
	
	return total_fleets;
}

// ============================================================================
// Player Queries
// ============================================================================
void game_get_player(void* game, uint32_t player_id, Player* out)
{
	if (!game || !out)
		{ return; }
	
	GameState* gameState = static_cast<GameState*>(game);
	const Player* player = gameState->get_player(player_id);
	
	if (player)
		{ *out = *player; }
	else
	{
		// Return a zeroed-out player structure
		std::memset(out, 0, sizeof(Player));
	}
}

void game_get_player_money_income(void* game, uint32_t player_id, int64_t* out)
{
	if (!game || !out)
		{ return; }
	
	GameState* gameState = static_cast<GameState*>(game);
	*out = gameState->get_player_money_income(player_id);
}

void game_get_player_metal_income(void* game, uint32_t player_id, int64_t* out)
{
	if (!game || !out)
		{ return; }
	
	GameState* gameState = static_cast<GameState*>(game);
	*out = gameState->get_player_metal_income(player_id);
}

void game_get_player_money_reserve(void* game, uint32_t player_id, int64_t* out)
{
	if (!game || !out)
		{ return; }
	
	GameState* gameState = static_cast<GameState*>(game);
	*out = gameState->get_player_money(player_id);
}

void game_get_player_metal_reserve(void* game, uint32_t player_id, int64_t* out)
{
	if (!game || !out)
		{ return; }
	
	GameState* gameState = static_cast<GameState*>(game);
	*out = gameState->get_player_metal_reserve(player_id);
}

// ============================================================================
// Planet Queries
// ============================================================================
void game_get_planet(void* game, uint32_t planetID, Planet* out)
{
	if (!game || !out)
		{ return; }
	
	GameState* gameState = static_cast<GameState*>(game);
	const Planet* planet = gameState->get_planet(planetID);
	
	if (planet)
		{ *out = *planet; }
	else
	{
		// Return a zeroed-out planet structure
		std::memset(out, 0, sizeof(Planet));
	}
}

void game_get_planet_perceived_values(void* game, uint32_t planetID, uint32_t player_id,
									   double* outTemp, double* outGravity)
{
	if (!game || !outTemp || !outGravity)
		{ return; }
	
	GameState* gameState = static_cast<GameState*>(game);
	const Planet* planet = gameState->get_planet(planetID);
	const Player* player = gameState->get_player(player_id);
	
	if (!planet || !player)
	{
		*outTemp = 0.0;
		*outGravity = 0.0;
		return;
	}
	
	// Perceived values are based on the player's ideal conditions
	// The formula creates a "happiness" or "desirability" score
	// Values range from 0 (worst) to 1 (best match for player's ideals)
	
	// Temperature perception: how close to ideal temperature
	double idealTemp = gameState->get_player_ideal_temperature(player_id);
	double tempDiff = std::abs(planet->true_temperature - idealTemp);
	*outTemp = std::max(0.0, 1.0 - tempDiff / 100.0);
	
	// Gravity perception: how close to ideal gravity
	double idealGrav = gameState->get_player_ideal_gravity(player_id);
	double gravDiff = std::abs(planet->true_gravity - idealGrav);
	*outGravity = std::max(0.0, 1.0 - gravDiff / 2.0);
}

// ============================================================================
// Player Public Information Queries
// ============================================================================
void game_get_full_player_info_history(void* game, uint32_t player_id, const std::vector<PlayerPublicInfo>** out)
	{
		if (!game || !out)
			{ return; }
		
		GameState* gameState = static_cast<GameState*>(game);
		*out = &gameState->get_full_player_info_history(player_id);
}

	uint32_t game_get_player_info_history_size(void* game, uint32_t player_id)
	{
		if (!game)
			{ return 0; }
		
		GameState* gameState = static_cast<GameState*>(game);
		const auto& history = gameState->get_full_player_info_history(player_id);
		return static_cast<uint32_t>(history.size());
}

// ============================================================================
// Money Allocation
// ============================================================================
void game_set_money_allocation(void* game, uint32_t player_id, const Player::MoneyAllocation* money_alloc)
{
	if (!game || !money_alloc)
		{ return; }
	
	GameState* gameState = static_cast<GameState*>(game);
	
	try
	{
		gameState->set_money_allocation(player_id, *money_alloc);
	}
	catch (const std::exception& e)
	{
		// In production, log the error
	}
}

void game_get_money_allocation(void* game, uint32_t player_id, Player::MoneyAllocation* out)
{
	if (!game || !out) 
		{ return; }
	
	GameState* gameState = static_cast<GameState*>(game);
	
	try
	{
		const Player::MoneyAllocation& alloc = gameState->get_money_allocation(player_id);
		*out = alloc;
	}
	catch (const std::exception& e)
	{
		// In production, log the error
		// Return a zeroed-out allocation
		std::memset(out, 0, sizeof(Player::MoneyAllocation));
	}
}

// ============================================================================
// AI RNG Seed Management
// ============================================================================
uint64_t game_get_ai_rng_seed(void* game)
{
	if (!game) 
		{ return 0; }
	
	GameState* gameState = static_cast<GameState*>(game);
	return gameState->get_ai_rng_seed();
}

void game_set_ai_rng_seed(void* game, uint64_t seed)
{
	if (!game)
		{ return; }
	
	GameState* gameState = static_cast<GameState*>(game);
	gameState->set_ai_rng_seed(seed);
}

// ============================================================================
// Turn Processing
// ============================================================================
void game_process_turn(void* game)
{
	if (!game)
		{ return; }
	
	GameState* gameState = static_cast<GameState*>(game);
	gameState->process_turn();
}

// ============================================================================
// Serialization
// ============================================================================
int game_serialize_state(void* game, void* buffer, int bufferSize)
{
	if (!game || !buffer || bufferSize <= 0)
		{ return -1; }
	
	GameState* gameState = static_cast<GameState*>(game);
	
	try
	{
		std::vector<uint8_t> serialized = gameState->serialize_state();
		
		if (static_cast<int>(serialized.size()) > bufferSize)
		{
			return -1; // Buffer too small
		}
		
		std::memcpy(buffer, serialized.data(), serialized.size());
		return static_cast<int>(serialized.size());
	}
	catch (const std::exception& e)
	{
		// In production, log the error
		return -1;
	}
}

int game_deserialize_state(void* game, const void* buffer, int bufferSize)
{
	if (!game || !buffer || bufferSize <= 0)
		{ return -1; }
	
	GameState* gameState = static_cast<GameState*>(game);
	
	try
	{
		std::vector<uint8_t> data(static_cast<const uint8_t*>(buffer),
								  static_cast<const uint8_t*>(buffer) + bufferSize);
		
		if (gameState->deserialize_state(data))
			{ return bufferSize; }
		else
			{ return -1; }
	}
	catch (const std::exception& e)
	{
		// In production, log the error
		return -1;
	}
}

int game_get_serialized_state_size(void* game)
{
	if (!game) 
		{ return -1; }
	
	GameState* gameState = static_cast<GameState*>(game);
	
	try
	{
		std::vector<uint8_t> serialized = gameState->serialize_state();
		return static_cast<int>(serialized.size());
	}
	catch (const std::exception& e)
	{
		// In production, log the error
		return -1;
	}
}

// ============================================================================
// Ship Design Queries and Management
// ============================================================================
uint32_t game_create_ship_design(void* game, uint32_t player_id, const char* name, ShipType type, 
                                 int32_t tech_range, int32_t tech_speed, int32_t tech_weapons, 
                                 int32_t tech_shields, int32_t tech_miniaturization)
{
	if (!game || !name) 
		{ return 0; }
	
	GameState* gameState = static_cast<GameState*>(game);
	return gameState->create_ship_design(player_id, std::string(name), type, tech_range, tech_speed, 
	                                    tech_weapons, tech_shields, tech_miniaturization);
}

void game_get_ship_design(void* game, uint32_t player_id, uint32_t design_id, ShipDesign* out)
{
	if (!game || !out)
		{ return; }
	
	GameState* gameState = static_cast<GameState*>(game);
	const ShipDesign* design = gameState->get_ship_design(player_id, design_id);
	
	if (design)
		{ *out = *design; }
	else
		{ std::memset(out, 0, sizeof(ShipDesign)); }
}

void game_get_player_ship_designs(void* game, uint32_t player_id, ShipDesign* out, uint32_t maxCount, uint32_t* outCount)
{
	if (!game || !out || !outCount) 
		{ return; }
	
	GameState* gameState = static_cast<GameState*>(game);
	const auto& designs = gameState->get_player_ship_designs(player_id);
	
	uint32_t count = std::min(static_cast<uint32_t>(designs.size()), maxCount);
	for (uint32_t i = 0; i < count; ++i)
	{
		out[i] = designs[i];
	}
	
	*outCount = count;
}

uint32_t game_get_num_player_ship_designs(void* game, uint32_t player_id)
{
	if (!game) 
		{ return 0; }
	
	GameState* gameState = static_cast<GameState*>(game);
	return static_cast<uint32_t>(gameState->get_player_ship_designs(player_id).size());
}

void game_delete_ship_design(void* game, uint32_t player_id, uint32_t design_id)
{
	if (!game) 
		{ return; }
	
	GameState* gameState = static_cast<GameState*>(game);
	(void)gameState->delete_ship_design(player_id, design_id);  // Ignore return value
}

void game_build_ship_from_design(void* game, uint32_t player_id, uint32_t design_id)
{
	if (!game) 
		{ return; }
	
	GameState* gameState = static_cast<GameState*>(game);
	gameState->build_ship_from_design(player_id, design_id);
}
