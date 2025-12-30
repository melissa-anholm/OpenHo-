#ifndef OPENHO_SHIP_H
#define OPENHO_SHIP_H

#include <cstdint>
#include <string>

// ============================================================================
// Forward Declarations
// ============================================================================

typedef uint32_t PlayerID;

// ============================================================================
// Data Structures
// ============================================================================

// Ship technological parameters (subset of TechnologyLevels)
struct ShipTechParameters
{
	int32_t range;
	int32_t speed;
	int32_t weapons;
	int32_t shields;
	int32_t miniaturization;
};

// Ship structure
struct Ship
{
	uint32_t id;
	ShipType type;
	PlayerID owner;
	ShipTechParameters techParams;
	
	// Position: either at a planet or in transit
	uint32_t current_planet_id;  // If not in transit
	uint32_t destination_planet_id;  // If in transit
	int32_t eta_turns;  // Turns until arrival (if in transit)
	
	// Fleet information
	uint32_t fleet_id;  // Every ship is always in a fleet; a lone ship is a fleet of one
};

// Ship design pattern (immutable once created)
struct ShipDesign
{
	uint32_t id;                    // Unique ID within player's design library
	std::string name;
	ShipType type;
	
	// Technology levels associated with this design
	int32_t tech_range;
	int32_t tech_speed;
	int32_t tech_weapons;
	int32_t tech_shields;
	int32_t tech_miniaturization;
	
	// Costs (calculated at design creation time, immutable)
	int64_t build_cost;              // Cost to build each ship of this design
	int64_t prototype_cost;          // Cost for the first ship of this design
	int64_t metal_cost;              // Metal cost per ship
};

#endif // OPENHO_SHIP_H
