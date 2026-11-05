#ifndef OPENHO_PLANET_VIEW_H
#define OPENHO_PLANET_VIEW_H

#include <cstdint>
#include <string>
#include "enums.h"

using PlanetViewCoord = double;

typedef int32_t PlayerID;

class Planet;
class ColonizedPlanet;

/// A player's knowledge snapshot for one authoritative Planet.
///
/// Identity and geometry are supplied by the associated Planet. This class
/// contains only observer-specific, potentially stale information and never
/// owns a ColonizedPlanet.
class PlanetView
{
public:
    uint32_t id;
    std::string name;
    PlanetViewCoord x;
    PlanetViewCoord y;
    PlayerID as_seen_by;

    double apparent_temperature;
    bool has_observed_gravity;
    int32_t metal;
    PlayerID apparent_owner;
    int32_t apparent_population;
    int32_t observation_year;

    // Populated only by an OWNED_COLONY observation. These are snapshots,
    // not a second ColonizedPlanet object.
    int32_t income;
    double funding_fraction;
    double mining_fraction;
    double terraforming_fraction;

    int32_t can_be_profitable;
    int32_t perceived_value;

    PlanetView(const Planet& planet, PlayerID player_id);

    /// Battle information that may contain an incorrect apparent owner.
    void set_obs_lost_battle_quickly(PlayerID apparent_owner, int32_t observation_year);

    /// Battle information with the actual owner and population.
    void set_obs_lost_battle_eventually(const Planet& planet, int32_t observation_year);

    /// Full environmental observation from a friendly stationed fleet.
    void set_obs_friendly_ship_visit(const Planet& planet, int32_t observation_year);

    /// Complete observation for a colony owned by the observing player.
    void set_obs_fullcolony(const Planet& planet,
                            const ColonizedPlanet& colony,
                            int32_t observation_year);
};

#endif // OPENHO_PLANET_VIEW_H
