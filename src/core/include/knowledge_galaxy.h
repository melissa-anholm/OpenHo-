#ifndef OPENHO_KNOWLEDGE_GALAXY_H
#define OPENHO_KNOWLEDGE_GALAXY_H

#include <cstdint>
#include <vector>
#include "knowledge_planet.h"

// ============================================================================
// Forward Declarations
// ============================================================================

typedef int32_t PlayerID;

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
	std::vector<KnowledgePlanet> knowledge_planets;  // Player's knowledge of each planet (indexed by planet_id)
	std::vector<std::vector<double>> distance_matrix;  // Local copy of distance matrix for O(1) access
	PlayerID player_id;
	
	// Space planets for holding in-transit fleets
	Planet* space_real_planet;  // Virtual planet owned by this KnowledgeGalaxy
	KnowledgePlanet* space_knowledge_planet;  // Player's knowledge view of the space planet

public:
	// Constructor - initializes with partial info for all planets
	KnowledgeGalaxy(const Galaxy& galaxy, PlayerID player_id);
	
	// Destructor - cleans up space_real_planet
	~KnowledgeGalaxy();
	
	// Accessors
	KnowledgePlanet* get_planet(uint32_t planet_id);
	const KnowledgePlanet* get_planet(uint32_t planet_id) const;
	
	size_t get_planet_count() const { return knowledge_planets.size(); }
	
	// Update player's knowledge of a planet
	void observe_planet(uint32_t planet_id, const Planet& real_planet, const Player* observer, int32_t current_year);
	
	// Access to real galaxy (for edge cases)
	const Planet* get_real_planet(uint32_t planet_id) const;
	
	// Get distance between two planets (O(1) local lookup, no network latency)
	// Returns Euclidean distance rounded to nearest integer
	// Throws std::out_of_range if planet IDs are invalid
	double get_distance(uint32_t from_id, uint32_t to_id) const;
	
	// Access to space planet (for in-transit fleets)
	Planet* get_space_real_planet() { return space_real_planet; }
	const Planet* get_space_real_planet() const { return space_real_planet; }
	
	KnowledgePlanet* get_space_knowledge_planet() { return space_knowledge_planet; }
	const KnowledgePlanet* get_space_knowledge_planet() const { return space_knowledge_planet; }
};

#endif // OPENHO_KNOWLEDGE_GALAXY_H
