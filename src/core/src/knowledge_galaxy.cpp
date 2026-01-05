#include "knowledge_galaxy.h"
#include "galaxy.h"
#include "planet.h"
#include "player.h"

// ============================================================================
// KnowledgeGalaxy Implementation
// ============================================================================

KnowledgeGalaxy::KnowledgeGalaxy(const Galaxy& galaxy, PlayerID player_id)
	: real_galaxy(&galaxy),
	  player_id(player_id)
{
	// Initialize KnowledgePlanets for all planets in the galaxy
	// Start with partial info (id, name, coordinates only)
	planets.reserve(galaxy.planets.size());
	
	for (const auto& planet : galaxy.planets) {
		planets.emplace_back(planet, player_id);
	}
}

KnowledgePlanet* KnowledgeGalaxy::get_planet(uint32_t planet_id)
{
	if (planet_id < planets.size()) {
		return &planets[planet_id];
	}
	return nullptr;
}

const KnowledgePlanet* KnowledgeGalaxy::get_planet(uint32_t planet_id) const
{
	if (planet_id < planets.size()) {
		return &planets[planet_id];
	}
	return nullptr;
}

void KnowledgeGalaxy::observe_planet(uint32_t planet_id, const Planet& real_planet, const Player* observer, int32_t current_year)
{
	if (planet_id < planets.size()) {
		planets[planet_id].observe_planet(real_planet, observer, current_year);
	}
}

const Planet* KnowledgeGalaxy::get_real_planet(uint32_t planet_id) const
{
	if (real_galaxy && planet_id < real_galaxy->planets.size()) {
		return &real_galaxy->planets[planet_id];
	}
	return nullptr;
}
