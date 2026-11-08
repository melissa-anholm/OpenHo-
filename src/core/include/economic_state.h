#ifndef OPENHO_ECONOMIC_STATE_H
#define OPENHO_ECONOMIC_STATE_H

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace EconomicAllocation
{
/// Set one normalized fraction and proportionally rebalance the supplied peers.
/// The selected fraction and all peer fractions must initially sum to 1.0.
bool adjust_fraction_proportionally(
    double value,
    double& selected,
    const std::vector<double*>& peer_fractions,
    std::string& error);
}

struct EconomicColonyAllocation
{
    std::string planet_name;
    double funding_fraction = 0.0;
    double mining_fraction = 0.0;
    double terraforming_fraction = 0.0;
};

/// Final, non-ordered economic choices submitted for a target turn.
struct EconomicActionSubmission
{
    uint32_t player_id = 0;
    uint32_t target_turn = 0;
    double savings_fraction = 0.0;
    double research_fraction = 0.0;
    double planets_fraction = 0.0;
    std::array<double, 6> research_fractions{};
    std::vector<EconomicColonyAllocation> colony_allocations;
    std::vector<std::string> planets_to_abandon;
    int64_t dip_amount = 0;
    bool dip_persistent = false;
};

struct FleetMoveAction
{
    uint32_t fleet_id = 0;
    std::string destination_planet_name;
};

/// Complete set of fleet movement orders submitted for one turn.
/// Fleet location and transit details remain authoritative on Fleet; these are
/// only commands to be applied at the next fleet-processing phase.
struct FleetActionSubmission
{
    uint32_t player_id = 0;
    uint32_t target_turn = 0;
    std::vector<FleetMoveAction> moves;
    std::vector<uint32_t> scraps;
};

struct EconomicColonyAfterState
{
    std::string planet_name;
    int32_t population = 0;
    int32_t income = 0;
};

/// Server-produced result of economic processing for one player and turn.
struct EconomicAfterState
{
    uint32_t turn = 0;
    int64_t planetary_income = 0;
    int64_t interest_income = 0;
    int64_t net_income = 0;
    int64_t effective_income = 0;
    int64_t savings_after_processing = 0;
    int64_t metal_reserve_after_processing = 0;
    std::vector<EconomicColonyAfterState> colonies;
};

#endif // OPENHO_ECONOMIC_STATE_H
