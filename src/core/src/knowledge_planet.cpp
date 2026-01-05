#include "knowledge_planet.h"
#include "planet.h"
#include "player.h"
#include "game_formulas.h"

// ============================================================================
// PlanetSnapshot Implementation
// ============================================================================

PlanetSnapshot::PlanetSnapshot(const Planet& planet, PlayerID player_id)
{
	// Initialize with partial info (id, name, coordinates only)
	id = planet.id;
	name = planet.name;
	x = planet.x;
	y = planet.y;
	as_seen_by = player_id;
	
	// Unknown fields for partial info
	apparent_temperature = UNKNOWN_DOUBLE_VALUE;
	apparent_gravity = UNKNOWN_DOUBLE_VALUE;
	metal = UNKNOWN_INT_VALUE;
	apparent_owner = OWNER_UNKNOWN;
	population = UNKNOWN_INT_VALUE;
	income = INCOME_UNKNOWN;
	observation_year = OBSERVATION_YEAR_UNKNOWN;
	can_be_profitable = PROFITABILITY_UNKNOWN;
	perceived_value = PERCEIVED_VALUE_UNKNOWN;
}

PlanetSnapshot PlanetSnapshot::full_info(const Planet& planet, PlayerID player_id, Player* owner_player)
{
	PlanetSnapshot snapshot(planet, player_id);
	
	// Calculate apparent values based on the observing player's ideals
	snapshot.apparent_temperature = GameFormulas::calculate_apparent_temperature(
		owner_player->get_ideal_temperature(), planet.true_temperature);
	snapshot.apparent_gravity = GameFormulas::calculate_apparent_gravity(
		owner_player->get_ideal_gravity(), planet.true_gravity);
	
	// Known fields from planet
	snapshot.metal = planet.metal;
	snapshot.apparent_owner = planet.owner;
	
	// Unknown fields (not available from Planet)
	snapshot.population = UNKNOWN_INT_VALUE;
	snapshot.income = INCOME_UNKNOWN;
	snapshot.observation_year = OBSERVATION_YEAR_UNKNOWN;
	snapshot.can_be_profitable = PROFITABILITY_UNKNOWN;
	snapshot.perceived_value = PERCEIVED_VALUE_UNKNOWN;
	
	return snapshot;
}
