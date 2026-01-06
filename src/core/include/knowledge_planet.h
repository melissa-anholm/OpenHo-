#ifndef OPENHO_KNOWLEDGE_PLANET_H
#define OPENHO_KNOWLEDGE_PLANET_H

#include <cstdint>
#include <memory>
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
// KnowledgePlanet Class
// ============================================================================

// Planet information snapshot (what a player knows about a planet)
class KnowledgePlanet
{
public:
	// Core planet identity - set at construction, should not be modified
	uint32_t id;
	std::string name;
	GalaxyCoord x;
	GalaxyCoord y;
	PlayerID as_seen_by;  // Which player created this snapshot

	// Observable fields - updated via observe_planet()
	double apparent_temperature;
	double apparent_gravity;
	int32_t metal;
	PlayerID apparent_owner;
	int32_t apparent_population;  // POPULATION_UNKNOWN if unknown, otherwise estimated population
	int32_t observation_year;  // When this information was collected
	
	int32_t can_be_profitable;
	int32_t perceived_value;
	
	// Nova state - can be updated independently, not by observe_planet()
	PlanetNovaState nova_state;
	
	// Colonization info - if this player has colonized this planet
	// nullptr if not colonized, otherwise contains colonization-specific data
	std::unique_ptr<class ColonizedPlanet> colonization;
	
	// Default constructor - initializes with partial info (id, name, coordinates only)
	KnowledgePlanet(const Planet& planet, PlayerID player_id);
	
	// Update snapshot with current observation of the planet
	void observe_planet(const Planet& planet, const Player* observer, int32_t current_year);
};

#endif // OPENHO_KNOWLEDGE_PLANET_H
