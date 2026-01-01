#ifndef OPENHO_ENUMS_H
#define OPENHO_ENUMS_H

#include <map>
#include <string>

// ============================================================================
// Game Enums
// ============================================================================

// Ship types
enum ShipType
{
	SHIP_SCOUT = 0,
	SHIP_FIGHTER = 1,
	SHIP_COLONY = 2,
	SHIP_SATELLITE = 3,
	SHIP_TANKER = 4,
	SHIP_DREADNOUGHT = 5,
	SHIP_BIOLOGICAL = 6
};

// Reverse enum lookup for ShipType
const std::map<ShipType, std::string> SHIP_TYPE_NAMES = 
{
	{ SHIP_SCOUT,       "Scout"       },
	{ SHIP_FIGHTER,     "Fighter"     },
	{ SHIP_COLONY,      "Colony"      },
	{ SHIP_SATELLITE,   "Satellite"   },
	{ SHIP_TANKER,      "Tanker"      },
	{ SHIP_DREADNOUGHT, "Dreadnought" },
	{ SHIP_BIOLOGICAL,  "Biological"  }
};

// Get the string name for a ShipType
inline std::string get_ship_type_name(ShipType type)
{
	std::map<ShipType, std::string>::const_iterator it = SHIP_TYPE_NAMES.find(type);
	if (it != SHIP_TYPE_NAMES.end())
		{ return it->second; }
	return "Unknown";
}

// Gender
enum Gender
{
	GENDER_F = 1,
	GENDER_M = 2
};

// Planet nova state
enum PlanetNovaState
{
	PLANET_NORMAL = 0,
	PLANET_WARNING = 1,
	PLANET_DESTROYED = 2
};

// Player type (human or computer)
enum PlayerType
{
	PLAYER_HUMAN = 0,
	PLAYER_COMPUTER = 1
};

	// Technology streams for research
enum TechStream
{
	TECH_RANGE = 0,
	TECH_SPEED = 1,
	TECH_WEAPONS = 2,
	TECH_SHIELDS = 3,
	TECH_MINI = 4,
	TECH_RADICAL = 5
};

// Colony quality levels for starting planets
enum ColonyQuality
{
	COLONY_OUTPOST = 0,
	COLONY_BARREN = 1,
	COLONY_BACKWARD = 2,
	COLONY_NORMAL = 3,
	COLONY_ADVANCED = 4,
	COLONY_THRIVING = 5,
	COLONY_ABUNDANT = 6
};

// Reverse enum lookup for ColonyQuality
const std::map<ColonyQuality, std::string> COLONY_QUALITY_NAMES = 
{
	{ COLONY_OUTPOST,   "Outpost"   },
	{ COLONY_BARREN,    "Barren"    },
	{ COLONY_BACKWARD,  "Backward"  },
	{ COLONY_NORMAL,    "Normal"    },
	{ COLONY_ADVANCED,  "Advanced"  },
	{ COLONY_THRIVING,  "Thriving"  },
	{ COLONY_ABUNDANT,  "Abundant"  }
};

// Get the string name for a ColonyQuality
inline std::string get_colony_quality_name(ColonyQuality quality)
{
	std::map<ColonyQuality, std::string>::const_iterator it = COLONY_QUALITY_NAMES.find(quality);
	if (it != COLONY_QUALITY_NAMES.end())
		{ return it->second; }
	return "Unknown";
}

// Galaxy generation shape patterns
enum GalaxyShape
{
	GALAXY_RANDOM = 0,
	GALAXY_SPIRAL = 1,
	GALAXY_CIRCLE = 2,
	GALAXY_RING = 3,
	GALAXY_CLUSTER = 4,
	GALAXY_GRID = 5
};

// ============================================================================
// Sentinel Values for Unknown/Unowned States
// ============================================================================

constexpr double UNKNOWN_DOUBLE_VALUE = -500.0;
constexpr int32_t UNKNOWN_INT_VALUE = -500;
constexpr int64_t INCOME_UNKNOWN = INT32_MIN;
constexpr uint32_t NOT_OWNED = 0;
constexpr uint32_t OWNER_UNKNOWN = UINT32_MAX;

// ============================================================================
// Error Codes
// ============================================================================
// TODO: Implement error code system in Phase 2b
// Currently, methods return nullptr/0/false on error. In the future, refactor
// to return ErrorCode values and use output parameters for results.
// See MASTER_PLAN.md for details.
/*
enum ErrorCode
{
	ERROR_SUCCESS = 0,                    // Operation completed successfully
	ERROR_ENTITY_NOT_FOUND = 1,           // Player, planet, ship, or design not found
	ERROR_INVALID_PLAYER_ID = 2,          // Player ID doesn't exist
	ERROR_DESIGN_LIMIT_EXCEEDED = 3,      // Player has reached max ship designs (100)
	ERROR_DESIGN_IN_USE = 4,              // Cannot delete design - ships are using it
	ERROR_INITIALIZATION_FAILED = 5       // Game initialization failed
};
*/

#endif  // OPENHO_ENUMS_H
