#ifndef OPENHO_SHIP_DESIGN_H
#define OPENHO_SHIP_DESIGN_H

#include "enums.h"
#include <cstdint>
#include <string>

// ============================================================================
// Forward Declarations
// ============================================================================

// Forward declaration for friend function
void set_ship_design_tech(struct ShipDesign& design, int32_t range, int32_t speed, 
                          int32_t weapons, int32_t shields, int32_t mini);

// ============================================================================
// Ship Design Structure
// ============================================================================

/// Ship design pattern (immutable once created)
/// Represents a blueprint for building ships with specific technology levels and costs
struct ShipDesign
{
public:
	uint32_t id;                    // Unique ID within player's design library
	std::string name;
	ShipType type;
	
	// Costs (calculated at design creation time, immutable)
	int64_t build_cost;              // Cost to build each ship of this design
	int64_t prototype_cost;          // Cost for the first ship of this design
	int64_t metal_cost;              // Metal cost per ship
	
	// Constructor to set tech levels during creation
	ShipDesign() = default;
	ShipDesign(int32_t range, int32_t speed, int32_t weapons, int32_t shields, int32_t mini)
		: tech_range(range), tech_speed(speed), tech_weapons(weapons), 
		  tech_shields(shields), tech_mini(mini) {}
	
	// Public getter methods for technology levels (read-only)
	int32_t get_range() const   { return tech_range; }
	int32_t get_speed() const   { return tech_speed; }
	int32_t get_weapons() const { return tech_weapons; }
	int32_t get_shields() const { return tech_shields; }
	int32_t get_mini() const    { return tech_mini; }

private:
	// Technology levels associated with this design (immutable after construction)
	int32_t tech_range = 0;
	int32_t tech_speed = 0;
	int32_t tech_weapons = 0;
	int32_t tech_shields = 0;
	int32_t tech_mini = 0;
	
	// Allow game.cpp to set tech levels during design creation
	friend void set_ship_design_tech(ShipDesign& design, int32_t range, int32_t speed, 
	                                  int32_t weapons, int32_t shields, int32_t mini);
};

// ============================================================================
// Helper Function for Tech Level Initialization
// ============================================================================

/// Helper function to initialize tech levels during ship design creation
/// This is the only way to set tech levels after construction
inline void set_ship_design_tech(ShipDesign& design, int32_t range, int32_t speed, 
                                 int32_t weapons, int32_t shields, int32_t mini)
{
	design.tech_range = range;
	design.tech_speed = speed;
	design.tech_weapons = weapons;
	design.tech_shields = shields;
	design.tech_mini = mini;
}

#endif // OPENHO_SHIP_DESIGN_H
