
#include "openho_core.h"
#include "player.h"
#include "game.h"
#include <cmath>

// ============================================================================
// Money Allocation Calculation Helpers
// ============================================================================

int64_t Player::calculate_savings_amount(const MoneyAllocation& money_alloc, int64_t player_income)
{
	double savings_amount = player_income * money_alloc.savings_fraction;
	return static_cast<int64_t>(std::round(savings_amount));
}

int64_t Player::calculate_research_amount(const MoneyAllocation& money_alloc, int64_t player_income)
{
	double research_amount = player_income * money_alloc.research_fraction;
	return static_cast<int64_t>(std::round(research_amount));
}

int64_t Player::calculate_planets_amount(double total_planets_fraction, int64_t player_income)
{
	double planets_amount = player_income * total_planets_fraction;
	return static_cast<int64_t>(std::round(planets_amount));
}

int64_t Player::calculate_research_stream_amount( const ResearchAllocation& player_research,
                                                  TechStream tech_type,
                                                  int64_t research_budget)
{
	double stream_fraction = 0.0;
	
	switch (tech_type)
	{
		case TECH_RANGE:
			stream_fraction = player_research.research_range_fraction;
			break;
		case TECH_SPEED:
			stream_fraction = player_research.research_speed_fraction;
			break;
		case TECH_WEAPONS:
			stream_fraction = player_research.research_weapons_fraction;
			break;
		case TECH_SHIELDS:
			stream_fraction = player_research.research_shields_fraction;
			break;
		case TECH_MINI:
			stream_fraction = player_research.research_mini_fraction;
			break;
		case TECH_RADICAL:
			stream_fraction = player_research.research_radical_fraction;
			break;
		default:
			return 0;
	}
	
	double tech_stream_amount = research_budget * stream_fraction;
	return static_cast<int64_t>(std::round(tech_stream_amount));
}


// ============================================================================
// Fleet Management
// ============================================================================

bool Player::validate_fleet(uint32_t design_id, uint32_t ship_count, uint32_t planet_id) const
{
	// Validate ship_count is positive
	if (ship_count == 0)
		return false;
	
	// Validate design exists for this player
	const ShipDesign* design = get_ship_design(design_id);
	if (!design)
		return false;
	
	// Validate planet exists through GameState
	const Planet* planet = game_state->get_planet(planet_id);
	if (!planet)
		return false;
	
	return true;
}

uint32_t Player::build_fleet(uint32_t fleet_id, uint32_t design_id, uint32_t ship_count, uint32_t planet_id)
{
	// Get the design and planet pointers
	const ShipDesign* design = get_ship_design(design_id);
	if (!design)
		return 0;
	
	Planet* planet = game_state->get_planet(planet_id);
	if (!planet)
		return 0;
	
	// Create fleet using private constructor (Player is a friend of Fleet)
	Fleet new_fleet(fleet_id, id, design, ship_count, planet);
	
	fleets.push_back(new_fleet);
	return fleet_id;
}

uint32_t Player::create_fleet(uint32_t design_id, uint32_t ship_count, uint32_t planet_id)
{
	// Validate parameters
	if (!validate_fleet(design_id, ship_count, planet_id))
		return 0;
	
	// Allocate a globally unique fleet ID from GameState
	uint32_t fleet_id = game_state->allocate_fleet_id();
	
	// Build and add the fleet
	return build_fleet(fleet_id, design_id, ship_count, planet_id);
}

Fleet* Player::get_fleet(uint32_t fleet_id)
{
	for (auto& fleet : fleets)
	{
		if (fleet.id == fleet_id)
			return &fleet;
	}
	return nullptr;
}
const Fleet* Player::get_fleet(uint32_t fleet_id) const
{
	for (const auto& fleet : fleets)
	{
		if (fleet.id == fleet_id)
			return &fleet;
	}
	return nullptr;
}

bool Player::delete_fleet(uint32_t fleet_id)
{
	for (size_t i = 0; i < fleets.size(); ++i)
	{
		if (fleets[i].id == fleet_id)
		{
			fleets.erase(fleets.begin() + i);
			return true;
		}
	}
	return false;
}

void Player::move_fleet(uint32_t fleet_id, uint32_t destination_planet_id)
{
	// TODO: Validate destination_planet_id is valid
	// TODO: Calculate distance and travel time
	// TODO: Check fuel availability
	
	Fleet* fleet = get_fleet(fleet_id);
	if (!fleet)
		return;
	
	// NOTE: This is a placeholder. GameState::move_fleet will handle the actual
	// planet lookup and distance calculations. This just marks the fleet as needing movement.
	fleet->in_transit = true;
	// TODO: Store destination_planet_id for GameState to look up the actual Planet pointer
}


// ============================================================================
// Ship Design Management
// ============================================================================
uint32_t Player::create_ship_design(const std::string& name, ShipType type, int32_t tech_range, int32_t tech_speed, int32_t tech_weapons, int32_t tech_shields, int32_t tech_mini)
{
	// TODO: Validate tech levels are within reasonable bounds
	// TODO: Check max 100 designs per player
	
	uint32_t design_id = next_ship_design_id++;
	ShipDesign design;
	design.id = design_id;
	design.name = name;
	design.type = type;
	
	// Set tech levels using the friend function
	set_ship_design_tech(design, tech_range, tech_speed, tech_weapons, tech_shields, tech_mini);
	
	ship_designs.push_back(design);
	return design_id;
}

const ShipDesign* Player::get_ship_design(uint32_t design_id) const
{
	for (const auto& design : ship_designs)
	{
		if (design.id == design_id)
			{ return &design; }
	}
	return nullptr;
}

bool Player::delete_ship_design(uint32_t design_id)
{
	// TODO: Check if any fleets use this design before deleting
	
	for (size_t i = 0; i < ship_designs.size(); ++i)
	{
		if (ship_designs[i].id == design_id)
		{
			ship_designs.erase(ship_designs.begin() + i);
			return true;
		}
	}
	return false;
}

// ============================================================================
// Player Info History Management
// ============================================================================

void Player::receive_player_public_info(uint32_t source_player_id, const PlayerPublicInfo& info)
{
	// Append the new info to the history for this player
	player_info_history[source_player_id].push_back(info);
}

const std::vector<PlayerPublicInfo>* Player::get_player_info_history(uint32_t player_id) const
{
	auto it = player_info_history.find(player_id);
	if (it != player_info_history.end())
	return &it->second;
	
	return nullptr;
}

bool Player::validate_player_info_history(uint32_t player_id) const
{
	auto it = player_info_history.find(player_id);
	if (it == player_info_history.end())
		{ return false; }  // No history for this player

	const auto& history = it->second;
	if (history.empty())
		{ return false; }  // Empty history
	
	// Check for consistency: each entry should have sequential years
	for (size_t i = 1; i < history.size(); ++i)
	{
		// Year should be sequential
		if (history[i].year != history[i-1].year + 1)
			{ return false; }
		
		// Player ID should be consistent
		if (history[i].player_id != history[i-1].player_id)
			{ return false; }
	}
	return true;
}

void Player::request_full_player_info_resync(uint32_t player_id)
{
	// This is a placeholder - in a real implementation, this would
	// communicate with GameState to request a full resync
	// For now, we just clear the history for this player
	player_info_history[player_id].clear();
}


// ============================================================================
// Technology Accessors
// ============================================================================

int32_t Player::get_tech_level(uint32_t tech_type) const
{
	switch (tech_type)
	{
		case 0:  // TECH_RANGE
			return tech.range;
		case 1:  // TECH_SPEED
			return tech.speed;
		case 2:  // TECH_WEAPONS
			return tech.weapons;
		case 3:  // TECH_SHIELDS
			return tech.shields;
		case 4:  // TECH_MINI
			return tech.mini;
		case 5:  // TECH_RADICAL
			return tech.radical;
		default:
			return -1;  // Invalid tech type
	}
}
