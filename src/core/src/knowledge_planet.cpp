#include "knowledge_planet.h"
#include "planet.h"
#include "player.h"
#include "game_formulas.h"

// ============================================================================
// KnowledgePlanet Implementation
// ============================================================================

KnowledgePlanet::KnowledgePlanet(const Planet& planet, PlayerID player_id)
	: id(planet.id),
	  name(planet.name),
	  x(planet.x),
	  y(planet.y),
	  as_seen_by(player_id),
	  nova_state(PLANET_NORMAL)
{
	// Initialize observable fields with unknown values
	apparent_temperature = UNKNOWN_DOUBLE_VALUE;
	apparent_gravity = UNKNOWN_DOUBLE_VALUE;
	metal = UNKNOWN_INT_VALUE;
	apparent_owner = OWNER_UNKNOWN;
	apparent_population = -1;  // -1 means unknown
	observation_year = OBSERVATION_YEAR_UNKNOWN;
	can_be_profitable = PROFITABILITY_UNKNOWN;
	perceived_value = PERCEIVED_VALUE_UNKNOWN;
}

void KnowledgePlanet::observe_planet(const Planet& planet, const Player* observer, int32_t current_year)
{
	// Update observable fields based on current planet state
	// Note: nova_state is NOT updated by this method
	
	// Calculate apparent values based on the observing player's ideals
	apparent_temperature = GameFormulas::calculate_apparent_temperature(
		observer->get_ideal_temperature(), planet.true_temperature);
	apparent_gravity = GameFormulas::calculate_apparent_gravity(
		observer->get_ideal_gravity(), planet.true_gravity);
	
	// Known fields from planet
	metal = planet.metal;
	apparent_owner = planet.owner;
	apparent_population = planet.population;  // Copy actual population
	
	// Unknown fields (not available from Planet)
	observation_year = current_year;
	can_be_profitable = PROFITABILITY_UNKNOWN;
	perceived_value = PERCEIVED_VALUE_UNKNOWN;
}

KnowledgePlanet KnowledgePlanet::full_info(const Planet& planet, PlayerID player_id, const Player* observer)
{
	KnowledgePlanet snapshot(planet, player_id);
	snapshot.observe_planet(planet, observer, 0);  // Year 0 for initial observation
	return snapshot;
}
