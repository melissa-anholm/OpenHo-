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
	uint32_t design_id = 0;
	const ShipDesign* design = nullptr;
	uint32_t ship_count = 0;
	int32_t fuel = 0;
};

struct FleetCapabilities
{
	int32_t effective_fuel = 0;
	int32_t effective_range = 0;
	int32_t effective_speed = 0;
};

/// Fleet represents a group of ships traveling together.
/// A fleet may contain multiple design-specific stacks.
struct Fleet
{
	friend class Player;  // Only Player can construct Fleet instances
	friend class GameState;  // GameState restores fleets from checkpoints
	
private:
	/// Private constructor - only Player can create Fleet instances
	Fleet(uint32_t fleet_id, PlayerID player_id, uint32_t design_id,
	      const ShipDesign* design,
	      uint32_t ship_count, const std::string& planet_name);

public:
	uint32_t id;                    // Unique fleet identifier
	PlayerID owner;                 // Which player owns this fleet
	
	std::vector<FleetShipStack> ship_stacks;
	
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

	/// Return the stack for a design, or nullptr if this fleet does not contain it.
	[[nodiscard]] FleetShipStack* find_stack(uint32_t design_id);
	[[nodiscard]] const FleetShipStack* find_stack(uint32_t design_id) const;
	/// Remove ships from a stack. The caller removes the fleet if it becomes empty.
	[[nodiscard]] bool remove_from_stack(uint32_t design_id, uint32_t ship_count,
	                                    FleetShipStack& extracted);
	/// Add ships to a stack, coalescing by design and conservatively merging fuel.
	void add_to_stack(const FleetShipStack& stack);
	
	/// Move fleet to destination planet
	/// Sets up fleet movement with distance and turns calculated from distance matrix
	void move_to(const std::string& destination_planet_name,
	             KnowledgeGalaxy* knowledge_galaxy,
	             uint32_t current_turn);

	[[nodiscard]] bool is_in_transit() const;
	[[nodiscard]] const std::string& stationed_planet_name() const;
	[[nodiscard]] const FleetInTransitLocation* transit_location() const;
	[[nodiscard]] uint32_t total_ship_count() const;
	[[nodiscard]] int32_t effective_fuel() const;
	[[nodiscard]] int32_t effective_range() const;
	[[nodiscard]] int32_t effective_speed() const;
	[[nodiscard]] FleetCapabilities capabilities() const;
	[[nodiscard]] int32_t movement_range() const { return effective_range(); }
};

#endif // FLEET_H
