#ifndef FLEET_H
#define FLEET_H

#include <cstdint>
#include <string>
#include <memory>
#include <utility>

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
	std::string origin_planet_name;
	std::string destination_planet_name;
	uint32_t departure_turn;            // Turn when fleet departed
	uint32_t arrival_turn;              // Turn when fleet arrives
	double distance;                    // Distance traveled (from matrix)
	uint32_t turns_to_travel;           // Turns needed to reach destination
	
	FleetTransit(std::string origin, std::string dest, uint32_t dep, uint32_t arr, double dist, uint32_t turns)
		: origin_planet_name(std::move(origin)),
		  destination_planet_name(std::move(dest)),
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
	      uint32_t ship_count, const std::string& planet_name);

public:
	uint32_t id;                    // Unique fleet identifier
	PlayerID owner;                 // Which player owns this fleet
	
	const ShipDesign* ship_design;  // All ships in fleet use same design (pointer, not owned)
	uint32_t ship_count;            // Number of identical ships in fleet (max 1000)
	
	int32_t fuel;                   // Current fuel level (all ships have same amount)
	bool in_transit;                // Is fleet currently traveling?
	
	std::string descriptor;
	
	// Position tracking
	std::string current_planet_name;      // Empty while in transit
	std::string origin_planet_name;       // Planet fleet departed from
	std::string destination_planet_name;  // Empty while stationed
	
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
	void move_to(const std::string& destination_planet_name,
	             KnowledgeGalaxy* knowledge_galaxy,
	             uint32_t current_turn);
};

#endif // FLEET_H
