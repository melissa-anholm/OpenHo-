#ifndef OPENHO_GAME_SETUP_H
#define OPENHO_GAME_SETUP_H

#include "openho_core.h"
#include <vector>
#include <memory>

// Forward declaration
class GameState;

// ============================================================================
// Player Setup Information
// ============================================================================

/// Configuration for a single player
struct PlayerSetup
{
	std::string name;
	Gender player_gender;  // GENDER_F or GENDER_M
	PlayerType type;  // PLAYER_HUMAN or PLAYER_COMPUTER
	int32_t ai_iq;    // AI difficulty level (0-100), only used if type == PLAYER_COMPUTER
	StartingColonyQuality starting_colony_quality;  // Quality of the player's starting colony
};

// ============================================================================
// Game Setup Manager
// ============================================================================

/// Manages the game setup flow: collecting user input for new games
/// This class handles the orchestration of gathering all necessary parameters
/// before creating a GameState instance.
class GameSetup
{
public:
	GameSetup();
	~GameSetup();
	
	/// Start a new game setup flow
	/// Queries user for galaxy parameters and player configuration
	/// Returns a fully initialized GameState if successful, nullptr otherwise
	[[nodiscard]] GameState* create_new_game();
	
	/// Get the galaxy parameters collected during setup
	const GalaxyGenerationParams& get_galaxy_params() const { return galaxy_params; }
	
	/// Get the player configurations collected during setup
	const std::vector<PlayerSetup>& get_player_setups() const { return player_setups; }
	
private:
	// Collected setup data
	GalaxyGenerationParams galaxy_params;
	std::vector<PlayerSetup> player_setups;
	
	/// Query user for galaxy generation parameters
	[[nodiscard]] GalaxyGenerationParams query_galaxy_parameters();
	
	/// Query user for player configuration
	[[nodiscard]] std::vector<PlayerSetup> query_player_configuration();
	
	/// Query user for a single player's configuration
	[[nodiscard]] PlayerSetup query_single_player(int player_number);
};

#endif // OPENHO_GAME_SETUP_H
