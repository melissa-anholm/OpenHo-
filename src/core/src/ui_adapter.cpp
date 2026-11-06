#include "ui_adapter.h"

#include "game.h"
#include "planet.h"
#include "planet_view.h"
#include "player.h"
#include "game_constants.h"
#include <cmath>

namespace
{
UiPlanetSnapshot make_planet_snapshot(const Planet& planet, uint32_t observing_player_id)
{
    UiPlanetSnapshot snapshot;
    snapshot.id = planet.id;
    snapshot.name = planet.name;
    snapshot.x = planet.x;
    snapshot.y = planet.y;

    const PlanetView* view = planet.get_view(static_cast<PlayerID>(observing_player_id));
    if (!view)
        return snapshot;

    snapshot.has_observation = true;
    snapshot.apparent_temperature = view->apparent_temperature;
    snapshot.has_observed_gravity = view->has_observed_gravity;
    snapshot.apparent_metal = view->metal;
    snapshot.apparent_owner = view->apparent_owner;
    snapshot.apparent_population = view->apparent_population;
    snapshot.observation_year = view->observation_year;
    snapshot.income = view->income;
    snapshot.can_be_profitable = view->can_be_profitable;
    snapshot.perceived_value = view->perceived_value;
    return snapshot;
}

UiColonySnapshot make_colony_snapshot(const GameState& game_state,
                                      const ColonizedPlanet& colony)
{
    UiColonySnapshot snapshot;
    snapshot.planet_id = colony.get_id();
    snapshot.population = colony.get_population();
    snapshot.income = colony.get_income();
    snapshot.funding_fraction = colony.get_funding_fraction();
    snapshot.mining_fraction = colony.get_mining_fraction();
    snapshot.terraforming_fraction = colony.get_terraforming_fraction();

    const Planet* planet = game_state.get_planet(snapshot.planet_id);
    if (planet)
        snapshot.planet_name = planet->name;
    return snapshot;
}

bool valid_research_fractions(const std::array<double, 6>& fractions)
{
    double total = 0.0;
    for (double fraction : fractions)
    {
        if (!std::isfinite(fraction) || fraction < 0.0)
            return false;
        total += fraction;
    }
    return std::abs(total - 1.0) <= GameConstants::Allocation_Sum_Tolerance;
}
}

GameUiAdapter::GameUiAdapter(GameState& game_state)
    : game_state(game_state)
{
}

UiGameSnapshot GameUiAdapter::snapshot(uint32_t observing_player_id) const
{
    UiGameSnapshot snapshot;
    snapshot.observing_player_id = observing_player_id;
    snapshot.current_turn = game_state.get_current_turn();
    snapshot.current_year = game_state.get_current_year();
    snapshot.all_players_ready = game_state.all_players_ready();

    const Galaxy& galaxy = game_state.get_galaxy();
    snapshot.planets.reserve(galaxy.planets.size());
    for (const Planet& planet : galaxy.planets)
        snapshot.planets.push_back(make_planet_snapshot(planet, observing_player_id));

    const std::vector<Player>& players = game_state.get_players();
    snapshot.players.reserve(players.size());
    for (const Player& player : players)
    {
        UiPlayerSnapshot player_snapshot;
        player_snapshot.id = player.id;
        player_snapshot.name = player.name;
        player_snapshot.eliminated = player.is_eliminated();
        player_snapshot.ready = game_state.get_player_turn_ready_status(player.id) == 1;
        if (player.id == observing_player_id)
        {
            player_snapshot.money = player.get_money();
            player_snapshot.money_income = player.get_money_income();
            player_snapshot.effective_income = player.get_effective_income();
            player_snapshot.metal_reserve = player.get_metal();
            player_snapshot.metal_income = player.get_metal_income();
            player_snapshot.dip_amount = player.get_dip_amount();
            player_snapshot.dip_persistent = player.is_dip_persistent();

            const Player::MoneyAllocation& allocation = player.get_spending_allocation();
            player_snapshot.savings_fraction = allocation.savings_fraction;
            player_snapshot.research_fraction = allocation.research_fraction;
            player_snapshot.planets_fraction = allocation.planets_fraction;
            player_snapshot.research_fractions = {
                allocation.research.research_range_fraction,
                allocation.research.research_speed_fraction,
                allocation.research.research_weapons_fraction,
                allocation.research.research_shields_fraction,
                allocation.research.research_mini_fraction,
                allocation.research.research_radical_fraction};

            const std::vector<ColonizedPlanet>& colonies = player.get_colonized_planets();
            player_snapshot.colonies.reserve(colonies.size());
            for (const ColonizedPlanet& colony : colonies)
                player_snapshot.colonies.push_back(make_colony_snapshot(game_state, colony));
        }

        snapshot.players.push_back(std::move(player_snapshot));
    }
    return snapshot;
}

ErrorCode GameUiAdapter::set_spending_allocation(
    uint32_t player_id,
    double savings_fraction,
    double research_fraction,
    double planets_fraction,
    const std::array<double, 6>& research_fractions)
{
    if (!valid_research_fractions(research_fractions))
        return ErrorCode::INVALID_ALLOCATION;

    Player::MoneyAllocation allocation{};
    allocation.savings_fraction = savings_fraction;
    allocation.research_fraction = research_fraction;
    allocation.planets_fraction = planets_fraction;
    allocation.research.research_range_fraction = research_fractions[0];
    allocation.research.research_speed_fraction = research_fractions[1];
    allocation.research.research_weapons_fraction = research_fractions[2];
    allocation.research.research_shields_fraction = research_fractions[3];
    allocation.research.research_mini_fraction = research_fractions[4];
    allocation.research.research_radical_fraction = research_fractions[5];

    const ErrorCode validation = game_state.check_player_set_spending_allocation(
        player_id, savings_fraction, research_fraction, planets_fraction);
    if (validation != ErrorCode::SUCCESS)
        return validation;

    game_state.set_money_allocation(player_id, allocation);
    return ErrorCode::SUCCESS;
}

ErrorCode GameUiAdapter::set_planet_allocation(
    uint32_t player_id,
    uint32_t planet_id,
    double mining_fraction,
    double terraforming_fraction)
{
    const ErrorCode validation = game_state.check_player_set_planet_allocation(
        player_id, planet_id, mining_fraction, terraforming_fraction);
    if (validation != ErrorCode::SUCCESS)
        return validation;

    game_state.set_planet_allocation(
        player_id, planet_id, mining_fraction, terraforming_fraction);
    return ErrorCode::SUCCESS;
}

ErrorCode GameUiAdapter::submit_economic_actions(
    uint32_t player_id,
    const UiEconomicSubmission& submission)
{
    EconomicActionSubmission command;
    command.player_id = player_id;
    command.target_turn = game_state.get_current_turn();
    command.savings_fraction = submission.savings_fraction;
    command.research_fraction = submission.research_fraction;
    command.planets_fraction = submission.planets_fraction;
    command.research_fractions = submission.research_fractions;
    command.colony_allocations = submission.colony_allocations;
    command.planets_to_abandon = submission.planets_to_abandon;
    command.dip_amount = submission.dip_amount;
    command.dip_persistent = submission.dip_persistent;
    return game_state.submit_economic_action(player_id, command);
}

ErrorCode GameUiAdapter::queue_planet_abandonment(
    uint32_t player_id, uint32_t planet_id)
{
    return game_state.queue_planet_abandonment(player_id, planet_id);
}

ErrorCode GameUiAdapter::cancel_planet_abandonment(
    uint32_t player_id, uint32_t planet_id)
{
    return game_state.cancel_planet_abandonment(player_id, planet_id);
}

ErrorCode GameUiAdapter::mark_ready(uint32_t player_id)
{
    return game_state.mark_player_turn_ready(player_id);
}

ErrorCode GameUiAdapter::mark_unready(uint32_t player_id)
{
    return game_state.mark_player_turn_unready(player_id);
}

ErrorCode GameUiAdapter::process_turn()
{
    if (!game_state.all_players_ready())
        return ErrorCode::GAME_NOT_READY;
    game_state.process_turn();
    return ErrorCode::SUCCESS;
}
