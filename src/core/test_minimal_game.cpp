#include "game.h"
#include "game_setup.h"
#include "game_formulas.h"
#include "save_file.h"
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

void test_append_only_save_file()
{
    const std::string path = "/tmp/openho_save_file_test.save";
    const std::vector<uint8_t> definition = {0x01, 0x02, 0x03, 0x04};
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
}

int main()
{
    test_game_initialization_and_readiness();
    test_dip_and_turn_processing();
    test_ship_construction();
    test_append_only_save_file();
    std::cout << "Gameplay regression scenarios passed.\n";
    return 0;
}
