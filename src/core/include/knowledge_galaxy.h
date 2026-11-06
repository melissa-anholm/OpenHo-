#ifndef OPENHO_KNOWLEDGE_GALAXY_H
#define OPENHO_KNOWLEDGE_GALAXY_H

#include <cstdint>
#include <vector>
#include "planet_view.h"
#include "planet_distance.h"

// ============================================================================
// Forward Declarations
// ============================================================================

typedef int32_t PlayerID;

struct Galaxy;
class Planet;
class Player;

// ============================================================================
// KnowledgeGalaxy Class
// ============================================================================

// Temporary player-view compatibility facade.
// Planet owns the PlanetView records; this class delegates lookups and keeps
// the distance-query compatibility behavior until direct query APIs exist.
class KnowledgeGalaxy
{
private:
	Galaxy* real_galaxy;  // Reference to the real galaxy (for compatibility queries)
	std::vector<std::vector<double>> distance_matrix;  // Local copy of distance matrix for O(1) access
	std::vector<std::vector<PlanetDistance>> destinations_by_distance;
	PlayerID player_id;

public:
	// Constructor - ensures this player's PlanetView exists for every planet
	KnowledgeGalaxy(Galaxy& galaxy, PlayerID player_id);
	
	// Destructor
	~KnowledgeGalaxy();
	
	// Accessors
	PlanetView* get_planet(uint32_t planet_id);
	const PlanetView* get_planet(uint32_t planet_id) const;
	
	size_t get_planet_count() const;
	
	// Access to real galaxy (for edge cases)
	const Planet* get_real_planet(uint32_t planet_id) const;
	
	// Get distance between two planets (O(1) local lookup, no network latency)
	// Returns Euclidean distance rounded to nearest integer
	// Throws std::out_of_range if planet IDs are invalid
	double get_distance(uint32_t from_id, uint32_t to_id) const;
	double get_distance(const std::string& from_name, const std::string& to_name) const;
	const std::vector<PlanetDistance>& get_destinations_by_distance(uint32_t from_id) const;
};

#endif // OPENHO_KNOWLEDGE_GALAXY_H
