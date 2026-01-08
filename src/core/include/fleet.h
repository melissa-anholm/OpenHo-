#ifndef FLEET_H
#define FLEET_H

#include <cstdint>
#include <string>
#include <memory>

// Forward declarations
struct Planet;
struct ShipDesign;
class KnowledgeGalaxy;
typedef int32_t PlayerID;

// Forward declaration
class Player;

// ============================================================================
// FleetTransit Struct
// ============================================================================

// Encapsulates all transit state for a fleet in motion
struct FleetTransit
{
	uint32_t origin_planet_id;          // ID of origin planet
	uint32_t destination_planet_id;     // ID of destination planet
	uint32_t departure_turn;            // Turn when fleet departed
	uint32_t arrival_turn;              // Turn when fleet arrives
	double distance;                    // Distance traveled (from matrix)
	uint32_t turns_to_travel;           // Turns needed to reach destination
	
	FleetTransit(uint32_t origin, uint32_t dest, uint32_t dep, uint32_t arr, double dist, uint32_t turns)
		: origin_planet_id(origin),
		  destination_planet_id(dest),
		  departure_turn(dep),
		  arrival_turn(arr),
		  distance(dist),
		  turns_to_travel(turns)
	{ }
};

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
	
	int32_t fuel;                   // Current fuel level (all ships have same amount)
	bool in_transit;                // Is fleet currently traveling?
	
	std::string descriptor;
	
	// Position tracking
	Planet* current_planet;         // Planet fleet is currently at (nullptr if in transit)
	Planet* origin_planet;          // Planet fleet departed from
	Planet* destination_planet;     // Planet fleet is heading to
	
	// Movement tracking
	double distance_to_destination; // Remaining distance in light-years
	uint32_t turns_to_destination;  // Estimated turns until arrival
	
	// Transit state (new architecture)
	std::unique_ptr<FleetTransit> transit;  // nullptr if docked, not-null if in transit
	
	// Move semantics (required by unique_ptr)
	Fleet(Fleet&&) = default;
	Fleet& operator=(Fleet&&) = default;
	
	// Delete copy semantics (unique_ptr is not copyable)
	Fleet(const Fleet&) = delete;
	Fleet& operator=(const Fleet&) = delete;
	
	// Member functions (implemented in fleet.cpp)
	
	/// Refuel fleet to maximum capacity (based on ship design's range)
	void refuel();
	
	/// Refuel fleet by a specific amount, capped at maximum capacity
	void partial_refuel(int32_t amount);
	
	/// Move fleet to destination planet
	/// Sets up fleet movement with distance and turns calculated from distance matrix
	void move_to(Planet* destination, KnowledgeGalaxy* knowledge_galaxy, uint32_t current_turn);
};

#endif // FLEET_H
