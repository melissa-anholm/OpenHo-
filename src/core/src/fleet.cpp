
#include "fleet.h"
#include "ship_design.h"
#include "planet.h"
#include "knowledge_galaxy.h"
#include <cmath>

// ============================================================================
// Fleet Constructor
// ============================================================================

Fleet::Fleet(uint32_t fleet_id, PlayerID player_id, const ShipDesign* design, 
             uint32_t ship_count, Planet* planet)
	: id(fleet_id),
	  owner(player_id),
	  ship_design(design),
	  ship_count(ship_count),
	  fuel(design->get_range()),
	  in_transit(false),
	  current_planet(planet),
	  origin_planet(planet),
	  destination_planet(nullptr),
	  distance_to_destination(0),
	  turns_to_destination(0)
{
	if(ship_design->type == SHIP_BIOLOGICAL)
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
	fuel = ship_design->get_range();
}
void Fleet::partial_refuel(int32_t amount)
{
	fuel += amount;
	if (fuel > ship_design->get_range())
		fuel = ship_design->get_range();  // Cap at maximum
}

void Fleet::move_to(Planet* destination, const KnowledgeGalaxy* knowledge_galaxy)
{
	// Validate inputs
	if (!destination || !knowledge_galaxy || !current_planet)
		return;
	
	// If fleet is already at destination, do nothing
	if (current_planet->id == destination->id)
		return;
	
	// Set up movement
	origin_planet = current_planet;
	destination_planet = destination;
	current_planet = nullptr;
	in_transit = true;
	
	// Get distance from knowledge galaxy's distance matrix
	double distance = knowledge_galaxy->get_distance(origin_planet->id, destination_planet->id);
	distance_to_destination = distance;
	
	// Calculate turns based on fleet speed
	if (ship_design && ship_design->get_speed() > 0)
	{
		turns_to_destination = static_cast<uint32_t>(
			std::ceil(distance_to_destination / ship_design->get_speed())
		);
	}
	else
	{
		turns_to_destination = 0;
	}
}
