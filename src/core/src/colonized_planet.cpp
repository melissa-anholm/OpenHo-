#include "colonized_planet.h"
#include "planet.h"
#include "player.h"
#include "game_formulas.h"

// ============================================================================
// ColonizedPlanet Implementation
// ============================================================================

ColonizedPlanet::ColonizedPlanet(
	Planet* p_base,
	Player* p_owner,
	int32_t p_pop,
	int32_t p_income,
	double p_funding,
	PlanetaryBudgetSplit p_budget,
	int32_t p_desirability)
	: base_planet(p_base),
	  owner_player(p_owner),
	  planet_funding_fraction(p_funding),
	  population(p_pop),
	  income(p_income),
	  budget_split(p_budget),
	  apparent_gravity(GameFormulas::calculate_apparent_gravity(
	      p_owner->get_ideal_gravity(), p_base->true_gravity)),
	  apparent_temperature(GameFormulas::calculate_apparent_temperature(
	      p_owner->get_ideal_temperature(), p_base->true_temperature)),
	  desirability(p_desirability)
{
	// Update the base planet's owner to this player
	base_planet->owner = p_owner->id;
}

void ColonizedPlanet::update_desirability()
{
	// Placeholder: All planets are maximally desirable (3)
	desirability = 3;
}
