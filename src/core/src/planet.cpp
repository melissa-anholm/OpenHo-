#include "planet.h"

// ============================================================================
// BudgetSplit Implementation
// ============================================================================


/*
ColonizedPlanet::BudgetSplit::BudgetSplit()
	: mining_fraction(0.5), terraforming_fraction(0.5)
{
}

double ColonizedPlanet::BudgetSplit::get_mining_fraction() const
{
	return mining_fraction;
}

double ColonizedPlanet::BudgetSplit::get_terraforming_fraction() const
{
	return terraforming_fraction;
}

void ColonizedPlanet::BudgetSplit::set_mining_fraction(double fraction)
{
	mining_fraction = fraction;
	normalize();
}

void ColonizedPlanet::BudgetSplit::set_terraforming_fraction(double fraction)
{
	terraforming_fraction = fraction;
	normalize();
}

void ColonizedPlanet::BudgetSplit::normalize()
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
*/


// ============================================================================
// ColonizedPlanet Accessors
// ============================================================================
/*
ColonizedPlanet::BudgetSplit& ColonizedPlanet::get_budget_split()
{
	return budget_split;
}

const ColonizedPlanet::BudgetSplit& ColonizedPlanet::get_budget_split() const
{
	return budget_split;
}
*/