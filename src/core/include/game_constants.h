#ifndef OPENHO_GAME_CONSTANTS_H
#define OPENHO_GAME_CONSTANTS_H

#include <cstdint>

// ============================================================================
// Game Balance Constants
// ============================================================================
// This file contains all tunable numeric constants for game balance.
// Modify these values to adjust game difficulty and mechanics.

namespace GameConstants
{
	// ========================================================================
	// Ship Design Limits
	// ========================================================================
	
	/// Maximum number of ship designs a player can have simultaneously.
	/// A player can create more designs over time, but only up to this many
	/// can exist at once. Creating a new design when at the limit requires
	/// deleting an existing design first.
	constexpr uint32_t Max_Ship_Designs_Per_Player = 100;
	
	// ========================================================================
	// Money and Resources
	// ========================================================================
	
	/// Interest rate applied to positive money savings each turn (as a percentage).
	/// Example: 15 means 15% interest per turn on positive savings.
	constexpr int32_t Money_Interest_Rate_Positive = 15;  // 15%
	
	/// Interest rate applied to negative money savings (debt) each turn (as a percentage).
	/// Example: 30 means 30% interest per turn on debt.
	/// This represents the cost of borrowing money.
	constexpr int32_t Money_Interest_Rate_Debt = 30;  // 30%
	
	// ========================================================================
	// Planet Mechanics
	// ========================================================================
	
	/// Maximum number of turns a planet can remain in nova warning state
	/// before being destroyed. The actual warning duration is determined
	/// by a Poisson distribution formula.
	constexpr int32_t Max_Nova_Warning_Turns = 15;
	
	// ========================================================================
	// Future Balance Parameters
	// ========================================================================
	// Add new constants here as needed for game balance tuning.
	
}  // namespace GameConstants

#endif  // OPENHO_GAME_CONSTANTS_H
