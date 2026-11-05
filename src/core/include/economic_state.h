#ifndef OPENHO_ECONOMIC_STATE_H
#define OPENHO_ECONOMIC_STATE_H

#include <array>
#include <cstdint>
#include <vector>

struct EconomicColonyAllocation
{
    uint32_t planet_id = 0;
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
    int64_t dip_amount = 0;
    bool dip_persistent = false;
};

struct EconomicColonyAfterState
{
    uint32_t planet_id = 0;
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
