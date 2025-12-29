#include "money_allocation_calculator.h"
#include <cmath>

// ============================================================================
// Top-Level Income Allocation
// ============================================================================

int64_t MoneyAllocationCalculator::calculate_savings_amount(const Player::MoneyAllocation& money_alloc, int64_t player_income)
{
	double savings_amount = player_income * money_alloc.savings_fraction;
	return static_cast<int64_t>(std::round(savings_amount));
}

int64_t MoneyAllocationCalculator::calculate_research_amount(const Player::MoneyAllocation& money_alloc, int64_t player_income)
{
	double research_amount = player_income * money_alloc.research_fraction;
	return static_cast<int64_t>(std::round(research_amount));
}

int64_t MoneyAllocationCalculator::calculate_planets_amount(double total_planets_fraction, int64_t player_income)
{
	double planets_amount = player_income * total_planets_fraction;
	return static_cast<int64_t>(std::round(planets_amount));
}

// ============================================================================
// Research Stream Allocation
// ============================================================================

int64_t MoneyAllocationCalculator::calculate_research_stream_amount(const ResearchAllocation& research,
                                                                     TechStream stream,
                                                                     int64_t research_budget)
{
	double stream_fraction = 0.0;
	
	switch (stream)
	{
		case TECH_RANGE:
			stream_fraction = research.research_range_fraction;
			break;
		case TECH_SPEED:
			stream_fraction = research.research_speed_fraction;
			break;
		case TECH_WEAPONS:
			stream_fraction = research.research_weapons_fraction;
			break;
		case TECH_SHIELDS:
			stream_fraction = research.research_shields_fraction;
			break;
		case TECH_MINIATURIZATION:
			stream_fraction = research.research_miniaturization_fraction;
			break;
		case TECH_RADICAL:
			stream_fraction = research.research_radical_fraction;
			break;
		default:
			return 0;
	}
	
	double stream_amount = research_budget * stream_fraction;
	return static_cast<int64_t>(std::round(stream_amount));
}


