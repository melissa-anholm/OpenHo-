#ifndef OPENHO_KNOWLEDGE_PLANET_H
#define OPENHO_KNOWLEDGE_PLANET_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include "enums.h"
#include "planet.h"

// ============================================================================
// Forward Declarations
// ============================================================================

typedef uint32_t EntityID;
typedef int32_t PlayerID;
typedef double GalaxyCoord;

class Player;
struct Fleet;

// ============================================================================
// FleetVisibleInfo Struct
// ============================================================================

/// Information about an enemy fleet visible at a planet
struct FleetVisibleInfo
{
	uint32_t fleet_id;           // Unique identifier for the fleet
	PlayerID owner;              // Which player owns this fleet
	uint32_t ship_count;         // Number of ships in the fleet
	// TODO: Add more observable properties as needed (design type, fuel level, etc.)
};

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
	
	// Fleet visibility - fleets at this planet
	std::vector<Fleet*> my_fleets;                 // Player's own fleets stationed here
	std::vector<FleetVisibleInfo> enemy_fleets;    // Enemy fleets visible at this planet
	
	// Default constructor - initializes with partial info (id, name, coordinates only)
	KnowledgePlanet(const Planet& planet, PlayerID player_id);
	
	// Update snapshot with current observation of the planet
	void observe_planet(const Planet& planet, const Player* observer, int32_t current_year);
	
	// Fleet management methods
	void add_my_fleet(Fleet* fleet);
	void remove_my_fleet(uint32_t fleet_id);
	void add_visible_enemy_fleet(const FleetVisibleInfo& fleet_info);
	void remove_visible_enemy_fleet(uint32_t fleet_id);
	void clear_enemy_fleets();
};

#endif // OPENHO_KNOWLEDGE_PLANET_H
