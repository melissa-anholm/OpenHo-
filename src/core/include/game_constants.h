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
	constexpr double min_gravity = 0.3;
	constexpr double max_gravity = 4.5;
	
	constexpr double min_temp = -400.0;
	constexpr double max_temp =  400.0;
	
	constexpr int32_t min_metal = 0;
	constexpr int32_t max_metal = 32000;
	
	constexpr double min_planet_distance = 4.0;
	
	constexpr double galaxy_max_density = 4.0;
	constexpr double galaxy_min_density = 0.25;
	
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
	
	/// Interest rate applied to positive money savings each turn.
	/// Example: 0.15 means 15% interest per turn on positive savings.
	constexpr double Money_Interest_Rate_Positive = 0.15;
	
	/// Interest rate applied to negative money savings (debt) each turn.
	/// Example: 0.30 means 30% interest per turn on debt.
	/// This represents the cost of borrowing money.
	constexpr double Money_Interest_Rate_Debt = 0.30;
	
	// ========================================================================
	// Planet Placement
	// ========================================================================
	
	/// Maximum number of attempts to find a valid position for a planet
	/// when enforcing minimum distance constraints.
	/// If a planet cannot be placed after this many attempts, it is skipped.
	constexpr uint32_t Planet_Placement_Max_Attempts = 10;
	
	/// Debug threshold: log a warning if planet placement takes more than
	/// this many attempts. Set to 5 for debug builds, higher for release.
	constexpr uint32_t Planet_Placement_Debug_Threshold = 5;
	
	/// Galaxy size scaling constants for random shape distribution.
	/// Formula: gal_size = sqrt(n_planets) * (Galaxy_Size_Scale_Base + Galaxy_Size_Scale_Density / density)
	/// This ensures:
	/// - At density=2.0: compact galaxy with 4-5 neighbors within radius 6
	/// - At density=0.5: sparse galaxy with 3-4 neighbors within radius 12
	/// - Scales appropriately with planet count
	constexpr double Galaxy_Size_Scale_Base = 5.0;
	constexpr double Galaxy_Size_Scale_Density = 6.4;
	
	/// Reduction factor applied to initial galaxy size.
	/// Helps ensure planets can be placed without excessive expansion.
	constexpr double Galaxy_Size_Reduction_Factor = 0.15;
	
	/// Expansion factor applied to galaxy boundaries when placement becomes difficult.
	/// Each expansion increases both x and y dimensions by this percentage.
	constexpr double Galaxy_Expansion_Factor = 0.05;
	
	// ========================================================================
	// Starting Planet Constants
	// ========================================================================
	
	/// Suitable gravity range for starting planets [min, max]
	/// Players' ideal_gravity will be set to their starting planet's true_gravity
	/// This range ensures starting planets have reasonable gravity values
	constexpr double Starting_Planet_Min_Gravity = 0.5;
	constexpr double Starting_Planet_Max_Gravity = 2.0;
	
	/// Starting colony initial values indexed by StartingColonyQuality
	/// Access: Starting_Colony_Population[START_NORMAL], etc.
	/// 
	/// TODO: These values will need to be adjusted during game balance testing.
	/// Current values are placeholders with reasonable progression.
	/// 
	/// Quality Levels:
	/// - START_OUTPOST (0): minimal values
	/// - START_BARREN (1): low values
	/// - START_BACKWARD (2): below normal values
	/// - START_NORMAL (3): baseline values
	/// - START_ADVANCED (4): above normal values
	/// - START_THRIVING (5): high values
	/// - START_ABUNDANT (6): very high values
	
	constexpr int32_t Starting_Colony_Population[] = {
		500000,      // START_OUTPOST
		750000,      // START_BARREN
		875000,      // START_BACKWARD
		1000000,     // START_NORMAL
		1250000,     // START_ADVANCED
		1500000,     // START_THRIVING
		2000000      // START_ABUNDANT
	};
	
	constexpr int32_t Starting_Colony_Metal[] = {
		5000,        // START_OUTPOST
		10000,       // START_BARREN
		12500,       // START_BACKWARD
		15000,       // START_NORMAL
		18750,       // START_ADVANCED
		22500,       // START_THRIVING
		30000        // START_ABUNDANT
	};
	
	constexpr int32_t Starting_Colony_Income[] = {
		10000,       // START_OUTPOST
		20000,       // START_BARREN
		25000,       // START_BACKWARD
		30000,       // START_NORMAL
		37500,       // START_ADVANCED
		45000,       // START_THRIVING
		60000        // START_ABUNDANT
	};
	
	/// Minimum planet buffer for galaxy generation
	/// When calculating minimum required planets for N players:
	/// min_planets = N / suitable_fraction * Min_Planet_Buffer
	/// This 10% buffer ensures we have extra planets beyond the minimum
	constexpr double Min_Planet_Buffer = 1.1;
	
	/// Maximum number of retries when galaxy generation fails to produce
	/// enough suitable home planets for the requested number of players
	constexpr uint32_t Galaxy_Gen_Retry_Count = 5;
	
	// ========================================================================
	// Future Balance Parameters
	// ========================================================================
	// Add new constants here as needed for game balance tuning.
	
}  // namespace GameConstants

#endif  // OPENHO_GAME_CONSTANTS_H
