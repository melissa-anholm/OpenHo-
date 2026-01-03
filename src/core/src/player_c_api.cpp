#include "player_c_api.h"
#include "player.h"
#include "ship_design.h"
#include "fleet.h"
#include "planet.h"

// ============================================================================
// Player Accessors (Read-Only)
// ============================================================================

double player_get_ideal_gravity(const Player* player)
{
	if (!player)
		return -1.0;
	return player->get_ideal_gravity();
}

double player_get_ideal_temperature(const Player* player)
{
	if (!player)
		return -1.0;
	return player->get_ideal_temperature();
}

int64_t player_get_money(const Player* player)
{
	if (!player)
		return -1;
	return player->get_money();
}

int64_t player_get_metal(const Player* player)
{
	if (!player)
		return -1;
	return player->get_metal();
}

int64_t player_get_money_income(const Player* player)
{
	if (!player)
		return -1;
	return player->get_money_income();
}

int64_t player_get_metal_income(const Player* player)
{
	if (!player)
		return -1;
	return player->get_metal_income();
}

ErrorCode player_get_spending_allocation(
	const Player* player,
	double* out_savings_fraction,
	double* out_research_fraction,
	double* out_planets_fraction)
{
	if (!player || !out_savings_fraction || !out_research_fraction || !out_planets_fraction)
		return ErrorCode::INVALID_PARAMETER;
	
	const auto& alloc = player->get_spending_allocation();
	*out_savings_fraction = alloc.savings_fraction;
	*out_research_fraction = alloc.research_fraction;
	*out_planets_fraction = alloc.planets_fraction;
	
	return ErrorCode::SUCCESS;
}

// ============================================================================
// Player Spending Allocation
// ============================================================================

ErrorCode player_set_spending_allocation(
	Player* player,
	double savings_fraction,
	double research_fraction,
	double planets_fraction)
{
	if (!player)
		return ErrorCode::INVALID_PARAMETER;
	
	// TODO: Implement validation and setting
	return ErrorCode::SUCCESS;
}

// ============================================================================
// Ship Design Management
// ============================================================================

uint32_t player_get_ship_design_count(const Player* player)
{
	if (!player)
		return 0;
	return player->get_ship_designs().size();
}

const ShipDesign* player_get_ship_design(const Player* player, uint32_t design_id)
{
	if (!player)
		return nullptr;
	return player->get_ship_design(design_id);
}

const ShipDesign* const* player_get_all_ship_designs(const Player* player)
{
	if (!player)
		return nullptr;
	// TODO: Return array of pointers (requires conversion from vector)
	return nullptr;
}

uint32_t player_design_ship(
	Player* player,
	const char* name,
	uint32_t ship_type,
	int32_t tech_range,
	int32_t tech_speed,
	int32_t tech_weapons,
	int32_t tech_shields,
	int32_t tech_miniaturization)
{
	if (!player || !name)
		return 0;
	
	// TODO: Implement with validation
	return player->create_ship_design(
		std::string(name),
		static_cast<ShipType>(ship_type),
		tech_range,
		tech_speed,
		tech_weapons,
		tech_shields,
		tech_miniaturization
	);
}

ErrorCode player_delete_ship_design(Player* player, uint32_t design_id)
{
	if (!player)
		return ErrorCode::INVALID_PLAYER_ID;
	
	if (!player->delete_ship_design(design_id))
		return ErrorCode::SHIP_DESIGN_NOT_FOUND;
	
	return ErrorCode::SUCCESS;
}

// ============================================================================
// Fleet Management
// ============================================================================

uint32_t player_get_fleet_count(const Player* player)
{
	if (!player)
		return 0;
	return player->get_fleets().size();
}

const Fleet* player_get_fleet(const Player* player, uint32_t fleet_id)
{
	if (!player)
		return nullptr;
	return player->get_fleet(fleet_id);
}

const Fleet* const* player_get_all_fleets(const Player* player)
{
	if (!player)
		return nullptr;
	// TODO: Return array of pointers (requires conversion from vector)
	return nullptr;
}

uint32_t player_build_fleet(
	Player* player,
	uint32_t design_id,
	uint32_t ship_count,
	uint32_t planet_id)
{
	if (!player)
		return 0;
	
	// TODO: Implement with validation
	// Note: Fleet ID allocation happens in Player::create_fleet via GameState
	return 0;
}

ErrorCode player_delete_fleet(Player* player, uint32_t fleet_id)
{
	if (!player)
		return ErrorCode::INVALID_PLAYER_ID;
	
	if (!player->delete_fleet(fleet_id))
		return ErrorCode::FLEET_NOT_FOUND;
	
	return ErrorCode::SUCCESS;
}

ErrorCode player_move_fleet(Player* player, uint32_t fleet_id, uint32_t destination_planet_id)
{
	if (!player)
		return ErrorCode::INVALID_PLAYER_ID;
	
	// TODO: Implement with validation
	player->move_fleet(fleet_id, destination_planet_id);
	return ErrorCode::SUCCESS;
}

ErrorCode player_refuel_fleet(Player* player, uint32_t fleet_id)
{
	if (!player)
		return ErrorCode::INVALID_PLAYER_ID;
	
	// TODO: Implement with validation
	return ErrorCode::SUCCESS;
}

// ============================================================================
// Colonized Planet Management
// ============================================================================

uint32_t player_get_colonized_planet_count(const Player* player)
{
	if (!player)
		return 0;
	return player->get_colonized_planets().size();
}

const ColonizedPlanet* player_get_colonized_planet(const Player* player, uint32_t index)
{
	if (!player || index >= player->get_colonized_planets().size())
		return nullptr;
	return &player->get_colonized_planets()[index];
}

const ColonizedPlanet* const* player_get_all_colonized_planets(const Player* player)
{
	if (!player)
		return nullptr;
	// TODO: Return array of pointers (requires conversion from vector)
	return nullptr;
}

ErrorCode player_set_planet_allocation(
	Player* player,
	uint32_t planet_id,
	double mining_fraction,
	double terraforming_fraction)
{
	if (!player)
		return ErrorCode::INVALID_PLAYER_ID;
	
	// TODO: Implement with validation
	return ErrorCode::SUCCESS;
}

// ============================================================================
// Technology & Research (Read-Only)
// ============================================================================

int32_t player_get_tech_level(const Player* player, uint32_t tech_type)
{
	if (!player || tech_type > 5)
		return -1;
	
	return player->get_tech_level(tech_type);
}

double player_get_research_allocation(const Player* player, uint32_t tech_type)
{
	if (!player || tech_type > 5)
		return -1.0;
	
	const auto& alloc = player->get_spending_allocation().research;
	switch (tech_type)
	{
		case 0:  // TECH_RANGE
			return alloc.research_range_fraction;
		case 1:  // TECH_SPEED
			return alloc.research_speed_fraction;
		case 2:  // TECH_WEAPONS
			return alloc.research_weapons_fraction;
		case 3:  // TECH_SHIELDS
			return alloc.research_shields_fraction;
		case 4:  // TECH_MINI
			return alloc.research_mini_fraction;
		case 5:  // TECH_RADICAL
			return alloc.research_radical_fraction;
		default:
			return -1.0;
	}
}
