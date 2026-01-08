#ifndef OPENHO_COLONIZED_PLANET_H
#define OPENHO_COLONIZED_PLANET_H

#include <cstdint>
#include <string>
#include "enums.h"
#include "planet.h"

// ============================================================================
// Forward Declarations
// ============================================================================

typedef uint32_t EntityID;
typedef int32_t PlayerID;
typedef double GalaxyCoord;

class Player;

// ============================================================================
// ColonizedPlanet Class
// ============================================================================

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
			: mining_fraction(input_mining), terraforming_fraction(input_terra)
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
	
	// Planet desirability rating (1-3 scale)
	int32_t desirability;  // 1 = poor, 2 = moderate, 3 = excellent
	
public:
	// Constructor: Creates a player-specific view of a planet
	ColonizedPlanet(
		Planet* p_base,
		Player* p_owner,
		int32_t p_pop = 10,
		int32_t p_income = -7501,
		double p_funding = 0.2,
		PlanetaryBudgetSplit p_budget = PlanetaryBudgetSplit(),
		int32_t p_desirability = 2);
	
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
	int32_t get_desirability() const { return desirability; }
	
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
	
	// Update desirability based on current conditions
	void update_desirability();
	
	// Get desirability as a descriptive string
	const char* get_desirability_description() const
	{
		switch (desirability)
		{
			case 1: return "Yuck!";
			case 2: return "Hmmm.";
			case 3: return "Oooh!";
			default: return "Unknown";
		}
	}
};

#endif // OPENHO_COLONIZED_PLANET_H
