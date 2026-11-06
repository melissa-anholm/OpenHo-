#include "fleet.h"
#include "ship_design.h"
#include "planet.h"
#include "knowledge_galaxy.h"
#include <cmath>
#include <memory>

// ============================================================================
// Fleet Constructor
// ============================================================================

Fleet::Fleet(uint32_t fleet_id, PlayerID player_id, const ShipDesign* design,
             uint32_t ship_count, const std::string& planet_name)
		: id(fleet_id),
	  owner(player_id),
	  ship_design(design),
	  ship_count(ship_count),
	  fuel(design ? design->get_range() : 0),
	  in_transit(false),
		  current_planet_name(planet_name),
		  origin_planet_name(planet_name),
		  destination_planet_name(),
	  distance_to_destination(0),
	  turns_to_destination(0),
	  transit(nullptr)
{
	if(ship_design && ship_design->type == SHIP_BIOLOGICAL)
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
	if (ship_design)
		{ fuel = ship_design->get_range(); }
}

void Fleet::partial_refuel(int32_t amount)
{
	if (ship_design)
	{
		fuel += amount;
		if (fuel > ship_design->get_range())
			{ fuel = ship_design->get_range(); }  // Cap at maximum
	}
}

void Fleet::move_to(const std::string& destination_name,
                    KnowledgeGalaxy* knowledge_galaxy,
                    uint32_t current_turn)
{
	// Validate inputs
	if (destination_name.empty() || !knowledge_galaxy || current_planet_name.empty())
		{ return; }
	
	// If fleet is already at destination, do nothing
	if (current_planet_name == destination_name)
		{ return; }
	
	// Get distance from knowledge galaxy's distance matrix
	double distance = knowledge_galaxy->get_distance(current_planet_name, destination_name);
	
	// Calculate turns to destination based on fleet's range (speed)
	uint32_t turns = 0;
	if (ship_design && ship_design->get_range() > 0)
	{
		turns = static_cast<uint32_t>(std::ceil(distance / ship_design->get_range()));
	}
	if (turns == 0)
		{ turns = 1; }  // At least 1 turn to travel
	
	uint32_t arrival_turn = current_turn + turns;
	
	// Create FleetTransit to hold transit state
	transit = std::make_unique<FleetTransit>(
			current_planet_name,
			destination_name,
		current_turn,
		arrival_turn,
		distance,
		turns );
	
	// Update fleet state
	in_transit = true;
	origin_planet_name = current_planet_name;
	destination_planet_name = destination_name;
	distance_to_destination = distance;
	turns_to_destination = turns;
	
	// An in-transit fleet has no stationed planet. Its origin and destination
	// names remain in the transit record until arrival.
	current_planet_name.clear();
}
