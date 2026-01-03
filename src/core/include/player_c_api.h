#ifndef OPENHO_PLAYER_C_API_H
#define OPENHO_PLAYER_C_API_H

#include "error_codes.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct Player Player;
typedef struct ShipDesign ShipDesign;
typedef struct Fleet Fleet;
typedef struct ColonizedPlanet ColonizedPlanet;

// ============================================================================
// Player Accessors (Read-Only)
// ============================================================================

/**
 * Get player's ideal gravity preference
 */
double player_get_ideal_gravity(const Player* player);

/**
 * Get player's ideal temperature preference
 */
double player_get_ideal_temperature(const Player* player);

/**
 * Get player's current money savings
 */
int64_t player_get_money(const Player* player);

/**
 * Get player's current metal reserve
 */
int64_t player_get_metal(const Player* player);

/**
 * Get player's money income per turn
 */
int64_t player_get_money_income(const Player* player);

/**
 * Get player's metal income per turn
 */
int64_t player_get_metal_income(const Player* player);

/**
 * Get player's current spending allocation
 * Returns: 0 on success, error code otherwise
 */
ErrorCode player_get_spending_allocation(
	const Player* player,
	double* out_savings_fraction,
	double* out_research_fraction,
	double* out_planets_fraction
);

// ============================================================================
// Player Spending Allocation
// ============================================================================

/**
 * Set player's spending allocation
 * Fractions should sum to approximately 1.0
 * Returns: 0 on success, error code otherwise
 */
ErrorCode player_set_spending_allocation(
	Player* player,
	double savings_fraction,
	double research_fraction,
	double planets_fraction
);

// ============================================================================
// Ship Design Management
// ============================================================================

/**
 * Get number of ship designs owned by player
 */
uint32_t player_get_ship_design_count(const Player* player);

/**
 * Get ship design by ID
 * Returns: pointer to ShipDesign, or NULL if not found
 */
const ShipDesign* player_get_ship_design(const Player* player, uint32_t design_id);

/**
 * Get all ship designs (array of ShipDesign pointers)
 * Returns: array of pointers, caller must not free
 * Use player_get_ship_design_count() to get array length
 */
const ShipDesign* const* player_get_all_ship_designs(const Player* player);

/**
 * Create a new ship design
 * Returns: design_id on success, 0 on error
 */
uint32_t player_design_ship(
	Player* player,
	const char* name,
	uint32_t ship_type,  // ShipType enum value
	int32_t tech_range,
	int32_t tech_speed,
	int32_t tech_weapons,
	int32_t tech_shields,
	int32_t tech_miniaturization
);

/**
 * Delete a ship design
 * Returns: error code
 */
ErrorCode player_delete_ship_design(Player* player, uint32_t design_id);

// ============================================================================
// Fleet Management
// ============================================================================

/**
 * Get number of fleets owned by player
 */
uint32_t player_get_fleet_count(const Player* player);

/**
 * Get fleet by ID
 * Returns: pointer to Fleet, or NULL if not found
 */
const Fleet* player_get_fleet(const Player* player, uint32_t fleet_id);

/**
 * Get all fleets (array of Fleet pointers)
 * Returns: array of pointers, caller must not free
 * Use player_get_fleet_count() to get array length
 */
const Fleet* const* player_get_all_fleets(const Player* player);

/**
 * Create a new fleet (validation should be done via GameState first)
 * Returns: fleet_id on success, 0 on error
 */
uint32_t player_build_fleet(
	Player* player,
	uint32_t design_id,
	uint32_t ship_count,
	uint32_t planet_id
);

/**
 * Delete a fleet
 * Returns: error code
 */
ErrorCode player_delete_fleet(Player* player, uint32_t fleet_id);

/**
 * Move a fleet to destination planet
 * Returns: error code
 */
ErrorCode player_move_fleet(Player* player, uint32_t fleet_id, uint32_t destination_planet_id);

/**
 * Refuel a fleet (at current planet)
 * Returns: error code
 */
ErrorCode player_refuel_fleet(Player* player, uint32_t fleet_id);

// ============================================================================
// Colonized Planet Management
// ============================================================================

/**
 * Get number of colonized planets owned by player
 */
uint32_t player_get_colonized_planet_count(const Player* player);

/**
 * Get colonized planet by index (not planet_id!)
 * Returns: pointer to ColonizedPlanet, or NULL if index out of range
 */
const ColonizedPlanet* player_get_colonized_planet(const Player* player, uint32_t index);

/**
 * Get all colonized planets (array of ColonizedPlanet pointers)
 * Returns: array of pointers, caller must not free
 * Use player_get_colonized_planet_count() to get array length
 */
const ColonizedPlanet* const* player_get_all_colonized_planets(const Player* player);

/**
 * Set planet's allocation (mining vs terraforming)
 * Returns: error code
 */
ErrorCode player_set_planet_allocation(
	Player* player,
	uint32_t planet_id,
	double mining_fraction,
	double terraforming_fraction
);

// ============================================================================
// Technology & Research (Read-Only)
// ============================================================================

/**
 * Get player's technology level for a specific tech type
 * tech_type: 0=range, 1=speed, 2=weapons, 3=shields, 4=mini, 5=radical
 * Returns: technology level, or -1 if invalid tech_type
 */
int32_t player_get_tech_level(const Player* player, uint32_t tech_type);

/**
 * Get player's research allocation for a specific tech type
 * tech_type: 0=range, 1=speed, 2=weapons, 3=shields, 4=mini, 5=radical
 * Returns: allocation fraction (0.0 to 1.0), or -1.0 if invalid tech_type
 */
double player_get_research_allocation(const Player* player, uint32_t tech_type);

#ifdef __cplusplus
}
#endif

#endif // OPENHO_PLAYER_C_API_H
