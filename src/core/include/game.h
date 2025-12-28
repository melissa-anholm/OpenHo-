#ifndef OPENHO_GAME_H
#define OPENHO_GAME_H

#include "openho_core.h"
#include "rng.h"
#include "game_constants.h"
#include "game_formulas.h"
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
	// Constructor
	GameState(const GalaxyGenerationParams& params);
	
	// Destructor
	~GameState();
	
	// Game state access
	Galaxy& get_galaxy();
	const Galaxy& get_galaxy() const;
	
	// RNG access
	DeterministicRNG& get_rng();
	const DeterministicRNG& get_rng() const;
	
	// Player access
	Player* get_player(uint32_t player_id);
	const Player* get_player(uint32_t player_id) const;
	
	// Planet access
	Planet* get_planet(uint32_t planet_id);
	const Planet* get_planet(uint32_t planet_id) const;
	
	// Get all planets owned by a player (O(1) lookup)
	const std::vector<size_t>& get_player_planets(uint32_t player_id) const;
	
	// Ship access
	Ship* get_ship(uint32_t ship_id);
	const Ship* get_ship(uint32_t ship_id) const;
	
	// Get all ships owned by a player (O(1) lookup)
	const std::vector<size_t>& get_player_ships(uint32_t player_id) const;
	
	// Player public information queries
	PlayerPublicInfo get_player_public_info(uint32_t player_id, uint32_t turn) const;
	PlayerPublicInfo get_player_public_info_current(uint32_t player_id) const;
	uint32_t get_player_info_history_size(uint32_t player_id) const;
	
	// Ship design management
	uint32_t create_ship_design(uint32_t player_id, const std::string& name, ShipType type, int32_t tech_range, int32_t tech_speed, int32_t tech_weapons, int32_t tech_shields, int32_t tech_miniaturization);
	const ShipDesign* get_ship_design(uint32_t player_id, uint32_t design_id) const;
	const std::vector<ShipDesign>& get_player_ship_designs(uint32_t player_id) const;
	bool delete_ship_design(uint32_t player_id, uint32_t design_id);
	void build_ship_from_design(uint32_t player_id, uint32_t design_id);
	
	// Turn processing
	void process_turn();
	
	// Money allocation
	void set_money_allocation(uint32_t player_id, const MoneyAllocation& alloc);
	const MoneyAllocation& get_money_allocation(uint32_t player_id) const;
	
	// AI RNG seed management
	uint64_t get_ai_rng_seed() const;
	void set_ai_rng_seed(uint64_t seed);
	
	// Serialization
	std::vector<uint8_t> serialize_state() const;
	bool deserialize_state(const std::vector<uint8_t>& data);
	
private:
	// Game state
	Galaxy galaxy;
	std::unique_ptr<DeterministicRNG> rng;
	
	// Entity ID mappings for quick lookup
	std::unordered_map<uint32_t, size_t> planet_id_to_index;
	std::unordered_map<uint32_t, size_t> ship_id_to_index;
	std::unordered_map<uint32_t, size_t> player_id_to_index;
	
	// Player ship ownership mapping: player_id -> vector of ship indices (O(1) lookup)
	std::unordered_map<uint32_t, std::vector<size_t>> player_ships;
	
	// Player planet ownership mapping: player_id -> vector of planet indices (O(1) lookup)
	std::unordered_map<uint32_t, std::vector<size_t>> player_planets;
	
	// Player public information history: player_id -> vector of PlayerPublicInfo (one per turn)
	std::unordered_map<uint32_t, std::vector<PlayerPublicInfo>> player_info_history;
	
	// Current AI RNG seed (tracked separately since Boost doesn't expose it)
	uint64_t current_ai_rng_seed;
	
	// Private helper methods
	void initialize_planets();
	void initialize_players();
	void build_entity_maps();
	void calculate_player_incomes();
	void update_planet_incomes();
	void process_money_allocation();
	void apply_money_interest();
	void process_research();
	void process_research_stream(Player& player, TechStream stream, int64_t research_budget);
	void process_terraforming();
	void process_mining();
	void process_ships();
	void process_novae();
	void capture_player_public_info();
	

};

#endif // OPENHO_GAME_H
