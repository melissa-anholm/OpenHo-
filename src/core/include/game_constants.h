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
		
	// Temperature constants in Kelvin
	// Range: 40K to 500K (approximately -233°C to 227°C, or -388°F to 440°F)
	constexpr double min_temp = 40.0;   // Kelvin
	constexpr double max_temp = 500.0;  // Kelvin
	
	constexpr int32_t min_metal = 0;
	constexpr int32_t max_metal = 32000;  // ...for a planet to start with.
	
	/// Best perceived temperature for any player (approximately 72°F or 22°C)
	/// Used in temperature perception formula: perceived = best_perceived_temp / ideal_temp * true_temp
	/// This represents the "Goldilocks" temperature that all players perceive as ideal when
	/// their ideal_temperature matches this value.
	constexpr double best_perceived_temperature_K = 295.37223;  // Approximately 72°F
	
	/// One-sided range for ideal temperature assignment (in Kelvin)
	/// Players' ideal_temperature values are selected from a truncated Gaussian distribution
	/// centered on best_perceived_temperature_K with sigma = ideal_temp_range.
	/// Valid range: [best_perceived_temperature_K - ideal_temp_range, best_perceived_temperature_K + ideal_temp_range]
	constexpr double ideal_temp_range = 55.0;  // One-sided range in Kelvin
	
	/// Best perceived gravity for any player (1.0g = Earth gravity)
	/// Used in gravity perception formula: perceived = best_perceived_gravity / ideal_gravity * true_gravity
	/// This represents the "Goldilocks" gravity that all players perceive as ideal when
	/// their ideal_gravity matches this value.
	constexpr double best_perceived_gravity = 1.0;  // Earth gravity
		
	/// Suitable gravity range for starting planets [min, max]
	/// Players' ideal_gravity will be set to their starting planet's true_gravity
	/// This range ensures starting planets have reasonable gravity values
	constexpr double Starting_Planet_Min_Gravity = 0.7;
	constexpr double Starting_Planet_Max_Gravity = 1.4;
	
	// Galaxy Generation Stuff:
	constexpr double galaxy_max_density = 4.0;
	constexpr double galaxy_min_density = 0.25;
	
	constexpr double min_planet_distance = 4.0;
	
	
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
	
	// Interest rates removed - now calculated dynamically in GameFormulas::calculate_money_interest()
	// Positive savings: 10 * sqrt(savings)
	// Negative savings (debt): 0.15 * savings
	
	// ========================================================================
	// Galaxy Construction / Planet Placement Abstract Parameters
	// ========================================================================
	
	/// Maximum number of attempts to find a valid position for a planet
	/// when enforcing minimum distance constraints.
	/// If a planet cannot be placed after this many attempts, it is skipped.
	constexpr uint32_t Planet_Placement_Max_Attempts = 10;
	
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
	/// Probably used only for "random" style galaxies.  Probably.
	constexpr double Galaxy_Expansion_Factor = 0.05;
	
	// ========================================================================
	// Starting Planet Constants
	// ========================================================================

	/// TODO: These values for money and metal availability will need to be 
	/// adjusted during game balance testing.  Current values are placeholders 
	/// with reasonable progression.
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
	
	// ========================================================================
	// Future Balance Parameters
	// ========================================================================
	// Add new constants here as needed for game balance tuning.
	
}  // namespace GameConstants

#endif  // OPENHO_GAME_CONSTANTS_H
