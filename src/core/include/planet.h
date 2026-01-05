#ifndef OPENHO_PLANET_H
#define OPENHO_PLANET_H

#include <cstdint>
#include <string>
#include "enums.h"

// ============================================================================
// Forward Declarations
// ============================================================================

typedef uint32_t EntityID;
typedef uint32_t PlayerID;
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
	
	// Immutable properties
	double true_gravity;
	
	// Mutable properties
	double true_temperature;
	int32_t metal;
	
	PlayerID owner;  // 0 if unowned
	
	// Nova state
	PlanetNovaState nova_state;
	
	// Constructor
	Planet(
		uint32_t p_id,
		const std::string& p_name,
		GalaxyCoord p_x,
		GalaxyCoord p_y,
		double p_true_gravity,
		double p_true_temperature,
		int32_t p_metal,
		PlayerID p_owner = 0);
};

#endif // OPENHO_PLANET_H
