#ifndef OPENHO_PLANET_H
#define OPENHO_PLANET_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include "enums.h"
#include "planet_view.h"

// ============================================================================
// Forward Declarations
// ============================================================================

typedef uint32_t EntityID;
typedef int32_t PlayerID;
typedef double GalaxyCoord;

// ============================================================================
// Planet Class
// ============================================================================

// Planet class - represents a planet in the galaxy (actual state on the host)
class Planet
{
public:
	uint32_t id;
	std::string name;
	GalaxyCoord x;
	GalaxyCoord y;
	
	double true_gravity;
	double true_temperature;
	int32_t metal;
	int32_t population;
	
	PlayerID owner;  // NOT_OWNED if unowned
	
	// Nova state
	PlanetNovaState nova_state;

	// Observer-specific knowledge snapshots. Planet owns the records, while
	// PlanetView contains only potentially stale player-specific information.
	std::unordered_map<PlayerID, PlanetView> Views;
	
	// Constructor
	Planet(
		uint32_t p_id,
		const std::string& p_name,
		GalaxyCoord p_x,
		GalaxyCoord p_y,
		double p_true_gravity,
		double p_true_temperature,
		int32_t p_metal,
		PlayerID p_owner = 0 );

	PlanetView& get_or_create_view(PlayerID observer_id);
	PlanetView* get_view(PlayerID observer_id);
	const PlanetView* get_view(PlayerID observer_id) const;
};

#endif // OPENHO_PLANET_H
