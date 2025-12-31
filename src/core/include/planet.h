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

// ============================================================================
// Data Structures
// ============================================================================

// Planet information snapshot (what a player knows about a planet)
struct PlanetInfo
{
	double temperature;
	double gravity;
	int32_t metal;
	std::string name;
	PlayerID owner;
	int32_t population;
	int32_t income;
	uint32_t observation_turn;  // When this information was collected
};

// Planet structure (actual state on the host)
class Planet
{
public:
	uint32_t id;
	std::string name;
	GalaxyCoord x;
	GalaxyCoord y;
	
	// Immutable properties
	double gravity;
	
	// Mutable properties
	double temperature;
	int32_t metal;
	
	PlayerID owner;  // 0 if unowned
//	bool can_be_profitable;
//	int perceived_value;
	
	// Nova state
	PlanetNovaState nova_state;
};

// Colonized planet - extends Planet with player-specific allocation information
class ColonizedPlanet : public Planet
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
	// Player-specific data (Private)
	double planet_funding_fraction;
	int32_t population;
	int32_t income;
	PlanetaryBudgetSplit budget_split;

public:
	// Constructor: Promotes an existing Planet to a ColonizedPlanet
	ColonizedPlanet(
		const Planet& p_base,
		int32_t p_pop = 10,
		int32_t p_income = -7501,
		double p_funding = 0.2,
		PlanetaryBudgetSplit p_budget = PlanetaryBudgetSplit() )
		: Planet(p_base), 
		  planet_funding_fraction(p_funding), 
		  population(p_pop), 
		  income(p_income), 
		  budget_split(p_budget)
		{ }

	// --- Getters ---
	double get_funding_fraction() const { return planet_funding_fraction; }
	int32_t get_population() const { return population; }
	int32_t get_income() const { return income; }
	double get_mining_fraction() const { return budget_split.mining_fraction; }
	double get_terraforming_fraction() const { return budget_split.terraforming_fraction; }

	// --- Setters ---
	void set_funding_fraction(double p_val) { planet_funding_fraction = p_val; }
	void set_population(int32_t p_val) { population = p_val; }
	void set_income(int32_t p_val) { income = p_val; }
	
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

#endif // OPENHO_PLANET_H
