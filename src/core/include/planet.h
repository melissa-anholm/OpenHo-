#ifndef OPENHO_PLANET_H
#define OPENHO_PLANET_H

#include <cstdint>
#include <string>
#include "enums.h"

// ============================================================================
// Forward Declarations
// ============================================================================

typedef uint32_t EntityID;
typedef uint32_t PlayerID;
typedef double GalaxyCoord;

class Player;  // Forward declaration

// ============================================================================
// Data Structures
// ============================================================================

// Forward declaration of PlanetSnapshot (defined after Planet class)
struct PlanetSnapshot;

// Planet structure (actual state on the host)
class Planet
{
public:
	uint32_t id;
	std::string name;
	GalaxyCoord x;
	GalaxyCoord y;
	
	// Immutable properties
	double true_gravity;
	
	// Mutable properties
	double true_temperature;
	int32_t metal;
	
	PlayerID owner;  // 0 if unowned
//	bool can_be_profitable;
//	int perceived_value;
	
	// Nova state
	PlanetNovaState nova_state;
	
	// Constructor
	Planet(
		uint32_t p_id,
		const std::string& p_name,
		GalaxyCoord p_x,
		GalaxyCoord p_y,
		double p_true_gravity,
		double p_true_temperature,
		int32_t p_metal,
		PlayerID p_owner = 0);
};

// Colonized planet - player-specific view of a planet with allocation information
class ColonizedPlanet
{
public:
	// Budget split for allocating planet resources between mining and terraforming
	struct PlanetaryBudgetSplit
	{
		double mining_fraction;
		double terraforming_fraction;
		
		void normalize()
		{
			double total = mining_fraction + terraforming_fraction;
			if (total == 0.0) { total = 1.0; mining_fraction = 0.5; terraforming_fraction = 0.5; }
			mining_fraction /= total;
			terraforming_fraction /= total;
		}
		void enforce_positive()
		{
			// 1. Ensure values are non-negative
			if (mining_fraction < 0.0) { mining_fraction *= -1.0; }
			if (terraforming_fraction < 0.0) { terraforming_fraction *= -1.0; }
			
			// 2. Cap values at 1.0.
			if (mining_fraction >= 1.0) { mining_fraction = 1.0; terraforming_fraction = 0.0; }
			else if (terraforming_fraction >= 1.0) { mining_fraction = 0.0; terraforming_fraction = 1.0; }
		}
		void positive_normalize()
		{
			enforce_positive();
			normalize();
		}
		PlanetaryBudgetSplit(double input_mining = 0.3, double input_terra = 0.7)
			: mining_fraction(input_mining)
			, terraforming_fraction(input_terra)
		{
			positive_normalize();
		}
	};

private:
	// Pointer to the base planet (not owned by this instance)
	Planet* base_planet;
	
	// Pointer to the owner player (not owned by this instance)
	Player* owner_player;
	
	// Player-specific data (Private)
	double planet_funding_fraction;
	int32_t population;
	int32_t income;
	PlanetaryBudgetSplit budget_split;
	
	// Apparent values based on owner's preferences
	double apparent_gravity;
	double apparent_temperature;
	
public:
	// Constructor: Creates a player-specific view of a planet
	ColonizedPlanet(
		Planet* p_base,
		Player* p_owner,
		int32_t p_pop = 10,
		int32_t p_income = -7501,
		double p_funding = 0.2,
		PlanetaryBudgetSplit p_budget = PlanetaryBudgetSplit());
	
	// --- Accessors to base planet ---
	uint32_t get_id() const { return base_planet->id; }
	const std::string& get_name() const { return base_planet->name; }
	GalaxyCoord get_x() const { return base_planet->x; }
	GalaxyCoord get_y() const { return base_planet->y; }
	double get_true_gravity() const { return base_planet->true_gravity; }
	double get_true_temperature() const { return base_planet->true_temperature; }
	int32_t get_metal() const { return base_planet->metal; }
	void set_metal(int32_t p_val) { base_planet->metal = p_val; }
	PlayerID get_owner() const { return base_planet->owner; }
	PlanetNovaState get_nova_state() const { return base_planet->nova_state; }
	void set_nova_state(PlanetNovaState p_val) { base_planet->nova_state = p_val; }
	
	// --- Getters for player-specific data ---
	double get_funding_fraction() const { return planet_funding_fraction; }
	int32_t get_population() const { return population; }
	int32_t get_income() const { return income; }
	double get_mining_fraction() const { return budget_split.mining_fraction; }
	double get_terraforming_fraction() const { return budget_split.terraforming_fraction; }
	double get_apparent_gravity() const { return apparent_gravity; }
	double get_apparent_temperature() const { return apparent_temperature; }
	
	// --- Setters for player-specific data ---
	void set_funding_fraction(double p_val) { planet_funding_fraction = p_val; }
	void set_population(int32_t p_val) { population = p_val; }
	void set_income(int32_t p_val) { income = p_val; }
	void set_apparent_gravity(double p_val) { apparent_gravity = p_val; }
	void set_apparent_temperature(double p_val) { apparent_temperature = p_val; }
	
	void set_budget_split(double p_mining, double p_terra)
	{
		budget_split.mining_fraction = p_mining;
		budget_split.terraforming_fraction = p_terra;
		budget_split.positive_normalize();
	}
	
	void set_mining_fraction(double p_val)
	{
		budget_split.mining_fraction = p_val;
		budget_split.enforce_positive();
		budget_split.terraforming_fraction = 1.0 - budget_split.mining_fraction;
	}

	void set_terraforming_fraction(double p_val)
	{
		budget_split.terraforming_fraction = p_val;
		budget_split.enforce_positive();
		budget_split.mining_fraction = 1.0 - budget_split.terraforming_fraction;
	}
};

// ============================================================================
// PlanetSnapshot Definition (after Planet class)
// ============================================================================

// Planet information snapshot (what a player knows about a planet)
struct PlanetSnapshot
{
	uint32_t id;
	std::string name;
	GalaxyCoord x;
	GalaxyCoord y;
	
	PlayerID as_seen_by;  // Which player created this snapshot
	double apparent_temperature;
	double apparent_gravity;
	int32_t metal;
	PlayerID apparent_owner;
	int32_t population;
	int32_t income;
	uint32_t observation_year;  // When this information was collected
	
	bool can_be_profitable;
	int perceived_value;
	
	// Factory methods to create snapshots
	// partial_info: only basic data (id, name, coordinates) are copied
	// full_info: all available fields are copied with apparent values calculated
	static PlanetSnapshot partial_info(const Planet& planet, PlayerID player_id);
	static PlanetSnapshot full_info(const Planet& planet, PlayerID player_id, Player* owner_player);
};

#endif // OPENHO_PLANET_H
