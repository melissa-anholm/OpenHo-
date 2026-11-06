#ifndef OPENHO_PLANET_DISTANCE_H
#define OPENHO_PLANET_DISTANCE_H

#include <cstdint>

struct PlanetDistance
{
    uint32_t destination_planet_id = 0;
    double distance = 0.0;
};

#endif // OPENHO_PLANET_DISTANCE_H
