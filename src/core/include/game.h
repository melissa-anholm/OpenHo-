#ifndef OPENHO_GAME_H
#define OPENHO_GAME_H

#include "openho_core.h"
#include "rng.h"
#include "game_constants.h"
#include "game_formulas.h"
#include "game_setup.h"
#include <memory>
#include <unordered_map>

// ============================================================================
// GameState Class
// ============================================================================

/**
 * Main game state manager.
 * 
 * Manages the Galaxy, RNG, and game logic.
 * This is the internal C++ representation that is wrapped by the C API.
 */
class GameState
{
public:
	// Constructor - initializes the entire game state including galaxy and first turn
	// Takes a GameSetup reference and copies the necessary data
	GameState(const class GameSetup& setup);
	
	// Destructor
	~GameState();
	
	// Game state access
	Galaxy& get_galaxy();
	const Galaxy& get_galaxy() const;
	
	// RNG access
	DeterministicRNG& get_rng();
	const DeterministicRNG& get_rng() const;
	
	// Player access
	[[nodiscard]] Player* get_player(uint32_t player_id);
	[[nodiscard]] const Player* get_player(uint32_t player_id) const;
	[[nodiscard]] Player* get_player_by_name(const std::string& name);
	[[nodiscard]] const Player* get_player_by_name(const std::string& name) const;
	
	// Planet access
	[[nodiscard]] Planet* get_planet(uint32_t planet_id);
	[[nodiscard]] const Planet* get_planet(uint32_t planet_id) const;
	[[nodiscard]] Planet* get_planet(const std::string& planet_name);
	[[nodiscard]] const Planet* get_planet(const std::string& planet_name) const;
	
	// Get all planets owned by a player (O(1) lookup)
	const std::vector<size_t>& get_player_planets(uint32_t player_id) const;
	
	// Get number of players
	uint32_t get_num_players() const;
	
	// Get current turn
	uint32_t get_current_turn() const;
	
	// Get current year
	uint32_t get_current_year() const;
	
	// Allocate a globally unique fleet ID
	[[nodiscard]] uint32_t allocate_fleet_id();
	
	// Get all players (for iteration)
	const std::vector<Player>& get_players() const;
	std::vector<Player>& get_players();
	
	// Initialize the first turn (called during game initialization)
	void start_first_turn();
	
	// Increment turn and year (called at end of process_turn)
	void increment_turn();
	void increment_year();
	

	
	// Player property accessors (for C API and internal use)
	int64_t get_player_money_income(uint32_t player_id) const;
	int64_t get_player_metal_income(uint32_t player_id) const;
	int64_t get_player_money(uint32_t player_id) const;
	int64_t get_player_metal_reserve(uint32_t player_id) const;
	double get_player_ideal_temperature(uint32_t player_id) const;
	double get_player_ideal_gravity(uint32_t player_id) const;
	
	// Player public information management
	/// Capture current player info and distribute to all players
	void capture_and_distribute_player_public_info();
	
	/// Get full player info history (for resync after disconnection)
	[[nodiscard]] const std::vector<PlayerPublicInfo>& get_full_player_info_history(uint32_t player_id) const;
	
	// Ship design management (delegates to Player)
	[[nodiscard]] uint32_t create_ship_design(uint32_t player_id, const std::string& name, ShipType type, int32_t tech_range, int32_t tech_speed, int32_t tech_weapons, int32_t tech_shields, int32_t tech_miniaturization);
	[[nodiscard]] const ShipDesign* get_ship_design(uint32_t player_id, uint32_t design_id) const;
	[[nodiscard]] const std::vector<ShipDesign>& get_player_ship_designs(uint32_t player_id) const;
	[[nodiscard]] bool delete_ship_design(uint32_t player_id, uint32_t design_id);
	void build_ship_from_design(uint32_t player_id, uint32_t design_id);
	
	// Fleet management (delegates to Player)
	// Note: design_id is validated and looked up internally before passing to Player::create_fleet
	[[nodiscard]] uint32_t create_fleet(uint32_t player_id, uint32_t design_id, uint32_t ship_count, uint32_t planet_id);
	[[nodiscard]] Fleet* get_fleet(uint32_t player_id, uint32_t fleet_id);
	[[nodiscard]] const Fleet* get_fleet(uint32_t player_id, uint32_t fleet_id) const;
	[[nodiscard]] const std::vector<Fleet>& get_player_fleets(uint32_t player_id) const;
	[[nodiscard]] bool delete_fleet(uint32_t player_id, uint32_t fleet_id);
	void move_fleet(uint32_t player_id, uint32_t fleet_id, uint32_t destination_planet_id);
	void refuel_fleet(uint32_t player_id, uint32_t fleet_id);
	
	// Turn processing
	void process_turn();
	
	// Money allocation
	void set_money_allocation(uint32_t player_id, const Player::MoneyAllocation& alloc);
	const Player::MoneyAllocation& get_money_allocation(uint32_t player_id) const;
	
	// AI RNG seed management
	uint64_t get_ai_rng_seed() const;
	void set_ai_rng_seed(uint64_t seed);
	
	// Serialization
	[[nodiscard]] std::vector<uint8_t> serialize_state() const;
	[[nodiscard]] bool deserialize_state(const std::vector<uint8_t>& data);
	
private:
	// Game state
	std::unique_ptr<Galaxy> galaxy;  // Initialized in constructor with params
	std::vector<Player> players;  // All players in the game
	std::unique_ptr<DeterministicRNG> rng;
	
	// Setup configuration (stored for reference and serialization)
	GalaxyGenerationParams galaxy_params;  // Galaxy generation parameters used to create this game
	std::vector<PlayerSetup> player_setups;  // Player configurations used to create this game
	
	// ========== IMMUTABLE MAPPINGS (built once, never change) ==========
	// Entity ID to index mappings - fixed after initialization
	std::unordered_map<uint32_t, size_t> planet_id_to_index;      // planet ID -> index in galaxy.planets
	std::unordered_map<std::string, size_t> planet_name_to_index;  // planet name -> index in galaxy.planets
	std::unordered_map<uint32_t, size_t> player_id_to_index;       // player ID -> index in players
	std::unordered_map<std::string, size_t> player_name_to_index;  // player name -> index in players
	
	// ========== MUTABLE MAPPINGS (updated frequently) ==========
	// Fleet ownership mapping: (player_id, fleet_id) -> fleet index (O(1) lookup)
	std::unordered_map<uint64_t, size_t> fleet_id_to_index;
	
	// Per-player fleet mapping: player_id -> vector of fleet indices (updated when fleets change)
	std::unordered_map<uint32_t, std::vector<size_t>> player_fleets;
	
	// Player planet ownership mapping: player_id -> vector of planet indices (updated when planets colonized/lost)
	std::unordered_map<uint32_t, std::vector<size_t>> player_planets;
	
	// Player public information history: player_id -> vector of PlayerPublicInfo (one per turn)
	std::unordered_map<uint32_t, std::vector<PlayerPublicInfo>> player_info_history;
	
	// Current AI RNG seed (tracked separately since Boost doesn't expose it)
	uint64_t current_ai_rng_seed;
	
	// Current game turn
	uint32_t current_turn = 0;
	
	// Current game year (starts at 2000, increments by 10 per turn)
	uint32_t current_year = 2000;
	
	// Fleet ID allocation (globally unique across all players)
	uint32_t next_fleet_id = 1;
	
	// Private helper methods
	std::vector<Player> initialize_players(const std::vector<PlayerSetup>& player_setups);
	void initialize_galaxy(const GalaxyGenerationParams& params);
	void build_entity_maps();
	void process_population_growth();
	void check_population_decreasing_events(uint32_t planet_id);
	void calculate_player_incomes();
	void process_player_incomes();
	void update_planet_incomes();
	void process_money_allocation();
	void process_research();
	void process_research_stream(Player& player, TechStream stream, int64_t research_budget);
	void process_planets();
	void process_ships();
	void process_novae();

	

};

#endif // OPENHO_GAME_H
