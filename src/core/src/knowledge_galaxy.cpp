#include "knowledge_galaxy.h"
#include "galaxy.h"
#include "planet.h"
#include "game_constants.h"

KnowledgeGalaxy::KnowledgeGalaxy(Galaxy& galaxy, PlayerID observer_id)
    : real_galaxy(&galaxy),
      distance_matrix(galaxy.distance_matrix),
      player_id(observer_id),
      space_real_planet(nullptr),
      space_planet_view(nullptr)
{
    // Planet owns the persistent observer-specific snapshot. This facade only
    // ensures that the current observer has a view record for every planet.
    for (auto& planet : galaxy.planets)
        planet.get_or_create_view(observer_id);

    space_real_planet = new Planet(
        UINT32_MAX,
        "Transit",
        static_cast<double>(INT32_MAX),
        static_cast<double>(INT32_MAX),
        1.0,
        GameConstants::best_perceived_temperature_K,
        0,
        observer_id);
    space_planet_view = new PlanetView(*space_real_planet, observer_id);
}

KnowledgeGalaxy::~KnowledgeGalaxy()
{
    delete space_planet_view;
    space_planet_view = nullptr;

    delete space_real_planet;
    space_real_planet = nullptr;
}

PlanetView* KnowledgeGalaxy::get_planet(uint32_t planet_id)
{
    if (!real_galaxy || planet_id >= real_galaxy->planets.size())
        return nullptr;
    return real_galaxy->planets[planet_id].get_view(player_id);
}

const PlanetView* KnowledgeGalaxy::get_planet(uint32_t planet_id) const
{
    if (!real_galaxy || planet_id >= real_galaxy->planets.size())
        return nullptr;
    return real_galaxy->planets[planet_id].get_view(player_id);
}

size_t KnowledgeGalaxy::get_planet_count() const
{
    return real_galaxy ? real_galaxy->planets.size() : 0;
}

const Planet* KnowledgeGalaxy::get_real_planet(uint32_t planet_id) const
{
    if (!real_galaxy || planet_id >= real_galaxy->planets.size())
        return nullptr;
    return &real_galaxy->planets[planet_id];
}

double KnowledgeGalaxy::get_distance(uint32_t from_id, uint32_t to_id) const
{
    return distance_matrix.at(from_id).at(to_id);
}
