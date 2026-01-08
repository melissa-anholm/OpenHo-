#ifndef OPENHO_PLAYER_H
#define OPENHO_PLAYER_H

#include "planet.h"
#include "colonized_planet.h"
#include "knowledge_planet.h"
#include "knowledge_galaxy.h"
#include "ship_design.h"
#include "fleet.h"
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <string>

// ============================================================================
// Forward Declarations
// ============================================================================

typedef int32_t PlayerID;

// ============================================================================
// Data Structures
// ============================================================================

// Public information about a player (visible to all other players)
struct PlayerPublicInfo
{
	uint32_t player_id;
	uint32_t year;      // Game year (same as turn number)
	uint32_t turn;      // Turn number (same as year, for convenience)
	
	// Technology levels (subset - no Radical)
	int32_t tech_range;
	int32_t tech_speed;
	int32_t tech_weapons;
	int32_t tech_shields;
	int32_t tech_mini;
	
	// Resources
	int64_t money_income;
	int64_t money_savings;
	int64_t metal_savings;
		
	// Territory
	uint32_t planets_owned;
	
	// Calculated metrics
	int64_t ship_power;
	int32_t victory_points;
};

// Player structure
// IMPORTANT: Player IDs must never be 0!
// NOT_OWNED (0) is reserved to mean unowned for planets.
// Player IDs must start from 1 and increment from there.
class Player
{
friend class GameState;  // GameState has access to all private members
// Note: C API functions in c_api.cpp access Player through GameState, which is a friend

public:
	uint32_t id;
	std::string name;
	Gender gender;  // Player's gender (for icons/graphics and pronouns in notifications)
	PlayerType type;
	int32_t iq;  // For computer players
	
	// Constructor
	Player(class GameState* game_state_ref) : game_state(game_state_ref) {}
	
	// Move semantics (required by Fleet's unique_ptr)
	Player(Player&&) = default;
	Player& operator=(Player&&) = default;
	
	// Delete copy semantics (Fleet is not copyable)
	Player(const Player&) = delete;
	Player& operator=(const Player&) = delete;
	
	// ========================================================================
	// Public Accessors
	// ========================================================================
	
	/// Get this player's ideal temperature for planets
	double get_ideal_temperature() const { return ideal_temperature; }
	/// Get this player's ideal gravity for planets
	double get_ideal_gravity() const { return ideal_gravity; }
	
	/// Get all fleets owned by this player
	const std::vector<Fleet>& get_fleets() const { return fleets; }
	
	// ========================================================================
	// Type Definitions (used by accessors below)
	// ========================================================================
	
	// Technology levels for a player
	struct TechnologyLevels
	{
		int32_t range;
		int32_t speed;
		int32_t weapons;
		int32_t shields;
		int32_t mini;
		int32_t radical;
	};
	// Research allocation fractions for a player
	// All fractions must sum to 1.0
	struct ResearchAllocation
	{
		double research_range_fraction;
		double research_speed_fraction;
		double research_weapons_fraction;
		double research_shields_fraction;
		double research_mini_fraction;
		double research_radical_fraction;
	};
	// Money allocation for this player (fractions-based)
	struct MoneyAllocation
	{
		double savings_fraction;
		double research_fraction;
		double planets_fraction;  // Fraction of income allocated to planet development
		ResearchAllocation research;
	};
	// Income breakdown for a player (calculated each turn)
	struct IncomeBreakdown
	{
		int64_t planetary_income;      // Income from owned planets
		int64_t interest_income;       // Interest from savings (positive or negative)
		int64_t windfall_income;       // Windfall from rare events
		int64_t total_income;          // Sum of above three
	};
	// Research progress (accumulated points per research stream)
	struct PartialResearchProgress
	{
		int64_t research_points_range;
		int64_t research_points_speed;
		int64_t research_points_weapons;
		int64_t research_points_shields;
		int64_t research_points_mini;
		int64_t research_points_radical;
	};
	
	/// Get all colonized planets owned by this player
	const std::vector<ColonizedPlanet>& get_colonized_planets() const { return colonized_planets; }
	
	/// Get this player's current money savings
	int64_t get_money() const { return money_savings; }
	
	/// Get this player's current metal reserve
	int64_t get_metal() const { return metal_reserve; }
	
	/// Get this player's money income per turn
	int64_t get_money_income() const { return money_income; }
	
	/// Get this player's metal income per turn
	int64_t get_metal_income() const { return metal_income; }
	
	/// Get this player's technology levels
	const TechnologyLevels& get_tech_levels() const { return tech; }
	
	/// Get a specific technology level (0=range, 1=speed, 2=weapons, 3=shields, 4=mini, 5=radical)
	int32_t get_tech_level(uint32_t tech_type) const;
	
	/// Get this player's spending allocation
	const MoneyAllocation& get_spending_allocation() const { return allocation; }
	
	// ========================================================================
	// Fleet Management
	// ========================================================================
	
	/// Create a new fleet for this player
	/// Validates parameters, allocates fleet_id from GameState, and builds the fleet
	[[nodiscard]] uint32_t create_fleet(uint32_t design_id, uint32_t ship_count, uint32_t planet_id);
	
	/// Validate fleet creation parameters
	/// Returns true if all parameters are valid
	[[nodiscard]] bool validate_fleet(uint32_t design_id, uint32_t ship_count, uint32_t planet_id) const;
	
	/// Build and add a fleet after validation
	/// Called internally by create_fleet() after validation and fleet_id allocation
	[[nodiscard]] uint32_t build_fleet(uint32_t fleet_id, uint32_t design_id, uint32_t ship_count, uint32_t planet_id);
	
	/// Get a fleet by ID (mutable)
	[[nodiscard]] Fleet* get_fleet(uint32_t fleet_id);
	
	/// Get a fleet by ID (const)
	[[nodiscard]] const Fleet* get_fleet(uint32_t fleet_id) const;
	
	/// Delete a fleet
	[[nodiscard]] bool delete_fleet(uint32_t fleet_id);
	
	/// Move a fleet to a destination planet
	void move_fleet(uint32_t fleet_id, uint32_t destination_planet_id);
	
	/// Get all fleets currently in transit
	[[nodiscard]] std::vector<Fleet*> get_fleets_in_transit();
	
	/// Get all fleets currently in transit (const version)
	[[nodiscard]] const std::vector<const Fleet*> get_fleets_in_transit() const;
	
	// ========================================================================
	// Ship Design Management
	// ========================================================================
	
	/// Create a new ship design for this player
	[[nodiscard]] uint32_t create_ship_design(const std::string& name, ShipType type, int32_t tech_range, int32_t tech_speed, int32_t tech_weapons, int32_t tech_shields, int32_t tech_mini);
	
	/// Get a ship design by ID
	[[nodiscard]] const ShipDesign* get_ship_design(uint32_t design_id) const;
	
	/// Get all ship designs for this player
	[[nodiscard]] const std::vector<ShipDesign>& get_ship_designs() const { return ship_designs; }
	
	/// Delete a ship design
	[[nodiscard]] bool delete_ship_design(uint32_t design_id);

private:
	// GameState reference for validation and fleet_id allocation
	class GameState* game_state;
	
	// Resources
	int64_t money_savings;  // Savings account
	int64_t metal_reserve;
		
	// Ideal planetary conditions (hidden from player)
	double ideal_temperature;
	double ideal_gravity;
	
	// Calculated properties
	int64_t money_income;  // Per turn
	int64_t metal_income;  // Per turn
	
	// Technology levels
	TechnologyLevels tech;
	// Income breakdown for current turn
	IncomeBreakdown current_turn_income;
	// Current money allocation
	MoneyAllocation allocation;
	// Research progress (accumulated points per research stream)
	PartialResearchProgress partial_research;
	// Colonized planets (owned by this player with allocation information)
	std::vector<ColonizedPlanet> colonized_planets;
	// Player's knowledge of the galaxy
	KnowledgeGalaxy* knowledge_galaxy;  // Owned by Player, initialized during game setup
	
	// Ship designs
	std::vector<ShipDesign> ship_designs;      // All designs, ordered by creation (max 100)
	uint32_t next_ship_design_id;                // Counter for unique design IDs (never resets)
	
	
	// Fleets (groups of identical ships)
	std::vector<Fleet> fleets;                 // All fleets owned by this player
	
	// Player public information history: player_id -> vector of PlayerPublicInfo (one per turn)
	std::unordered_map<uint32_t, std::vector<PlayerPublicInfo>> player_info_history;
	
	// Player info history management
	/// Receive a new player public info snapshot (called by GameState each turn)
	void receive_player_public_info(uint32_t source_player_id, const PlayerPublicInfo& info);
	
	/// Get full player info history for a specific player (for validation/resync)
	[[nodiscard]] const std::vector<PlayerPublicInfo>* get_player_info_history(uint32_t player_id) const;
	
	/// Validate consistency of player info history
	[[nodiscard]] bool validate_player_info_history(uint32_t player_id) const;
	
	/// Request full resync of player info history from GameState
	void request_full_player_info_resync(uint32_t player_id);

	// ========================================================================
	// Money Allocation Calculation Helpers
	// ========================================================================
	
	/// Calculate the amount of money to allocate to savings.
	static int64_t calculate_savings_amount(const MoneyAllocation& money_alloc, int64_t player_income);
	
	/// Calculate the amount of money to allocate to research.
	static int64_t calculate_research_amount(const MoneyAllocation& money_alloc, int64_t player_income);
	
	/// Calculate the amount of money to allocate to planetary development.
	static int64_t calculate_planets_amount(double total_planets_fraction, int64_t player_income);
	
	/// Calculate the amount of money to allocate to a specific research stream.
	static int64_t calculate_research_stream_amount(const ResearchAllocation& research,
	                                                 TechStream stream,
	                                                 int64_t research_budget);
};

#endif // OPENHO_PLAYER_H
