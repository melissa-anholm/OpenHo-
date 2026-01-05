#ifndef OPENHO_KNOWLEDGE_GALAXY_H
#define OPENHO_KNOWLEDGE_GALAXY_H

#include <cstdint>
#include <vector>
#include "knowledge_planet.h"

// ============================================================================
// Forward Declarations
// ============================================================================

typedef uint32_t PlayerID;

class Galaxy;
class Planet;
class Player;

// ============================================================================
// KnowledgeGalaxy Class
// ============================================================================

// Player's knowledge of the galaxy
// Contains KnowledgePlanets representing the player's view of each planet
// Primarily self-sufficient for validation; can reference real galaxy if needed
class KnowledgeGalaxy
{
private:
	const Galaxy* real_galaxy;  // Reference to the real galaxy (for edge cases)
	std::vector<KnowledgePlanet> planets;  // Player's knowledge of each planet (indexed by planet_id)
	PlayerID player_id;

public:
	// Constructor - initializes with partial info for all planets
	KnowledgeGalaxy(const Galaxy& galaxy, PlayerID player_id);
	
	// Accessors
	KnowledgePlanet* get_planet(uint32_t planet_id);
	const KnowledgePlanet* get_planet(uint32_t planet_id) const;
	
	size_t get_planet_count() const { return planets.size(); }
	
	// Update player's knowledge of a planet
	void observe_planet(uint32_t planet_id, const Planet& real_planet, const Player* observer, int32_t current_year);
	
	// Access to real galaxy (for edge cases)
	const Planet* get_real_planet(uint32_t planet_id) const;
};

#endif // OPENHO_KNOWLEDGE_GALAXY_H
