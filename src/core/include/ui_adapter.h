#ifndef OPENHO_UI_ADAPTER_H
#define OPENHO_UI_ADAPTER_H

#include "economic_state.h"
#include "error_codes.h"
#include <array>
#include <cstdint>
#include <string>
#include <vector>

class GameState;

/// UI-safe copy of one player's knowledge of a planet.
/// It contains no pointers into the game engine and may be retained by a UI.
struct UiPlanetSnapshot
{
    uint32_t id = 0;
    std::string name;
    double x = 0.0;
    double y = 0.0;
    bool has_observation = false;
    double apparent_temperature = 0.0;
    bool has_observed_gravity = false;
    int32_t apparent_metal = 0;
    int32_t apparent_owner = 0;
    int32_t apparent_population = 0;
    int32_t observation_year = 0;
    int32_t income = 0;
    int32_t can_be_profitable = 0;
    int32_t perceived_value = 0;
};

/// UI-safe copy of one colony's currently authoritative economic information.
struct UiColonySnapshot
{
    uint32_t planet_id = 0;
    std::string planet_name;
    int32_t population = 0;
    int32_t income = 0;
    double funding_fraction = 0.0;
    double mining_fraction = 0.0;
    double terraforming_fraction = 0.0;
};

/// UI-safe copy of the player state needed by the first UI vertical slice.
struct UiPlayerSnapshot
{
    uint32_t id = 0;
    std::string name;
    bool eliminated = false;
    bool ready = false;
    int64_t money = 0;
    int64_t money_income = 0;
    int64_t effective_income = 0;
    int64_t metal_reserve = 0;
    int64_t metal_income = 0;
    int64_t dip_amount = 0;
    bool dip_persistent = false;
    double savings_fraction = 0.0;
    double research_fraction = 0.0;
    double planets_fraction = 0.0;
    std::array<double, 6> research_fractions{};
    std::vector<UiColonySnapshot> colonies;
};

/// Complete display snapshot for one observing player.
struct UiGameSnapshot
{
    uint32_t observing_player_id = 0;
    uint32_t current_turn = 0;
    uint32_t current_year = 2000;
    bool all_players_ready = false;
    std::vector<UiPlanetSnapshot> planets;
    std::vector<UiPlayerSnapshot> players;
};

/// Final economic choices submitted by a UI for the current turn boundary.
struct UiEconomicSubmission
{
    double savings_fraction = 0.0;
    double research_fraction = 0.0;
    double planets_fraction = 0.0;
    std::array<double, 6> research_fractions{};
    std::vector<EconomicColonyAllocation> colony_allocations;
    std::vector<uint32_t> planets_to_abandon;
    int64_t dip_amount = 0;
    bool dip_persistent = false;
};

/// Portable C++ boundary between a frontend and GameState.
///
/// The adapter returns copies for display and accepts validated value commands.
/// A frontend must not retain pointers or references to GameState domain objects.
class GameUiAdapter
{
public:
    explicit GameUiAdapter(GameState& game_state);

    [[nodiscard]] UiGameSnapshot snapshot(uint32_t observing_player_id) const;

    [[nodiscard]] ErrorCode set_spending_allocation(
        uint32_t player_id,
        double savings_fraction,
        double research_fraction,
        double planets_fraction,
        const std::array<double, 6>& research_fractions);
    [[nodiscard]] ErrorCode set_planet_allocation(
        uint32_t player_id,
        uint32_t planet_id,
        double mining_fraction,
        double terraforming_fraction);
    [[nodiscard]] ErrorCode submit_economic_actions(
        uint32_t player_id,
        const UiEconomicSubmission& submission);

    [[nodiscard]] ErrorCode queue_planet_abandonment(
        uint32_t player_id, uint32_t planet_id);
    [[nodiscard]] ErrorCode cancel_planet_abandonment(
        uint32_t player_id, uint32_t planet_id);
    [[nodiscard]] ErrorCode mark_ready(uint32_t player_id);
    [[nodiscard]] ErrorCode mark_unready(uint32_t player_id);
    [[nodiscard]] ErrorCode process_turn();

private:
    GameState& game_state;
};

#endif // OPENHO_UI_ADAPTER_H
