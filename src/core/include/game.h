#ifndef OPENHO_GAME_H
#define OPENHO_GAME_H

#include "openho_core.h"
#include "rng.h"
#include "game_constants.h"
#include "game_formulas.h"
#include "game_setup.h"
#include "error_codes.h"
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
	Galaxy& get_galaxy()
		{ return *galaxy; }
	const Galaxy& get_galaxy() const
		{ return *galaxy; }

	// RNG access
	DeterministicRNG& get_rng()
		{ return *rng; }
	const DeterministicRNG& get_rng() const
		{ return *rng; }
	
	//
	uint32_t get_num_players() const
		{ return static_cast<uint32_t>(players.size()); }
	uint32_t get_current_turn() const
		{ return current_turn; }
	uint32_t get_current_year() const
		{ return current_year; }
	
	// Get all players (for iteration)
	const std::vector<Player>& get_players() const
		{ return players; }
	std::vector<Player>& get_players()
		{ return players; }

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
		
	// Allocate a globally unique fleet ID
	[[nodiscard]] uint32_t allocate_fleet_id();
		{ return next_fleet_id++; }
	
	
	// Initialize the first turn (called during game initialization)
	void start_first_turn();
	
	// Increment turn and year (called at end of process_turn)
	void increment_turn()
		{ current_turn++; }
	void increment_year()
		{ current_year += 10; }
	
	
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
	
	// ========================================================================
	// Validation Methods (check_* pattern)
	// ========================================================================
	
	/// Check if a player can build a fleet with given parameters
	[[nodiscard]] ErrorCode check_player_build_fleet(uint32_t player_id, uint32_t design_id, uint32_t ship_count, uint32_t planet_id) const;
	
	/// Check if a player can design a ship with given parameters
	[[nodiscard]] ErrorCode check_player_design_ship(uint32_t player_id, const std::string& name, ShipType type, int32_t tech_range, int32_t tech_speed, int32_t tech_weapons, int32_t tech_shields, int32_t tech_mini) const;
	
	/// Check if a player can set spending allocation
	[[nodiscard]] ErrorCode check_player_set_spending_allocation(uint32_t player_id, double savings_frac, double research_frac, double planets_frac) const;
	
	/// Check if a player can move a fleet
	[[nodiscard]] ErrorCode check_player_move_fleet(uint32_t player_id, uint32_t fleet_id, uint32_t destination_planet_id) const;
	
	/// Check if a player can set planet allocation
	[[nodiscard]] ErrorCode check_player_set_planet_allocation(uint32_t player_id, uint32_t planet_id, double mining_frac, double terraforming_frac) const;
	
	// RNG seed accessors (delegates to RNG)
	uint64_t get_deterministic_seed() const
		{ return rng->getDeterministicSeed(); }
	void set_deterministic_seed(uint64_t seed)
		{ rng->setDeterministicSeed(seed); }
	uint64_t get_ai_rng_seed() const
		{ return rng->getAISeed(); }
	void set_ai_rng_seed(uint64_t seed)
		{ rng->setAISeed(seed); }
	
	
	// Serialization
	[[nodiscard]] std::vector<uint8_t> serialize_state() const;
	[[nodiscard]] bool deserialize_state(const std::vector<uint8_t>& data);
	
private:
	// Current game turn
	uint32_t current_turn = 0;
	// Current game year (starts at 2000, increments by 10 per turn)
	uint32_t current_year = 2000;
	// Fleet ID allocation (globally unique across all players)
	uint32_t next_fleet_id = 1;
	
	
	// Game state
	std::unique_ptr<Galaxy> galaxy;  // Initialized in constructor with params
	std::vector<Player> players;  // All players in the game
	std::unique_ptr<DeterministicRNG> rng;
	std::unique_ptr<class TextAssets> text_assets;  // Text assets (names, descriptions, etc.)
	
	// Setup configuration (stored for reference and serialization)
	GalaxyGenerationParams galaxy_params;  // Galaxy generation parameters used to create this game
	std::vector<PlayerSetup> player_setups;  // Player configurations used to create this game
	
	// Friend class for Galaxy to access GameState members
	friend class Galaxy;
	
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
	
	
	// Private helper methods
	std::vector<Player> initialize_players(const std::vector<PlayerSetup>& player_setups);
	std::unique_ptr<Galaxy> initialize_galaxy(const GalaxyGenerationParams& params);
	void build_entity_maps();
	
	// Assign suitable planets to players based on their starting colony quality
	// Takes the suitable planets vector to avoid recalculating it
	void assign_planets_random(const std::vector<Planet*>& suitable_planets);
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
