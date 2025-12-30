#ifndef OPENHO_MONEY_ALLOCATION_CALCULATOR_H
#define OPENHO_MONEY_ALLOCATION_CALCULATOR_H

#include "openho_core.h"
#include <cstdint>

// ============================================================================
// MoneyAllocationCalculator Class
// ============================================================================

/**
 * Calculates absolute money amounts from allocation fractions.
 * 
 * Converts the normalized fractions in MoneyAllocation, ResearchAllocation,
	 * and allocation information into actual money amounts based on a
 * player's income.
 * 
 * Note: This calculator does not inspect the internal structure of allocation
 * objects beyond what is explicitly passed as parameters. Callers are responsible
 * for computing aggregate values (like total planets fraction) before passing them
 * to the calculator.
 */
class MoneyAllocationCalculator
{
public:
	// ========================================================================
	// Top-Level Income Allocation
	// ========================================================================
	
	/// Calculate the amount of money to allocate to savings.
	/// 
	/// @param money_alloc The player's money allocation
	/// @param player_income The player's total income this turn
	/// @return The amount to add to savings
	static int64_t calculate_savings_amount(const Player::MoneyAllocation& money_alloc, int64_t player_income);
	
	/// Calculate the amount of money to allocate to research.
	/// 
	/// @param money_alloc The player's money allocation
	/// @param player_income The player's total income this turn
	/// @return The amount to allocate to research
	static int64_t calculate_research_amount(const Player::MoneyAllocation& money_alloc, int64_t player_income);
	
	/// Calculate the amount of money to allocate to planetary development.
	/// 
	/// @param total_planets_fraction The sum of all planet planets_fractions
	/// @param player_income The player's total income this turn
	/// @return The total amount to allocate to all planets
	static int64_t calculate_planets_amount(double total_planets_fraction, int64_t player_income);
	
	// ========================================================================
	// Research Stream Allocation
	// ========================================================================
	
	/// Calculate the amount of money to allocate to a specific research stream.
	/// 
	/// @param research The research allocation fractions
	/// @param stream The technology stream to calculate for
	/// @param research_budget The total research budget for this turn
	/// @return The amount to allocate to this research stream
	static int64_t calculate_research_stream_amount(const Player::ResearchAllocation& research,
	                                                  TechStream stream,
	                                                  int64_t research_budget);
	
};

#endif  // OPENHO_MONEY_ALLOCATION_CALCULATOR_H
