#include "knowledge_planet.h"
#include "planet.h"
#include "player.h"
#include "game_formulas.h"
#include <algorithm>

// ============================================================================
// KnowledgePlanet Implementation
// ============================================================================

KnowledgePlanet::KnowledgePlanet(const Planet& planet, PlayerID player_id)
	: id(planet.id),
	  name(planet.name),
	  x(planet.x),
	  y(planet.y),
	  as_seen_by(player_id),
	  nova_state(PLANET_NORMAL)
{
	// Initialize observable fields with unknown values
	apparent_temperature = UNKNOWN_DOUBLE_VALUE;
	apparent_gravity = UNKNOWN_DOUBLE_VALUE;
	metal = UNKNOWN_INT_VALUE;
	apparent_owner = OWNER_UNKNOWN;
	apparent_population = POPULATION_UNKNOWN;
	observation_year = OBSERVATION_YEAR_UNKNOWN;
	can_be_profitable = PROFITABILITY_UNKNOWN;
	perceived_value = PERCEIVED_VALUE_UNKNOWN;
}

void KnowledgePlanet::observe_planet(const Planet& planet, const Player* observer, int32_t current_year)
{
	// Update observable fields based on current planet state
	// Note: nova_state is NOT updated by this method
	
	// Calculate apparent values based on the observing player's ideals
	apparent_temperature = GameFormulas::calculate_apparent_temperature(
		observer->get_ideal_temperature(), planet.true_temperature);
	apparent_gravity = GameFormulas::calculate_apparent_gravity(
		observer->get_ideal_gravity(), planet.true_gravity);
	
	// Known fields from planet
	metal = planet.metal;
	apparent_owner = planet.owner;
	apparent_population = planet.population;  // Copy actual population
	
	// Unknown fields (not available from Planet)
	observation_year = current_year;
	can_be_profitable = PROFITABILITY_UNKNOWN;
	perceived_value = PERCEIVED_VALUE_UNKNOWN;
}


// ============================================================================
// Fleet Management Methods
// ============================================================================

void KnowledgePlanet::add_my_fleet(Fleet* fleet)
{
	if (!fleet)
		return;
	
	// Check if fleet is already in the list to avoid duplicates
	for (const auto& existing_fleet : my_fleets)
	{
		if (existing_fleet && existing_fleet->id == fleet->id)
			return;  // Fleet already added
	}
	
	my_fleets.push_back(fleet);
}

void KnowledgePlanet::remove_my_fleet(uint32_t fleet_id)
{
	auto it = std::find_if(my_fleets.begin(), my_fleets.end(),
		[fleet_id](const Fleet* fleet) { return fleet && fleet->id == fleet_id; });
	
	if (it != my_fleets.end())
		my_fleets.erase(it);
}

void KnowledgePlanet::add_visible_enemy_fleet(const FleetVisibleInfo& fleet_info)
{
	// Check if fleet is already in the list to avoid duplicates
	for (const auto& existing_fleet : enemy_fleets)
	{
		if (existing_fleet.fleet_id == fleet_info.fleet_id)
			return;  // Fleet already added
	}
	
	enemy_fleets.push_back(fleet_info);
}

void KnowledgePlanet::remove_visible_enemy_fleet(uint32_t fleet_id)
{
	auto it = std::find_if(enemy_fleets.begin(), enemy_fleets.end(),
		[fleet_id](const FleetVisibleInfo& fleet) { return fleet.fleet_id == fleet_id; });
	
	if (it != enemy_fleets.end())
		enemy_fleets.erase(it);
}

void KnowledgePlanet::clear_enemy_fleets()
{
	enemy_fleets.clear();
}
