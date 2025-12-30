#ifndef OPENHO_PLANET_H
#define OPENHO_PLANET_H

#include <cstdint>
#include <string>

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
public:
	// Budget split for this planet (how to divide its budget between mining and terraforming)
	struct BudgetSplit
	{
	private:
		double mining_fraction;
		double terraforming_fraction;
		
	public:
		// Constructor
		BudgetSplit() : mining_fraction(0.5), terraforming_fraction(0.5) {}
		
		// Getters
		double get_mining_fraction() const
		{
			return mining_fraction;
		}
		
		double get_terraforming_fraction() const
		{
			return terraforming_fraction;
		}
		
		// Setters
		void set_mining_fraction(double fraction)
		{
			mining_fraction = fraction;
			normalize();
		}
		
		void set_terraforming_fraction(double fraction)
		{
			terraforming_fraction = fraction;
			normalize();
		}
		
		// Normalize the fractions so they sum to 1.0
		void normalize()
		{
			double total = mining_fraction + terraforming_fraction;
			if (total > 0.0)
			{
				mining_fraction /= total;
				terraforming_fraction /= total;
			}
			else
			{
				// If both are zero, split evenly
				mining_fraction = 0.5;
				terraforming_fraction = 0.5;
			}
		}
	};
	
	// Member variables
	double planet_funding_fraction;      // What fraction of total planet budget this planet gets
	BudgetSplit budget_split;            // How to split this planet's budget between mining/terraforming
	
	// Player-specific properties (only for colonized planets)
	int32_t population;                  // Population on this planet
	int32_t income;                      // Calculated based on population, temperature, gravity, and owner's ideals
};

#endif // OPENHO_PLANET_H
