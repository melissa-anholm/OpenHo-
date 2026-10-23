#include "game.h"
#include "game_setup.h"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

int main()
{
    std::vector<PlayerSetup> player_setups = {
        {"Player One", GENDER_F, PLAYER_HUMAN, 100, START_NORMAL},
        {"Player Two", GENDER_M, PLAYER_HUMAN, 100, START_NORMAL}
    };

    GalaxyGenerationParams galaxy_params(40, 2, 0.5, GALAXY_RANDOM, 42);
    GameSetup setup(galaxy_params, player_setups);
    GameState game(setup);

    assert(game.get_num_players() == 2);
    assert(game.get_galaxy().planets.size() == 40);
    assert(game.get_current_turn() == 0);
    assert(game.get_current_year() == 2000);
    assert(!game.all_players_ready());

    for (const Player& player : game.get_players())
    {
        assert(player.get_colonized_planets().size() == 1);
        assert(player.get_ideal_temperature() > 0.0);
        assert(player.get_ideal_gravity() > 0.0);
        assert(player.get_colonized_planets().front().get_population() > 0);
        assert(player.get_colonized_planets().front().get_income() > 0);
        assert(player.get_colonized_planets().front().get_metal() > 0);
    }

    assert(game.mark_player_turn_ready(1) == ErrorCode::SUCCESS);
    assert(game.get_player_turn_ready_status(1) == 1);
    assert(game.get_player_turn_ready_status(2) == 0);
    assert(!game.all_players_ready());
    assert(game.mark_player_turn_ready(2) == ErrorCode::SUCCESS);
    assert(game.all_players_ready());

    game.process_turn();
    assert(game.get_current_turn() == 1);
    assert(game.get_current_year() == 2010);
    assert(!game.all_players_ready());

    game.process_turn();
    game.process_turn();
    assert(game.get_current_turn() == 3);
    assert(game.get_current_year() == 2030);

    std::cout << "Minimal RANDOM galaxy game advanced through 3 turns successfully.\n";
    return 0;
}
