#include "planet.h"
#include "player.h"
#include "game_formulas.h"

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
	PlayerID p_owner)
	: id(p_id),
	  name(p_name),
	  x(p_x),
	  y(p_y),
	  true_gravity(p_true_gravity),
	  true_temperature(p_true_temperature),
	  metal(p_metal),
	  owner(p_owner),
	  nova_state(PLANET_NORMAL)
{
}

// ============================================================================
// ColonizedPlanet Implementation
// ============================================================================
ColonizedPlanet::ColonizedPlanet(
	Planet* p_base,
	Player* p_owner,
	int32_t p_pop,
	int32_t p_income,
	double p_funding,
	PlanetaryBudgetSplit p_budget)
	: base_planet(p_base),
	  owner_player(p_owner),
	  planet_funding_fraction(p_funding),
	  population(p_pop),
	  income(p_income),
	  budget_split(p_budget),
	  apparent_gravity(GameFormulas::calculate_apparent_gravity(
	      p_owner->get_ideal_gravity(), p_base->true_gravity)),
	  apparent_temperature(GameFormulas::calculate_apparent_temperature(
	      p_owner->get_ideal_temperature(), p_base->true_temperature))
{
	// Update the base planet's owner to this player
	base_planet->owner = p_owner->id;
}

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
