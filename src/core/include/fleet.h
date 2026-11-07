#ifndef FLEET_H
#define FLEET_H

#include <cstdint>
#include <string>
#include <memory>
#include <utility>
#include <variant>
#include <vector>

// Forward declarations
struct Planet;
struct ShipDesign;
class KnowledgeGalaxy;
typedef int32_t PlayerID;

// Forward declaration
class Player;

struct FleetStationedLocation
{
	std::string planet_name;
};

struct FleetInTransitLocation
{
	std::string origin_planet_name;
	std::string destination_planet_name;
	uint32_t departure_turn = 0;
	uint32_t arrival_turn = 0;
	double distance = 0.0;
	uint32_t turns_to_travel = 0;
};

using FleetLocation = std::variant<FleetStationedLocation, FleetInTransitLocation>;

struct FleetShipStack
{
	uint32_t design_id;
	const ShipDesign* design;
	uint32_t ship_count;
};

/// Fleet represents a group of ships traveling together.
/// A fleet may contain multiple design-specific stacks.
struct Fleet
{
	friend class Player;  // Only Player can construct Fleet instances
	
private:
	/// Private constructor - only Player can create Fleet instances
	Fleet(uint32_t fleet_id, PlayerID player_id, uint32_t design_id,
	      const ShipDesign* design,
	      uint32_t ship_count, const std::string& planet_name);

public:
	uint32_t id;                    // Unique fleet identifier
	PlayerID owner;                 // Which player owns this fleet
	
	std::vector<FleetShipStack> ship_stacks;
	
	int32_t fuel;                   // Shared fleet fuel level
	bool newly_built_this_turn;     // True only during the turn in which this fleet was built
	
	std::string descriptor;
	FleetLocation location;
	
	// Move semantics (required by unique_ptr)
	Fleet(Fleet&&) = default;
	Fleet& operator=(Fleet&&) = default;
	
	// Delete copy semantics (unique_ptr is not copyable)
	Fleet(const Fleet&) = delete;
	Fleet& operator=(const Fleet&) = delete;
	
	// Member functions (implemented in fleet.cpp)
	
	/// Refuel fleet to maximum capacity (limited by the slowest ship stack)
	void refuel();
	
	/// Refuel fleet by a specific amount, capped at maximum capacity
	void partial_refuel(int32_t amount);
	
	/// Move fleet to destination planet
	/// Sets up fleet movement with distance and turns calculated from distance matrix
	void move_to(const std::string& destination_planet_name,
	             KnowledgeGalaxy* knowledge_galaxy,
	             uint32_t current_turn);

	[[nodiscard]] bool is_in_transit() const;
	[[nodiscard]] const std::string& stationed_planet_name() const;
	[[nodiscard]] const FleetInTransitLocation* transit_location() const;
	[[nodiscard]] uint32_t total_ship_count() const;
	[[nodiscard]] int32_t movement_range() const;
};

#endif // FLEET_H
