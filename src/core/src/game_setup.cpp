#include "game_setup.h"
#include "game.h"
#include <iostream>
#include <limits>

// ============================================================================
// GameSetup Constructor and Destructor
// ============================================================================

GameSetup::GameSetup()
	: galaxy_params(100, 1, 0.5, GALAXY_RANDOM, 0)
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
	
	// Update galaxy params with actual number of players
	galaxy_params.n_players = player_setups.size();
	
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
	
	// Note: n_players will be set after player configuration is queried
	// For now, use a placeholder value
	uint32_t n_players = 1;
	
	double density;
	std::cout << "Planet distribution density (0.25-4.0): ";
	std::cin >> density;
	// Check that density is within range!!
	if(density>GameConstants::galaxy_max_density)
	{
		density=GameConstants::galaxy_max_density;
	}
	if(density<GameConstants::galaxy_min_density)
	{
		density=GameConstants::galaxy_min_density;
	}
	
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
	
	return GalaxyGenerationParams(n_planets, n_players, density, shape, seed);
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
	
	// Query player type first (Human or Computer)
	std::cout << "Is this player human or computer? (0 = Human, 1 = Computer): ";
	int type_input;
	std::cin >> type_input;
	player.type = static_cast<PlayerType>(type_input);
	
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	
	if (player.type == PLAYER_HUMAN)
	{
		// Human player: get name and gender, set IQ to 100
		std::cout << "Player name: ";
		std::getline(std::cin, player.name);
		
		std::cout << "Player gender (1 = Female, 2 = Male): ";
		int gender_input;
		std::cin >> gender_input;
		player.player_gender = static_cast<Gender>(gender_input);
		
		player.ai_iq = 100;  // Human players always get IQ 100 (unused)
	}
	else
	{
		// Computer player: get IQ, set placeholder name, random gender
		std::cout << "AI IQ (50-200): ";
		int32_t iq_input;
		std::cin >> iq_input;
		
		// Clamp IQ to valid range [50, 200]
		if (iq_input < 50)
		{
			std::cout << "IQ " << iq_input << " is below minimum (50). Adjusting to 50." << std::endl;
			player.ai_iq = 50;
		}
		else if (iq_input > 200)
		{
			std::cout << "IQ " << iq_input << " is above maximum (200). Adjusting to 200." << std::endl;
			player.ai_iq = 200;
		}
		else
		{
			player.ai_iq = iq_input;
		}
		
		// Set placeholder name and gender (will be assigned in GameState::initialize_players)
		player.name = "";
		player.player_gender = GENDER_OTHER;  // Placeholder, will be randomly selected in initialize_players()
	}
	
	// For now, hardcode starting colony quality to Normal
	player.starting_colony_quality = START_NORMAL;
	
	return player;
}
