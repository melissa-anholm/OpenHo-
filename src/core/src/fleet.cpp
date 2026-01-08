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
             uint32_t ship_count, Planet* planet)
	: id(fleet_id),
	  owner(player_id),
	  ship_design(design),
	  ship_count(ship_count),
	  fuel(design ? design->get_range() : 0),
	  in_transit(false),
	  current_planet(planet),
	  origin_planet(planet),
	  destination_planet(nullptr),
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
		fuel = ship_design->get_range();
}

void Fleet::partial_refuel(int32_t amount)
{
	if (ship_design)
	{
		fuel += amount;
		if (fuel > ship_design->get_range())
			fuel = ship_design->get_range();  // Cap at maximum
	}
}

void Fleet::move_to(Planet* destination, KnowledgeGalaxy* knowledge_galaxy, uint32_t current_turn)
{
	// Validate inputs
	if (!destination || !knowledge_galaxy || !current_planet)
		return;
	
	// If fleet is already at destination, do nothing
	if (current_planet->id == destination->id)
		return;
	
	// Get origin planet ID
	uint32_t origin_id = current_planet->id;
	uint32_t dest_id = destination->id;
	
	// Get distance from knowledge galaxy's distance matrix
	double distance = knowledge_galaxy->get_distance(origin_id, dest_id);
	
	// Calculate turns to destination based on fleet's range (speed)
	uint32_t turns = 0;
	if (ship_design && ship_design->get_range() > 0)
	{
		turns = static_cast<uint32_t>(std::ceil(distance / ship_design->get_range()));
	}
	if (turns == 0)
		turns = 1;  // At least 1 turn to travel
	
	uint32_t arrival_turn = current_turn + turns;
	
	// Create FleetTransit to hold transit state
	transit = std::make_unique<FleetTransit>(
		origin_id,
		dest_id,
		current_turn,
		arrival_turn,
		distance,
		turns
	);
	
	// Update fleet state
	in_transit = true;
	origin_planet = current_planet;
	destination_planet = destination;
	distance_to_destination = distance;
	turns_to_destination = turns;
	
	// Move fleet to space planet (both real and knowledge)
	Planet* space_planet = knowledge_galaxy->get_space_real_planet();
	if (space_planet)
	{
		current_planet = space_planet;
		
		// Add fleet to space knowledge planet's fleet list
		KnowledgePlanet* space_knowledge = knowledge_galaxy->get_space_knowledge_planet();
		if (space_knowledge)
		{
			space_knowledge->add_my_fleet(this);
		}
	}
}
