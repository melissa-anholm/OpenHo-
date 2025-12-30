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
	
	// Nova state
	PlanetState state;
};

// Colonized planet - extends Planet with player-specific allocation information
class ColonizedPlanet : public Planet
{
private:
	// Budget split for allocating planet resources between mining and terraforming
	struct BudgetSplit
	{
	private:
		double mining_fraction;
		double terraforming_fraction;
		
	public:
		// Constructor
		BudgetSplit();
		
		// Getters
		double get_mining_fraction() const;
		double get_terraforming_fraction() const;
		
		// Setters
		void set_mining_fraction(double fraction);
		void set_terraforming_fraction(double fraction);
		
		// Normalize the fractions so they sum to 1.0
		void normalize();
	};
	
	BudgetSplit budget_split;            // How to split this planet's budget between mining/terraforming

public:
	// Accessors for budget split
	BudgetSplit& get_budget_split();
	const BudgetSplit& get_budget_split() const;
	
	// Member variables
	double planet_funding_fraction;      // What fraction of total planet budget this planet gets
	
	// Player-specific properties (only for colonized planets)
	int32_t population;                  // Population on this planet
	int32_t income;                      // Calculated based on population, temperature, gravity, and owner's ideals
};

#endif // OPENHO_PLANET_H
