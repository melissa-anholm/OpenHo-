#ifndef OPENHO_CORE_H
#define OPENHO_CORE_H

#include <cstdint>
#include <vector>
#include <string>

// ============================================================================
// Fundamental Types
// ============================================================================

// Unique identifiers for game entities
typedef uint32_t EntityID;
typedef uint32_t PlayerID;

// Coordinate types
typedef double GalaxyCoord;

// ============================================================================
// Enums
// ============================================================================

// Ship types
enum ShipType
{
	SHIP_SCOUT = 0,
	SHIP_FIGHTER = 1,
	SHIP_COLONY = 2,
	SHIP_SATELLITE = 3,
	SHIP_TANKER = 4,
	SHIP_DREADNOUGHT = 5,
	SHIP_BIOLOGICAL = 6
};

// Planet nova state
enum PlanetState
{
	PLANET_NORMAL = 0,
	PLANET_WARNING = 1,
	PLANET_DESTROYED = 2
};

// Player type (human or computer)
enum PlayerType
{
	PLAYER_HUMAN = 0,
	PLAYER_COMPUTER = 1
};

// Technology streams for research
enum TechStream
{
	TECH_RANGE = 0,
	TECH_SPEED = 1,
	TECH_WEAPONS = 2,
	TECH_SHIELDS = 3,
	TECH_MINIATURIZATION = 4,
	TECH_RADICAL = 5
};

// ============================================================================
// Include Domain-Specific Headers
// ============================================================================

#include "planet.h"
#include "ship.h"
#include "player.h"
#include "galaxy.h"

// ============================================================================
// C API for Objective-C++ Bridging
// ============================================================================

#ifdef __cplusplus
extern "C"
{
#endif

// Game initialization and management
void* game_create(const GalaxyGenerationParams* params);
void game_destroy(void* game);

// Game state queries
uint32_t game_get_current_turn(void* game);
uint32_t game_get_num_players(void* game);
uint32_t game_get_num_planets(void* game);
uint32_t game_get_num_ships(void* game);

// Player queries
void game_get_player(void* game, uint32_t player_id, Player* out);
int64_t game_get_player_money_income(void* game, uint32_t player_id);
int64_t game_get_player_metal_income(void* game, uint32_t player_id);
int64_t game_get_player_money_reserve(void* game, uint32_t player_id);
int64_t game_get_player_metal_reserve(void* game, uint32_t player_id);

// Planet queries
void game_get_planet(void* game, uint32_t planet_id, Planet* out);
void game_get_planet_perceived_values(void* game, uint32_t planet_id, uint32_t player_id, double* out_temp, double* out_grav);

// Ship queries
void game_get_ship(void* game, uint32_t ship_id, Ship* out);

// Fleet queries
uint32_t game_get_num_fleets(void* game);

// Player public information queries
void game_get_player_public_info(void* game, uint32_t player_id, uint32_t turn, PlayerPublicInfo* out);
void game_get_player_public_info_current(void* game, uint32_t player_id, PlayerPublicInfo* out);
uint32_t game_get_player_info_history_size(void* game, uint32_t player_id);

// Ship design queries and management
uint32_t game_create_ship_design(void* game, uint32_t player_id, const char* name, ShipType type, int32_t tech_range, int32_t tech_speed, int32_t tech_weapons, int32_t tech_shields, int32_t tech_miniaturization);
void game_get_ship_design(void* game, uint32_t player_id, uint32_t design_id, ShipDesign* out);
void game_get_player_ship_designs(void* game, uint32_t player_id, ShipDesign* out, uint32_t max_count, uint32_t* out_count);
uint32_t game_get_num_player_ship_designs(void* game, uint32_t player_id);
void game_delete_ship_design(void* game, uint32_t player_id, uint32_t design_id);
void game_build_ship_from_design(void* game, uint32_t player_id, uint32_t design_id);

// Money allocation
void game_set_money_allocation(void* game, uint32_t player_id, const Player::MoneyAllocation* alloc);
void game_get_money_allocation(void* game, uint32_t player_id, Player::MoneyAllocation* out);

// AI RNG seed management
uint64_t game_get_ai_rng_seed(void* game);
void game_set_ai_rng_seed(void* game, uint64_t seed);

// Turn processing
void game_process_turn(void* game);

// Serialization
int game_serialize_state(void* game, void* buffer, int buffer_size);
int game_deserialize_state(void* game, const void* buffer, int buffer_size);
int game_get_serialized_state_size(void* game);

#ifdef __cplusplus
}
#endif

#endif // OPENHO_CORE_H
