
#include "fleet.h"
#include "ship_design.h"
#include "planet.h"

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
}

// ============================================================================
// Fleet Member Functions
// ============================================================================

void Fleet::refuel()
{
	fuel = ship_design->get_range();
}

void Fleet::partial_refuel(double amount)
{
	fuel += amount;
	if (fuel > ship_design->get_range())
		fuel = ship_design->get_range();  // Cap at maximum
}
