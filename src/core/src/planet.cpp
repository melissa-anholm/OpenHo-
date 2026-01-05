#include "planet.h"

// ============================================================================
// Planet Implementation
// ============================================================================

Planet::Planet(
	uint32_t p_id,
	const std::string& p_name,
	GalaxyCoord p_x,
	GalaxyCoord p_y,
	double p_true_gravity,
	double p_true_temperature,
	int32_t p_metal,
	PlayerID p_owner )
	: id(p_id),
	  name(p_name),
	  x(p_x),
	  y(p_y),
	  true_gravity(p_true_gravity),
	  true_temperature(p_true_temperature),
	  metal(p_metal),
	  population(0),
	  owner(p_owner),
	  nova_state(PLANET_NORMAL)
{ }
