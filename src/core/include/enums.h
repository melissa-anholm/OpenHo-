#ifndef OPENHO_ENUMS_H
#define OPENHO_ENUMS_H

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
	TECH_MINIATURIZATION = 4,
	TECH_RADICAL = 5
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
