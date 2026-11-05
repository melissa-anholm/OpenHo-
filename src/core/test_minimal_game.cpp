#include "game.h"
#include "game_setup.h"
#include "game_formulas.h"
#include "save_file.h"
#include "rng.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

namespace
{
std::unique_ptr<GameState> create_two_player_game()
{
    std::vector<PlayerSetup> player_setups = {
        {"Player One", GENDER_F, PLAYER_HUMAN, 100, START_NORMAL},
        {"Player Two", GENDER_M, PLAYER_HUMAN, 100, START_NORMAL}
    };
    GalaxyGenerationParams galaxy_params(40, 2, 0.5, GALAXY_RANDOM, 42);
    GameSetup setup(galaxy_params, player_setups);
    return std::make_unique<GameState>(setup);
}

std::unique_ptr<GameState> create_ship_game()
{
    std::vector<PlayerSetup> player_setups = {
        {"Ship Builder", GENDER_F, PLAYER_HUMAN, 100, START_NORMAL}
    };
    GalaxyGenerationParams galaxy_params(20, 1, 0.5, GALAXY_RANDOM, 7);
    GameSetup setup(galaxy_params, player_setups);
    return std::make_unique<GameState>(setup);
}

void test_game_initialization_and_readiness()
{
    auto game = create_two_player_game();

    assert(game->get_num_players() == 2);
    assert(game->get_galaxy().planets.size() == 40);
    assert(game->get_current_turn() == 0);
    assert(game->get_current_year() == 2000);
    assert(!game->all_players_ready());

    for (const Player& player : game->get_players())
    {
        assert(player.get_colonized_planets().size() == 1);
        assert(player.get_ideal_temperature() > 0.0);
        assert(player.get_ideal_gravity() > 0.0);
        assert(player.get_colonized_planets().front().get_population() > 0);
        assert(player.get_colonized_planets().front().get_income() > 0);
        assert(player.get_colonized_planets().front().get_metal() > 0);
    }

    assert(game->mark_player_turn_ready(1) == ErrorCode::SUCCESS);
    assert(game->get_player_turn_ready_status(1) == 1);
    assert(game->get_player_turn_ready_status(2) == 0);
    assert(!game->all_players_ready());
    assert(game->mark_player_turn_ready(2) == ErrorCode::SUCCESS);
    assert(game->all_players_ready());
}

void test_dip_and_turn_processing()
{
    auto game = create_two_player_game();
    const Player& initial_player = game->get_players().front();
    const ColonizedPlanet& initial_colony = initial_player.get_colonized_planets().front();
    const int32_t initial_population = initial_colony.get_population();
    const int32_t initial_income = initial_colony.get_income();
    const int64_t expected_growth = GameFormulas::calculate_population_growth(
        initial_population,
        initial_colony.get_true_temperature(),
        initial_colony.get_true_gravity(),
        initial_player.get_ideal_temperature(),
        initial_player.get_ideal_gravity());

    Player::MoneyAllocation savings_only = game->get_money_allocation(1);
    savings_only.savings_fraction = 1.0;
    savings_only.research_fraction = 0.0;
    savings_only.planets_fraction = 0.0;
    game->set_money_allocation(1, savings_only);
    game->set_money_allocation(2, savings_only);

    const int64_t initial_savings = game->get_players().front().get_money();
    constexpr int64_t dip_amount = 100000;
    const int64_t maximum_initial_dip = GameFormulas::calculate_available_build_money(
        initial_savings, initial_income, 1);
    game->set_dip_amount(1, dip_amount);
    const int64_t persistent_dip_amount = std::min(dip_amount, maximum_initial_dip);
    assert(game->get_players().front().get_money() ==
           initial_savings - persistent_dip_amount);

    game->set_dip_amount(1, dip_amount * 2);
    const int64_t increased_persistent_dip = std::min(dip_amount * 2, maximum_initial_dip);
    assert(game->get_players().front().get_money() ==
           initial_savings - increased_persistent_dip);
    game->clear_dip_amount(1);
    assert(game->get_players().front().get_money() == initial_savings);
    game->set_dip_amount(1, dip_amount);

    constexpr int64_t one_time_dip_amount = 50000;
    const int64_t second_player_income =
        game->get_players()[1].get_colonized_planets().front().get_income();
    const int64_t maximum_one_time_dip = GameFormulas::calculate_available_build_money(
        game->get_players()[1].get_money(), second_player_income, 1);
    game->set_one_time_dip_amount(2, one_time_dip_amount);
    const int64_t actual_one_time_dip = std::min(one_time_dip_amount, maximum_one_time_dip);

    assert(game->get_dip_amount(1) == persistent_dip_amount);
    assert(game->is_dip_persistent(1));
    assert(game->get_dip_amount(2) == actual_one_time_dip);
    assert(!game->is_dip_persistent(2));

    assert(game->mark_player_turn_ready(1) == ErrorCode::SUCCESS);
    assert(game->mark_player_turn_ready(2) == ErrorCode::SUCCESS);
    game->process_turn();

    assert(game->get_current_turn() == 1);
    assert(game->get_current_year() == 2010);
    assert(!game->all_players_ready());

    const Player& first_player_after_turn = game->get_players().front();
    const ColonizedPlanet& first_colony_after_turn =
        first_player_after_turn.get_colonized_planets().front();
    assert(first_colony_after_turn.get_population() == initial_population + expected_growth);
    assert(first_colony_after_turn.get_income() > initial_income);
    assert(first_player_after_turn.get_money_income() ==
           first_colony_after_turn.get_income() +
           GameFormulas::calculate_money_interest(-dip_amount));
    assert(first_player_after_turn.get_effective_income() ==
           first_player_after_turn.get_money_income() + persistent_dip_amount);
    assert(first_player_after_turn.get_money() ==
           first_player_after_turn.get_money_income() - persistent_dip_amount);
    assert(game->get_dip_amount(1) == persistent_dip_amount);
    assert(game->is_dip_persistent(1));

    const Player& second_player_after_turn = game->get_players()[1];
    assert(second_player_after_turn.get_effective_income() ==
           second_player_after_turn.get_money_income() + actual_one_time_dip);
    assert(game->get_dip_amount(2) == 0);
    assert(!game->is_dip_persistent(2));

    game->process_turn();
    assert(game->get_players().front().get_effective_income() ==
           game->get_players().front().get_money_income() + persistent_dip_amount);
    assert(game->get_players()[1].get_effective_income() ==
           game->get_players()[1].get_money_income());

    const int64_t savings_before_clear = game->get_players().front().get_money();
    game->clear_dip_amount(1);
    assert(game->get_dip_amount(1) == 0);
    assert(!game->is_dip_persistent(1));
    assert(game->get_players().front().get_money() ==
           savings_before_clear + persistent_dip_amount);

    game->process_turn();
    assert(game->get_players().front().get_effective_income() ==
           game->get_players().front().get_money_income());
    assert(game->get_current_turn() == 3);
    assert(game->get_current_year() == 2030);
}

void test_ship_construction()
{
    auto game = create_ship_game();
    game->process_turn();

    const uint32_t ship_planet_id =
        game->get_players().front().get_colonized_planets().front().get_id();
    const uint32_t design_id = game->get_player(1)->create_ship_design(
        "Placeholder Scout", SHIP_SCOUT, 1, 2, 1, 1, 0);
    assert(design_id != 0);

    const ShipDesign* design = game->get_player(1)->get_ship_design(design_id);
    assert(design != nullptr);
    assert(design->get_build_cost() == 4425);
    assert(design->get_prototype_cost() == 44250);
    assert(design->get_metal_cost() == 443);
    assert(!design->get_has_been_prototyped());

    const int64_t money_before_first_build = game->get_player(1)->get_money();
    const int64_t metal_before_first_build = game->get_player(1)->get_metal();
    assert(game->build_ships_from_design(1, design_id, 3, ship_planet_id) ==
           ErrorCode::SUCCESS);
    assert(game->get_player(1)->get_money() == money_before_first_build - 57525);
    assert(game->get_player(1)->get_metal() == metal_before_first_build - 1329);
    assert(game->get_player(1)->get_fleets().size() == 1);
    assert(game->get_player(1)->get_fleets().front().ship_count == 3);
    assert(design->get_has_been_prototyped());

    const int64_t money_before_second_build = game->get_player(1)->get_money();
    const int64_t metal_before_second_build = game->get_player(1)->get_metal();
    assert(game->build_ships_from_design(1, design_id, 2, ship_planet_id) ==
           ErrorCode::SUCCESS);
    assert(game->get_player(1)->get_money() == money_before_second_build - 8850);
    assert(game->get_player(1)->get_metal() == metal_before_second_build - 886);
    assert(game->get_player(1)->get_fleets().size() == 2);
    assert(game->get_player(1)->get_fleets()[1].ship_count == 2);

    const int64_t money_before_failed_build = game->get_player(1)->get_money();
    const int64_t metal_before_failed_build = game->get_player(1)->get_metal();
    assert(game->build_ships_from_design(1, design_id, 1001, ship_planet_id) ==
           ErrorCode::INVALID_FLEET_SIZE);
    assert(game->get_player(1)->get_money() == money_before_failed_build);
    assert(game->get_player(1)->get_metal() == metal_before_failed_build);
    assert(game->get_player(1)->get_fleets().size() == 2);

    const int64_t requested_excessive_dip = 100000000;
    const int64_t ship_planetary_income =
        game->get_player(1)->get_colonized_planets().front().get_income();
    const int64_t maximum_ship_dip = GameFormulas::calculate_available_build_money(
        money_before_failed_build, ship_planetary_income, 1);
    game->set_dip_amount(1, requested_excessive_dip);
    assert(game->build_ships_from_design(1, design_id, 1, ship_planet_id) ==
           ErrorCode::INSUFFICIENT_MONEY);
    assert(game->get_player(1)->get_money() ==
           money_before_failed_build - std::min(requested_excessive_dip, maximum_ship_dip));
    assert(game->get_player(1)->get_metal() == metal_before_failed_build);
    assert(game->get_player(1)->get_fleets().size() == 2);
    game->clear_dip_amount(1);
}

void test_savings_rate_modifier()
{
    auto game = create_two_player_game();
    Player& player = game->get_players().front();

    assert(player.get_savings_rate_modifier() == 1.0);
    player.set_savings_rate_modifier(2.0);
    assert(player.get_savings_rate_modifier() == 2.0);
    assert(GameFormulas::calculate_money_interest(100, player.get_savings_rate_modifier()) == 200);
    assert(GameFormulas::calculate_money_interest(-100, player.get_savings_rate_modifier()) == -15);
}

void test_planet_view_observation_precedence()
{
    auto game = create_two_player_game();
    const uint32_t observer_id = 1;
    const uint32_t owned_planet_id = game->get_player(observer_id)->get_colonized_planets().front().get_id();
    Planet* owned_planet = game->get_planet(owned_planet_id);
    assert(owned_planet != nullptr);

    uint32_t unowned_planet_id = 0;
    for (const Planet& planet : game->get_galaxy().planets)
    {
        if (planet.owner == 0)
        {
            unowned_planet_id = planet.id;
            break;
        }
    }
    assert(unowned_planet_id != 0);

    // A quickly lost battle can report an explicitly supplied, incorrect owner.
    assert(game->set_obs_lost_battle_quickly(observer_id, unowned_planet_id, 2));
    Planet* unowned_planet = game->get_planet(unowned_planet_id);
    assert(unowned_planet != nullptr);
    PlanetView* view = unowned_planet->get_view(observer_id);
    assert(view != nullptr);
    assert(view->apparent_owner == 2);
    assert(view->observation_year == game->get_current_year());

    // Later event types may only report authoritative planet data.
    assert(game->set_obs_lost_battle_eventually(observer_id, unowned_planet_id));
    assert(view->apparent_owner == 0);
    assert(view->apparent_population == 0);
    assert(game->set_obs_friendly_ship_visit(observer_id, unowned_planet_id));
    assert(view->metal == unowned_planet->metal);

    // Owned-colony knowledge has precedence over any weaker observation.
    assert(game->set_obs_fullcolony(observer_id, owned_planet_id));
    view = owned_planet->get_view(observer_id);
    assert(view != nullptr);
    assert(view->apparent_owner == static_cast<PlayerID>(observer_id));
    assert(view->income == game->get_player(observer_id)->get_colonized_planets().front().get_income());

    // A planet with no current observation source retains its last observation.
    const int32_t last_observation_year = unowned_planet->get_view(observer_id)->observation_year;
    assert(game->mark_player_turn_ready(1) == ErrorCode::SUCCESS);
    assert(game->mark_player_turn_ready(2) == ErrorCode::SUCCESS);
    game->process_turn();
    assert(unowned_planet->get_view(observer_id)->observation_year == last_observation_year);
}

void test_append_only_save_file()
{
    auto game = create_two_player_game();
    const std::vector<uint8_t> definition_payload =
        game->serialize_immutable_definition();
    assert(!definition_payload.empty());

    GameDefinition decoded_definition;
    assert(decode_game_definition(definition_payload, decoded_definition));
    assert(decoded_definition.schema_version == GameDefinition::CURRENT_SCHEMA_VERSION);
    assert(decoded_definition.planets.size() == game->get_galaxy().planets.size());
    assert(decoded_definition.home_planet_ids.size() == 2);
    assert(decoded_definition.player_setups.size() == 2);
    assert(decoded_definition.player_ideal_temperatures.size() == 2);
    assert(decoded_definition.player_ideal_gravities.size() == 2);
    assert(decoded_definition.player_ideal_temperatures.front() ==
           game->get_players().front().get_ideal_temperature());
    assert(decoded_definition.player_ideal_gravities.front() ==
           game->get_players().front().get_ideal_gravity());
    assert(decoded_definition.galaxy_params.shape == GALAXY_RANDOM);
    assert(decoded_definition.galaxy_params.seed == 42);
    assert(decoded_definition.planets.front().id == game->get_galaxy().planets.front().id);
    assert(decoded_definition.planets.front().name == game->get_galaxy().planets.front().name);
    assert(decoded_definition.planets.front().true_gravity ==
           game->get_galaxy().planets.front().true_gravity);

    game->get_players().front().set_savings_rate_modifier(2.0);
    game->get_players().back().set_eliminated(true);
    game->get_galaxy().planets.front().true_temperature += 12.5;
    game->get_galaxy().planets.front().metal -= 3;
    game->get_galaxy().planets.front().nova_state = PLANET_WARNING;
    uint32_t observed_planet_id = 0;
    for (const Planet& planet : game->get_galaxy().planets)
    {
        if (planet.owner == 0)
        {
            observed_planet_id = planet.id;
            break;
        }
    }
    assert(observed_planet_id != 0);
    assert(game->set_obs_lost_battle_quickly(1, observed_planet_id, 2));
    Planet* observed_planet = game->get_planet(observed_planet_id);
    assert(observed_planet != nullptr);
    observed_planet->get_or_create_view(1).income = 12345;
    observed_planet->get_or_create_view(1).has_observed_gravity = true;
    const std::vector<uint8_t> runtime_payload = game->serialize_state();
    assert(!runtime_payload.empty());
    RuntimeCheckpointPayload decoded_runtime;
    assert(decode_runtime_checkpoint(runtime_payload, decoded_runtime));
    assert(decoded_runtime.deterministic_seed == game->get_det_seed());
    assert(decoded_runtime.ai_seed == game->get_ai_seed());
    assert(!decoded_runtime.deterministic_rng_state.empty());
    assert(!decoded_runtime.ai_rng_state.empty());

    auto restored_game = create_two_player_game();
    restored_game->get_players().front().set_savings_rate_modifier(1.0);
    restored_game->get_galaxy().planets.front().true_temperature = 0.0;
    restored_game->get_galaxy().planets.front().metal = 0;
    assert(restored_game->deserialize_state(runtime_payload));
    assert(restored_game->get_current_turn() == game->get_current_turn());
    assert(restored_game->get_current_year() == game->get_current_year());
    assert(restored_game->get_players().front().get_savings_rate_modifier() == 2.0);
    assert(!restored_game->get_players().front().is_eliminated());
    assert(restored_game->get_players().back().is_eliminated());
    assert(restored_game->get_galaxy().planets.front().true_temperature ==
           game->get_galaxy().planets.front().true_temperature);
    assert(restored_game->get_galaxy().planets.front().metal ==
           game->get_galaxy().planets.front().metal);
    assert(restored_game->get_galaxy().planets.front().nova_state == PLANET_WARNING);
    const Planet* restored_observed_planet = restored_game->get_planet(observed_planet_id);
    assert(restored_observed_planet != nullptr);
    const PlanetView* restored_view = restored_observed_planet->get_view(1);
    assert(restored_view != nullptr);
    assert(restored_view->apparent_owner == 2);
    assert(restored_view->observation_year == game->get_current_year());
    assert(restored_view->income == 12345);
    assert(restored_view->has_observed_gravity);

    const std::string path = "/tmp/openho_save_file_test.save";
    const std::vector<uint8_t> definition = definition_payload;
    const std::vector<uint8_t> first_payload = {0x10, 0x20};
    const std::vector<uint8_t> second_payload = {0x30, 0x40, 0x50};

    std::remove(path.c_str());
    assert(SaveFile::create(path, definition));
    assert(SaveFile::append_checkpoint(
        path, 1, SaveRecordPhase::TURN_CHECKPOINT, first_payload));
    assert(SaveFile::append_checkpoint(
        path, 2, SaveRecordPhase::MANUAL_CHECKPOINT, second_payload));

    std::vector<uint8_t> loaded_definition;
    assert(SaveFile::read_immutable_definition(path, loaded_definition));
    assert(loaded_definition == definition);

    SaveCheckpoint latest;
    assert(SaveFile::read_latest_checkpoint(path, latest));
    assert(latest.turn == 2);
    assert(latest.phase == SaveRecordPhase::MANUAL_CHECKPOINT);
    assert(latest.payload == second_payload);

    // Simulate a process interruption that leaves an incomplete trailing record.
    {
        std::ofstream damaged(path, std::ios::binary | std::ios::app);
        const char incomplete[] = {'R', 'C', 'D'};
        damaged.write(incomplete, sizeof(incomplete));
    }
    SaveCheckpoint after_interruption;
    assert(SaveFile::read_latest_checkpoint(path, after_interruption));
    assert(after_interruption.turn == 2);
    assert(after_interruption.payload == second_payload);

    const std::vector<uint8_t> repaired_payload = {0x60, 0x70};
    assert(SaveFile::append_checkpoint(
        path, 3, SaveRecordPhase::TURN_CHECKPOINT, repaired_payload));
    SaveCheckpoint after_repair;
    assert(SaveFile::read_latest_checkpoint(path, after_repair));
    assert(after_repair.turn == 3);
    assert(after_repair.payload == repaired_payload);

    assert(SaveFile::compact(path));
    SaveCheckpoint after_compaction;
    assert(SaveFile::read_latest_checkpoint(path, after_compaction));
    assert(after_compaction.turn == 3);
    assert(after_compaction.payload == repaired_payload);

    std::remove(path.c_str());
}

void test_rng_state_continuation()
{
    DeterministicRNG original(1234, 5678);
    for (int i = 0; i < 11; ++i)
    {
        (void)original.nextUInt64();
        (void)original.nextAIUInt64();
    }
    const std::vector<uint8_t> deterministic_state = original.serialize_deterministic_rng_state();
    const std::vector<uint8_t> ai_state = original.serialize_ai_rng_state();

    DeterministicRNG restored(1234, 5678);
    restored.deserialize_deterministic_rng_state(deterministic_state);
    restored.deserialize_ai_rng_state(ai_state);
    for (int i = 0; i < 8; ++i)
    {
        assert(original.nextUInt64() == restored.nextUInt64());
        assert(original.nextAIUInt64() == restored.nextAIUInt64());
    }
}
}

int main()
{
    test_game_initialization_and_readiness();
	 test_dip_and_turn_processing();
	 test_ship_construction();
	 test_savings_rate_modifier();
	 test_planet_view_observation_precedence();
	 test_append_only_save_file();
	 test_rng_state_continuation();
    std::cout << "Gameplay regression scenarios passed.\n";
    return 0;
}
