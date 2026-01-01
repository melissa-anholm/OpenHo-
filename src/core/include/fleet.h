#ifndef FLEET_H
#define FLEET_H

#include <cstdint>
#include <string>

// Forward declarations
struct Planet;
struct ShipDesign;
typedef uint32_t PlayerID;

// Forward declaration
class Player;

/// Fleet represents a group of identical ships traveling together
/// All ships in a fleet must have the same design and fuel level
struct Fleet
{
	friend class Player;  // Only Player can construct Fleet instances
	
private:
	/// Private constructor - only Player can create Fleet instances
	Fleet(uint32_t fleet_id, PlayerID player_id, const ShipDesign* design, 
	      uint32_t ship_count, Planet* planet);


public:
	uint32_t id;                    // Unique fleet identifier
	PlayerID owner;                 // Which player owns this fleet
	
	const ShipDesign* ship_design;  // All ships in fleet use same design (pointer, not owned)
	uint32_t ship_count;            // Number of identical ships in fleet (max 1000)
	
	int64_t fuel;                   // Current fuel level (all ships have same amount)
	bool in_transit;                // Is fleet currently traveling?
	
	// Position tracking
	Planet* current_planet;         // Planet fleet is currently at (nullptr if in transit)
	Planet* origin_planet;          // Planet fleet departed from
	Planet* destination_planet;     // Planet fleet is heading to
	
	// Movement tracking
	double distance_to_destination; // Remaining distance in light-years
	uint32_t turns_to_destination;  // Estimated turns until arrival
	
	// Member functions (implemented in fleet.cpp)
	
	/// Refuel fleet to maximum capacity (based on ship design's range)
	void refuel();
	
	/// Refuel fleet by a specific amount, capped at maximum capacity
	void partial_refuel(int64_t amount);
};

#endif // FLEET_H
