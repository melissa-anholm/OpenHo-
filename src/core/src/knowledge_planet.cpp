#include "knowledge_planet.h"
#include "planet.h"
#include "player.h"
#include "game_formulas.h"

// ============================================================================
// PlanetSnapshot Implementation
// ============================================================================

PlanetSnapshot PlanetSnapshot::partial_info(const Planet& planet, PlayerID player_id)
{
	PlanetSnapshot snapshot;
	snapshot.id = planet.id;
	snapshot.name = planet.name;
	snapshot.x = planet.x;
	snapshot.y = planet.y;
	snapshot.as_seen_by = player_id;
	
	// Unknown fields for partial info
	snapshot.apparent_temperature = UNKNOWN_DOUBLE_VALUE;
	snapshot.apparent_gravity = UNKNOWN_DOUBLE_VALUE;
	snapshot.metal = UNKNOWN_INT_VALUE;
	snapshot.apparent_owner = OWNER_UNKNOWN;
	snapshot.population = UNKNOWN_INT_VALUE;
	snapshot.income = INCOME_UNKNOWN;
	snapshot.observation_year = 0;
	snapshot.can_be_profitable = false;
	snapshot.perceived_value = 0;
	
	return snapshot;
}

PlanetSnapshot PlanetSnapshot::full_info(const Planet& planet, PlayerID player_id, Player* owner_player)
{
	PlanetSnapshot snapshot;
	snapshot.id = planet.id;
	snapshot.name = planet.name;
	snapshot.x = planet.x;
	snapshot.y = planet.y;
	snapshot.as_seen_by = player_id;
	
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
	snapshot.observation_year = 0;
	snapshot.can_be_profitable = false;
	snapshot.perceived_value = 0;
	
	return snapshot;
}
