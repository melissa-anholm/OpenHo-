#include "planet_view.h"
#include "planet.h"
#include "colonized_planet.h"

PlanetView::PlanetView(const Planet& planet, PlayerID player_id)
    : id(planet.id),
      name(planet.name),
      x(planet.x),
      y(planet.y),
      as_seen_by(player_id),
      apparent_temperature(UNKNOWN_DOUBLE_VALUE),
      has_observed_gravity(false),
      metal(UNKNOWN_INT_VALUE),
      apparent_owner(OWNER_UNKNOWN),
      apparent_population(POPULATION_UNKNOWN),
      observation_year(OBSERVATION_YEAR_UNKNOWN),
      income(0),
      can_be_profitable(PROFITABILITY_UNKNOWN),
      perceived_value(PERCEIVED_VALUE_UNKNOWN)
{ }

void PlanetView::set_obs_lost_battle_quickly(PlayerID owner, int32_t current_year)
{
    observation_year = current_year;
    apparent_owner = owner;
}

void PlanetView::set_obs_lost_battle_eventually(const Planet& planet, int32_t current_year)
{
    observation_year = current_year;
    apparent_owner = planet.owner;
    apparent_population = planet.population;
}

void PlanetView::set_obs_friendly_ship_visit(const Planet& planet, int32_t current_year)
{
    set_obs_lost_battle_eventually(planet, current_year);
    has_observed_gravity = true;
    apparent_temperature = planet.true_temperature;
    metal = planet.metal;
}

void PlanetView::set_obs_fullcolony(const Planet& planet,
                                    const ColonizedPlanet& colony,
                                    int32_t current_year)
{
    set_obs_friendly_ship_visit(planet, current_year);
    apparent_population = planet.population;
    income = colony.get_income();
}
