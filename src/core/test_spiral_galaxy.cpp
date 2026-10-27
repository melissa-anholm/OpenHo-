#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

#include "galaxy.h"
#include "game_constants.h"
#include "rng.h"

namespace
{
void print_parameter_diagnostics()
{
    std::cout << "Spiral parameter diagnostics:\n";

    const double test_angles[] = {M_PI / 4.0, M_PI / 2.0, 3.0 * M_PI / 4.0, M_PI};
    for (double delta_theta : test_angles)
    {
        const double a = 100.0 / std::sqrt(delta_theta);
        const double core_radius = 20.0;
        const double ratio = 3.0;
        const double outer_radius = core_radius * ratio;
        const double theta_core = (core_radius / a) * (core_radius / a);
        const double r_average = (core_radius + outer_radius) / 2.0;
        const double angular_distance = r_average * (delta_theta - theta_core);
        const double radial_distance = outer_radius - core_radius;
        const double arc_length = std::sqrt(
            radial_distance * radial_distance + angular_distance * angular_distance);

        std::cout << "  Angular span: " << std::fixed << std::setprecision(4)
                  << delta_theta << " rad (" << delta_theta * 180.0 / M_PI << " deg)\n"
                  << "    Tightness (a): " << a << "\n"
                  << "    Example arm length: " << arc_length << " units\n"
                  << "    Estimated planets per arm: "
                  << static_cast<int>(arc_length / GameConstants::min_planet_distance)
                  << "\n";
    }

    std::cout << "\nFermat spiral samples:\n";
    const double sample_a = 100.0 / std::sqrt(M_PI / 2.0);
    for (int i = 0; i <= 4; ++i)
    {
        const double theta = (i / 4.0) * (M_PI / 2.0);
        const double radius = sample_a * std::sqrt(theta);
        std::cout << "  theta = " << std::setprecision(4) << theta
                  << ", radius = " << radius << "\n";
    }

    std::cout << "\nConfigured generation ranges:\n"
              << "  Angular span: [45, 180] degrees\n"
              << "  Arm-length/core-radius ratio: [2.0, 6.0]\n"
              << "  Core overlap floor: 6.0 units\n";
}

void print_generation_diagnostics(const std::vector<PlanetCoord>& coords)
{
    std::cout << "\nGenerated coordinates: " << coords.size() << "\n";
    if (coords.empty())
        return;

    double min_radius = std::numeric_limits<double>::max();
    double max_radius = 0.0;
    double average_radius = 0.0;
    for (const auto& coord : coords)
    {
        const double radius = std::hypot(coord.first, coord.second);
        min_radius = std::min(min_radius, radius);
        max_radius = std::max(max_radius, radius);
        average_radius += radius;
    }
    average_radius /= static_cast<double>(coords.size());

    std::cout << "  Min distance from center: " << min_radius << "\n"
              << "  Max distance from center: " << max_radius << "\n"
              << "  Average distance from center: " << average_radius << "\n";

    uint32_t spacing_violations = 0;
    for (size_t i = 0; i < coords.size(); ++i)
    {
        for (size_t j = i + 1; j < coords.size(); ++j)
        {
            const double dx = coords[i].first - coords[j].first;
            const double dy = coords[i].second - coords[j].second;
            if (std::hypot(dx, dy) < GameConstants::min_planet_distance - 0.01)
                ++spacing_violations;
        }
    }

    if (spacing_violations == 0)
        std::cout << "  Minimum-spacing check: PASS\n";
    else
        std::cout << "  Minimum-spacing check: FAIL ("
                  << spacing_violations << " violations)\n";
}
} // namespace

int main()
{
    try
    {
        constexpr uint32_t planet_count = 150;
        constexpr uint32_t player_count = 3;
        constexpr double density = 0.5;
        constexpr uint64_t seed = 12345;

        GalaxyGenerationParams params(
            planet_count,
            player_count,
            density,
            GALAXY_SPIRAL,
            seed);
        DeterministicRNG rng(seed, seed + 1);

        std::cout << "=== Spiral Galaxy Coordinate Diagnostic ===\n\n"
                  << "Requested planets: " << params.n_planets << "\n"
                  << "Players / spiral arms: " << params.n_players << "\n"
                  << "Density: " << params.density << "\n"
                  << "Seed: " << params.seed << "\n\n";

        print_parameter_diagnostics();
        const std::vector<PlanetCoord> coords =
            Galaxy::generate_coordinates_spiral(params, rng);
        print_generation_diagnostics(coords);

        std::cout << "\n=== Diagnostic Complete ===\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "ERROR: " << error.what() << '\n';
        return 1;
    }
}
