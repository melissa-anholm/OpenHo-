#include "fleet.h"
#include "ship_design.h"
#include "planet.h"
#include "knowledge_galaxy.h"
#include <algorithm>
#include <cmath>
#include <climits>
#include <memory>

// ============================================================================
// Fleet Constructor
// ============================================================================

Fleet::Fleet(uint32_t fleet_id, PlayerID player_id, uint32_t design_id,
             const ShipDesign* design,
             uint32_t ship_count, const std::string& planet_name)
		: id(fleet_id),
	  owner(player_id),
	  ship_stacks{{design_id, design, ship_count}},
	  fuel(design ? design->get_range() : 0),
	  newly_built_this_turn(true),
	  location(FleetStationedLocation{planet_name})
{
	if(design && design->type == SHIP_BIOLOGICAL)
	{
		fuel = 0;
	}
	descriptor = "";
}

// ============================================================================
// Fleet Member Functions
// ============================================================================

void Fleet::refuel()
{
	fuel = movement_range();
}

void Fleet::partial_refuel(int32_t amount)
{
	if (!ship_stacks.empty())
	{
		fuel += amount;
		if (fuel > movement_range())
			{ fuel = movement_range(); }  // Cap at the slowest stack
	}
}

void Fleet::move_to(const std::string& destination_name,
                    KnowledgeGalaxy* knowledge_galaxy,
                    uint32_t current_turn)
{
	// Validate inputs
	if (destination_name.empty() || !knowledge_galaxy || is_in_transit())
		{ return; }
	const std::string& current_planet_name = stationed_planet_name();
	
	// If fleet is already at destination, do nothing
	if (current_planet_name == destination_name)
		{ return; }
	
	// Get distance from knowledge galaxy's distance matrix
	double distance = knowledge_galaxy->get_distance(current_planet_name, destination_name);
	
	// Calculate turns to destination based on fleet's range (speed)
	uint32_t turns = 0;
	if (movement_range() > 0)
	{
		turns = static_cast<uint32_t>(std::ceil(distance / movement_range()));
	}
	if (turns == 0)
		{ turns = 1; }  // At least 1 turn to travel
	
	uint32_t arrival_turn = current_turn + turns;
	
	location = FleetInTransitLocation{
		current_planet_name, destination_name, current_turn, arrival_turn,
		distance, turns};
}

bool Fleet::is_in_transit() const
{
	return std::holds_alternative<FleetInTransitLocation>(location);
}

const std::string& Fleet::stationed_planet_name() const
{
	return std::get<FleetStationedLocation>(location).planet_name;
}

const FleetInTransitLocation* Fleet::transit_location() const
{
	return std::get_if<FleetInTransitLocation>(&location);
}

uint32_t Fleet::total_ship_count() const
{
	uint64_t total = 0;
	for (const auto& stack : ship_stacks)
		total += stack.ship_count;
	return total > UINT32_MAX ? UINT32_MAX : static_cast<uint32_t>(total);
}

int32_t Fleet::movement_range() const
{
	if (ship_stacks.empty())
		return 0;
	int32_t range = INT32_MAX;
	for (const auto& stack : ship_stacks)
	{
		if (!stack.design)
			return 0;
		range = std::min(range, stack.design->get_range());
	}
	return range == INT32_MAX ? 0 : range;
}
