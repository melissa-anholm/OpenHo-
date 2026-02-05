#include "galaxy.h"
#include "game.h"
#include "rng.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>

int main()
{
std::cout << "=== Cluster Galaxy Wedge-Based Home Planet Selection Test ===" << std::endl;

// Create a deterministic RNG with different seed
DeterministicRNG rng(98765, 0);

// Test parameters
uint32_t n_planets = 150;
uint32_t n_players = 3;
double angular_offset = 45.0;  // 45 degree offset

std::cout << "\nTest Parameters:" << std::endl;
std::cout << "  Total planets: " << n_planets << std::endl;
std::cout << "  Number of players (wedges): " << n_players << std::endl;
std::cout << "  Angular offset: " << angular_offset << " degrees" << std::endl;

// Generate random planet coordinates
std::vector<PlanetCoord> all_coords;
double radius = 100.0;

std::cout << "\nGenerating " << n_planets << " random planets in a circle..." << std::endl;
for (uint32_t i = 0; i < n_planets; ++i)
{
double angle = rng.nextDouble() * 2.0 * M_PI;
double r = rng.nextDouble() * radius;
double x = r * std::cos(angle);
double y = r * std::sin(angle);
all_coords.push_back({x, y});
}

// Convert angular offset to radians
double offset_rad = angular_offset * M_PI / 180.0;
double wedge_angle = 2.0 * M_PI / n_players;

std::cout << "\nWedge Analysis:" << std::endl;
std::cout << "  Wedge angle: " << (wedge_angle * 180.0 / M_PI) << " degrees" << std::endl;

// For each wedge, count planets
for (uint32_t wedge_idx = 0; wedge_idx < n_players; ++wedge_idx)
{
double wedge_start = offset_rad + wedge_idx * wedge_angle;
double wedge_end = wedge_start + wedge_angle;

std::cout << "\n  Wedge " << (wedge_idx + 1) << ":" << std::endl;
std::cout << "    Start angle: " << (wedge_start * 180.0 / M_PI) << "°" << std::endl;
std::cout << "    End angle: " << (wedge_end * 180.0 / M_PI) << "°" << std::endl;

// Count planets in this wedge
uint32_t count = 0;
for (const auto& coord : all_coords)
{
double angle = std::atan2(coord.second, coord.first);
if (angle < 0) angle += 2.0 * M_PI;

bool in_wedge = false;
if (wedge_end <= 2.0 * M_PI)
{
in_wedge = (angle >= wedge_start && angle < wedge_end);
}
else
{
double wrapped_end = wedge_end - 2.0 * M_PI;
in_wedge = (angle >= wedge_start || angle < wrapped_end);
}

if (in_wedge) count++;
}

std::cout << "    Planets in wedge: " << count << std::endl;

if (count > 0)
{
// Select a random planet from this wedge
std::vector<PlanetCoord> planets_in_wedge;
for (const auto& coord : all_coords)
{
double angle = std::atan2(coord.second, coord.first);
if (angle < 0) angle += 2.0 * M_PI;

bool in_wedge = false;
if (wedge_end <= 2.0 * M_PI)
{
in_wedge = (angle >= wedge_start && angle < wedge_end);
}
else
{
double wrapped_end = wedge_end - 2.0 * M_PI;
in_wedge = (angle >= wedge_start || angle < wrapped_end);
}

if (in_wedge) planets_in_wedge.push_back(coord);
}

uint32_t random_idx = rng.nextInt32Range(0, planets_in_wedge.size() - 1);
PlanetCoord home_planet = planets_in_wedge[random_idx];
std::cout << "    Home planet: (" << std::fixed << std::setprecision(2) 
          << home_planet.first << ", " << home_planet.second << ")" << std::endl;
}
}

std::cout << "\n=== Test Complete ===" << std::endl;

return 0;
}
