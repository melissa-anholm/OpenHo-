
#include "openho_core.h"
#include "player.h"
#include <cmath>

// ============================================================================
// Money Allocation Calculation Helpers
// ============================================================================

int64_t Player::calculate_savings_amount(const MoneyAllocation& money_alloc, int64_t player_income)
{
	double savings_amount = player_income * money_alloc.savings_fraction;
	return static_cast<int64_t>(std::round(savings_amount));
}

int64_t Player::calculate_research_amount(const MoneyAllocation& money_alloc, int64_t player_income)
{
	double research_amount = player_income * money_alloc.research_fraction;
	return static_cast<int64_t>(std::round(research_amount));
}

int64_t Player::calculate_planets_amount(double total_planets_fraction, int64_t player_income)
{
	double planets_amount = player_income * total_planets_fraction;
	return static_cast<int64_t>(std::round(planets_amount));
}

int64_t Player::calculate_research_stream_amount( const ResearchAllocation& player_research,
                                                  TechStream tech_type,
                                                  int64_t research_budget)
{
	double stream_fraction = 0.0;
	
	switch (tech_type)
	{
		case TECH_RANGE:
			stream_fraction = player_research.research_range_fraction;
			break;
		case TECH_SPEED:
			stream_fraction = player_research.research_speed_fraction;
			break;
		case TECH_WEAPONS:
			stream_fraction = player_research.research_weapons_fraction;
			break;
		case TECH_SHIELDS:
			stream_fraction = player_research.research_shields_fraction;
			break;
		case TECH_MINIATURIZATION:
			stream_fraction = player_research.research_miniaturization_fraction;
			break;
		case TECH_RADICAL:
			stream_fraction = player_research.research_radical_fraction;
			break;
		default:
			return 0;
	}
	
	double tech_stream_amount = research_budget * stream_fraction;
	return static_cast<int64_t>(std::round(tech_stream_amount));
}
