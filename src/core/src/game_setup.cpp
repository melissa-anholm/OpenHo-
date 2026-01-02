#include "game_setup.h"
#include "game.h"
#include <iostream>
#include <limits>

// ============================================================================
// GameSetup Constructor and Destructor
// ============================================================================

GameSetup::GameSetup()
	: galaxy_params(100, 0.5, GALAXY_RANDOM, 0)
{ }

GameSetup::~GameSetup()
{ }

// ============================================================================
// Game Creation
// ============================================================================
GameState* GameSetup::create_new_game()
{
	// Query user for galaxy parameters
	galaxy_params = query_galaxy_parameters();
	
	// Query user for player configuration
	player_setups = query_player_configuration();
	
	// TODO: Apply player configuration to the game
	// For now, we'll just create the game with default players
	// In the future, this will use the player_setups to configure the players
	
	try
	{
		GameState* game = new GameState(*this);
		return game;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Failed to create game: " << e.what() << std::endl;
		return nullptr;
	}
}

// ============================================================================
// Galaxy Parameter Query
// ============================================================================

GalaxyGenerationParams GameSetup::query_galaxy_parameters()
{
	std::cout << "\n=== Galaxy Configuration ===" << std::endl;
	
	uint32_t n_planets;
	std::cout << "Number of planets (5-500): ";
	std::cin >> n_planets;
	
	double density;
	std::cout << "Planet distribution density (0.0-1.0): ";
	std::cin >> density;
	
	std::cout << "Select galaxy shape:" << std::endl;
	std::cout << "0 = Random, 1 = Spiral, 2 = Circle, 3 = Ring, 4 = Cluster, 5 = Grid" << std::endl;
	std::cout << "Galaxy shape (0-5): ";
	int shape_input;
	std::cin >> shape_input;
	GalaxyShape shape = static_cast<GalaxyShape>(shape_input);
	
	uint64_t seed;
	std::cout << "Random seed (0 for random): ";
	std::cin >> seed;
	
	std::cout << std::endl;
	
	return GalaxyGenerationParams(n_planets, density, shape, seed);
}

// ============================================================================
// Player Configuration Query
// ============================================================================

std::vector<PlayerSetup> GameSetup::query_player_configuration()
{
	std::vector<PlayerSetup> player_setups;
	
	std::cout << "\n=== Player Configuration ===" << std::endl;
	
	// Query number of players
	int num_players;
	std::cout << "Number of players (2-8): ";
	std::cin >> num_players;
	
	// Clamp to valid range
	if (num_players < 2) num_players = 2;
	if (num_players > 8) num_players = 8;
	
	// Query each player's configuration
	for (int i = 0; i < num_players; ++i)
	{
		PlayerSetup player = query_single_player(i + 1);
		player_setups.push_back(player);
	}
	
	std::cout << std::endl;
	
	return player_setups;
}

PlayerSetup GameSetup::query_single_player(int player_number)
{
	PlayerSetup player;
	
	std::cout << "\n--- Player " << player_number << " ---" << std::endl;
	
	// Query player name
	std::cout << "Player name: ";
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	std::getline(std::cin, player.name);
	
	// Query player gender
	std::cout << "Player gender (1 = Female, 2 = Male): ";
	int gender_input;
	std::cin >> gender_input;
	player.player_gender = static_cast<Gender>(gender_input);
	
	// Query player type
	std::cout << "Player type (0 = Human, 1 = Computer): ";
	int type_input;
	std::cin >> type_input;
	player.type = static_cast<PlayerType>(type_input);
	
	// Query AI IQ if computer player
	if (player.type == PLAYER_COMPUTER)
	{
		std::cout << "AI IQ (0-100): ";
		std::cin >> player.ai_iq;
	}
	else
	{
		player.ai_iq = 0;
	}
	
	// For now, hardcode starting colony quality to Normal
	player.starting_colony_quality = COLONY_NORMAL;
	
	return player;
}
