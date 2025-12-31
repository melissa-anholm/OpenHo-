#ifndef OPENHO_PLAYER_H
#define OPENHO_PLAYER_H

#include "planet.h"
#include "ship_design.h"
#include "fleet.h"
#include <cstdint>
#include <vector>
#include <string>

// ============================================================================
// Forward Declarations
// ============================================================================

typedef uint32_t PlayerID;

// ============================================================================
// Data Structures
// ============================================================================

// Public information about a player (visible to all other players)
struct PlayerPublicInfo
{
	uint32_t player_id;
	uint32_t turn;
	
	// Technology levels (subset - no Radical)
	int32_t tech_range;
	int32_t tech_speed;
	int32_t tech_weapons;
	int32_t tech_shields;
	int32_t tech_miniaturization;
	
	// Resources
	int64_t money_income;
	int64_t money_savings;
	int64_t metal_savings;
	int64_t income;  // Total income this turn (for historical tracking)
	
	// Calculated metrics
	int64_t ship_power;
	int32_t victory_points;
};

// Player structure
// IMPORTANT: Player IDs must never be 0!
// PlayerID 0 is reserved to mean "NOT_OWNED" for planets.
// Player IDs must start from 1 and increment from there.
class Player
{
	friend class GameState;  // GameState has access to all private members
	// Note: C API functions in c_api.cpp access Player through GameState, which is a friend

public:
	uint32_t id;
	std::string name;
	PlayerType type;
	int32_t iq;  // For computer players
	
	// ========================================================================
	// Public Accessors
	// ========================================================================
	
	/// Get this player's ideal temperature for planets
	double get_ideal_temperature() const { return ideal_temperature; }
	
	/// Get this player's ideal gravity for planets
	double get_ideal_gravity() const { return ideal_gravity; }
	
	/// Get all fleets owned by this player
	const std::vector<Fleet>& get_fleets() const { return fleets; }
	
	// Technology levels for a player
	struct TechnologyLevels
	{
		int32_t range;
		int32_t speed;
		int32_t weapons;
		int32_t shields;
		int32_t miniaturization;
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
		double research_miniaturization_fraction;
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
		int64_t research_points_miniaturization;
		int64_t research_points_radical;
	};

private:
	// Resources
	int64_t money;  // Savings account
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
	
	// Fog of war: player's knowledge of planets
	std::vector<PlanetSnapshot> planetKnowledge;
	
	// Ship designs
	std::vector<ShipDesign> ship_designs;      // All designs, ordered by creation (max 100)
	uint32_t next_ship_design_id;                // Counter for unique design IDs (never resets)
	
	// Fleets (groups of identical ships)
	std::vector<Fleet> fleets;                 // All fleets owned by this player
	uint32_t next_fleet_id;                    // Counter for unique fleet IDs (never resets)
	
	// Public accessor for current turn's public information
	[[nodiscard]] PlayerPublicInfo get_player_public_info_current() const;

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
