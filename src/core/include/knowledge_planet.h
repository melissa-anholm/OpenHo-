#ifndef OPENHO_KNOWLEDGE_PLANET_H
#define OPENHO_KNOWLEDGE_PLANET_H

#include <cstdint>
#include <string>
#include "enums.h"
#include "planet.h"

// ============================================================================
// Forward Declarations
// ============================================================================

typedef uint32_t EntityID;
typedef uint32_t PlayerID;
typedef double GalaxyCoord;

class Player;

// ============================================================================
// PlanetSnapshot Class (Foundation for KnowledgePlanet)
// ============================================================================

// Planet information snapshot (what a player knows about a planet)
// This will eventually become the KnowledgePlanet class
class PlanetSnapshot
{
public:
	uint32_t id;
	std::string name;
	GalaxyCoord x;
	GalaxyCoord y;
	
	PlayerID as_seen_by;  // Which player created this snapshot
	double apparent_temperature;
	double apparent_gravity;
	int32_t metal;
	PlayerID apparent_owner;
	int32_t population;
	int32_t income;
	int32_t observation_year;  // When this information was collected
	
	int32_t can_be_profitable;
	int32_t perceived_value;
	
	// Default constructor - initializes with partial info (id, name, coordinates only)
	PlanetSnapshot(const Planet& planet, PlayerID player_id);
	
	// Factory method to create full info snapshot
	// full_info: all available fields are copied with apparent values calculated
	static PlanetSnapshot full_info(const Planet& planet, PlayerID player_id, Player* owner_player);
};

#endif // OPENHO_KNOWLEDGE_PLANET_H
