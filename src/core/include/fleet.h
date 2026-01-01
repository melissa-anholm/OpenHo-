#ifndef FLEET_H
#define FLEET_H

#include <cstdint>
#include <string>

// Forward declarations
struct Planet;
struct ShipDesign;
typedef uint32_t PlayerID;

/// Fleet represents a group of identical ships traveling together
/// All ships in a fleet must have the same design and fuel level
struct Fleet
{
	uint32_t id;                    // Unique fleet identifier
	PlayerID owner;                 // Which player owns this fleet
	
	const ShipDesign* ship_design;  // All ships in fleet use same design (pointer, not owned)
	uint32_t ship_count;            // Number of identical ships in fleet (max 1000)
	
	double fuel;                    // Current fuel level (all ships have same amount)
	bool in_transit;                // Is fleet currently traveling?
	
	// Position tracking
	Planet* current_planet;         // Planet fleet is currently at (nullptr if in transit)
	Planet* origin_planet;          // Planet fleet departed from
	Planet* destination_planet;     // Planet fleet is heading to
	
	// Movement tracking
	double distance_to_destination; // Remaining distance in light-years
	uint32_t turns_to_destination;  // Estimated turns until arrival
	
	// Inline member functions
	
	/// Refuel fleet to maximum capacity (based on ship design's range)
	void refuel()
	{
		fuel = ship_design->get_range();
	}
	
	/// Refuel fleet by a specific amount, capped at maximum capacity
	void partial_refuel(double amount)
	{
		fuel += amount;
		if (fuel > ship_design->get_range())
			fuel = ship_design->get_range();  // Cap at maximum
	}
};

#endif // FLEET_H
