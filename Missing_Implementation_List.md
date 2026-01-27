# Missing/Stubbed Implementation List

**Purpose:** Track all functions and values that need implementation or reverse-engineering from the original game.

**Last Updated:** January 26, 2026 (Session 13 - Economic Mechanics Complete)

---

## Table of Contents & Implementation Checklist

### Data-Driven Values (Require Reverse-Engineering) - 5 items

- [ ] [Planet Desirability Rating](#planet-desirability-rating) - Placeholder
- [ ] [Planetary Income Calculation](#planetary-income-calculation) - Placeholder
- [ ] [Ship Design Costs](#ship-design-costs) - Placeholder (3 functions)
- [ ] [Player Metrics](#player-metrics) - Placeholder (2 functions)
- [ ] [Population Growth](#population-growth) - Placeholder (ignores conditions)

### Non-Data-Driven Functions (Clear Logic, Need Implementation) - 5 items

- [ ] [Galaxy Shape Distribution Algorithms](#galaxy-shape-distribution-algorithms) - Stub (4 shapes)
  - [ ] Spiral galaxy distribution
  - [ ] Circle galaxy distribution
  - [ ] Ring galaxy distribution
  - [ ] Cluster galaxy distribution
- [ ] [Apparent Gravity/Temperature Perception](#apparent-gravitytemperature-perception) - IMPLEMENTED
- [ ] [Turn Processing](#turn-processing) - Partially implemented (5 missing steps)
- [ ] [Fleet Movement & Combat](#fleet-movement--combat) - Partially implemented (combat stub)
- [ ] [Nova Warning Duration](#nova-warning-duration) - Placeholder

### Partially Implemented (Need Completion) - 3 items

- [ ] [Ship Design Creation](#ship-design-creation) - Incomplete (3 missing steps)
- [ ] [Fleet Creation](#fleet-creation) - Incomplete (3 missing steps)
- [ ] [Player Initial Setup](#player-initial-setup) - Incomplete (9 missing steps)

### Infrastructure/Setup (Needed for Game Flow) - 1 item

- [ ] [Turn Ready Tracking](#turn-ready-tracking) - Stub (2 functions)

### Summary

**Total Items:** 20 functions/systems  
**Implemented:** 6 (30%)  
**Partially Implemented:** 3 (15%)  
**Placeholder/Stub:** 11 (55%)

---

## Data-Driven Values

These require playing through the original Spaceward Ho! and reverse-engineering the formulas/values used.

### Planet Desirability Rating

**Status:** ✅ Implemented (placeholder - all planets get desirability 3)  
**Location:** `planet.h` / `planet.cpp` (ColonizedPlanet class)

- `ColonizedPlanet::recalculate_desirability()` - Overall suitability rating for a planet
  - Depends on: apparent_gravity, apparent_temperature, metal_remaining, population (if colonized), etc.
  - Notes: Used by UI to display planet attractiveness; recalculated when colonized and each turn
  - Current: Placeholder returns 3 (all planets maximally desirable)
  - **Key Design Question:** What factors contribute to desirability? How are they weighted?
  - **Implementation Notes:** Infrastructure in place; ready for real formula when reverse-engineered

**Usage Pattern:**
1. **Exploration:** Calculate on-the-fly for UI display (not stored)
2. **Colonization:** Calculate and store with ColonizedPlanet object
3. **Turn Processing:** Recalculate each turn as part of planet processing

**Factors to Consider:**
- Gravity mismatch (via apparent_gravity)
- Temperature mismatch (via apparent_temperature)
- Metal availability (affects long-term value)
- Population (if colonized; affects current productivity)
- Existing infrastructure (if colonized)
- Distance from other planets (strategic value)
- Proximity to enemy planets (defensive value)

**Reverse-Engineering Approach:**
- Play original game and observe planet ratings
- Test with various gravity/temperature combinations
- Test with different metal amounts
- Determine if rating is numeric (0-100), star-based (1-5), or descriptive
- Identify which factors are most important
- Determine if rating changes based on player tech level or research

**Implementation Notes:**
- Should return a consistent numeric value (e.g., 0-100 or 0.0-1.0)
- Must account for player's ideal_gravity and ideal_temperature
- May need to consider player's current tech levels
- Should be deterministic (same inputs = same output)
- Consider if desirability should change as player researches new tech

---

### Planetary Income Calculation

**Status:** Placeholder (returns 0)  
**Location:** `game_formulas.cpp`

- `calculate_planetary_income()` - Total income from all owned planets
  - Depends on: population, mining allocation, terraforming progress, planet conditions, etc.
  - Notes: Critical for economy; affects all other calculations
  - Current: Always returns 0

**Reverse-Engineering Approach:**
- Create planets with different populations and allocations
- Observe income generated per turn
- Test with different mining/terraforming splits
- Determine if income is linear with population or other formula
- Check if planet conditions (gravity/temperature mismatch) affect income
- Determine base income per population unit

### Ship Design Costs

**Status:** Placeholder (returns 1 for all)  
**Location:** `game_formulas.cpp`

- `calculate_ship_design_build_cost()` - Cost to build each ship of a design
  - Depends on: tech_range, tech_speed, tech_weapons, tech_shields, tech_mini
  - Notes: Should scale with technology levels, likely non-linear
  
- `calculate_ship_design_prototype_cost()` - One-time cost for first ship of new design
  - Depends on: same tech parameters
  - Notes: Typically much higher than build cost
  
- `calculate_ship_design_metal_cost()` - Metal required per ship
  - Depends on: same tech parameters
  - Notes: Separate from money cost

**Reverse-Engineering Approach:**
- Create ships with varying tech levels in original game
- Note the costs displayed
- Build multiple ships and compare prototype vs. build costs
- Determine if costs are linear, quadratic, exponential, or other formula

---

### Player Metrics (Public Information)

**Status:** Placeholder (returns 1)  
**Location:** `game_formulas.cpp`

- `calculate_player_fleet_power()` - Combat strength metric
  - Depends on: all ships owned by player
  - Notes: Visible to all players; used for diplomacy/threat assessment
  - Logic: Sum combat values of all ships (likely based on weapons/shields tech)
  
- `calculate_player_victory_points()` - Overall progress metric
  - Depends on: planets owned, ships built, tech levels, population, etc.
  - Notes: Visible to all players; determines winner in some game modes
  - Logic: Weighted sum of various factors

**Implementation Notes:**
- Fleet power: Iterate through all player's fleets, sum ship combat values
- Victory points: Determine weighting for planets, population, tech levels, ships
- Both are primarily UI/display metrics, not core game mechanics

---

### Population Growth

**Status:** Placeholder (10% per turn, ignores planet conditions)  
**Location:** `game_formulas.cpp`

- `calculate_population_growth()` - Population change per turn
  - Depends on: current_population, planet_temperature, planet_gravity, ideal_temperature, ideal_gravity
  - Notes: Critical for economy; affects income generation
  - Current: Simple 10% growth regardless of conditions, capped at 1,000,000
  - Parameters for planet conditions are accepted but completely unused
  - **Key Design Question:** How do gravity/temperature mismatches affect growth? Linear penalty? Exponential? Threshold-based?

**Reverse-Engineering Approach:**
- Create planets with various gravity/temperature mismatches
- Observe population growth rates
- Test with ideal conditions vs. poor conditions
- Determine penalty formula for mismatch (linear? exponential? threshold?)
- Check if there's a minimum/maximum growth rate
- Test extreme mismatches to find breaking points

---

### Terraforming Efficiency

**Status:** ✅ IMPLEMENTED  
**Location:** `game_formulas.cpp`

- `calculate_temperature_change()` - Temperature adjustment from spending
  - Formula: `delta_temperature = 0.0811 × sqrt(money_spent)` (in Fahrenheit)
  - Converted to Kelvin for internal storage
  - Overshoot prevention: terraforming stops at ideal temperature
  - Refund logic for excess allocation
  - **Implementation:** Complete with proper unit conversions

**Reverse-Engineering Status:** ✅ Formula verified and implemented

---

### Mining Efficiency

**Status:** ✅ IMPLEMENTED  
**Location:** `game_formulas.cpp`

- `calculate_metal_mined()` - Metal extracted from planet per turn
  - Formula: `metal_extracted = 20.0 × sqrt(money_spent)`
  - Depletes planet metal reserves
  - Refund logic for insufficient planet metal
  - **Implementation:** Complete with proper resource management

**Reverse-Engineering Status:** ✅ Formula verified and implemented

---

### Interest Rates

**Status:** ✅ IMPLEMENTED  
**Location:** `game_formulas.cpp`

- `calculate_money_interest()` - Interest on savings/debt
  - Negative savings (debt): 15% interest (0.15 × savings)
  - Positive savings: Square root formula (10.0 × sqrt(savings))
  - Interest applied to player income, not savings directly
  - **Implementation:** Complete with dual rate system

**Reverse-Engineering Status:** ✅ Formula verified and implemented

---

### Income Allocation System

**Status:** ✅ IMPLEMENTED  
**Location:** `game.cpp` - `process_money_allocation()`

- Income distribution according to allocation fractions:
  - `savings_fraction` → added to money_savings
  - `research_fraction` → used for research
  - `planets_fraction` → used for mining/terraforming
- Money for mining/terraforming comes from income allocation, not savings
- Fixed critical bug where income was double-counted
- **Implementation:** Complete with proper fraction-based distribution

**Reverse-Engineering Status:** ✅ System verified and implemented

---

### Research Conversion

**Status:** Placeholder (1:1 conversion)  
**Location:** `game_formulas.cpp`

- `convert_money_to_research_points()` - Money → Research points
  - Depends on: money_allocated
  - Notes: Affects technology advancement speed
  - Current: Simple 1:1 conversion
  - **Key Design Question:** Is conversion always 1:1, or do different tech streams have different rates?

**Reverse-Engineering Approach:**
- Allocate money to research different streams
- Observe research point accumulation
- Check if conversion rate is consistent across streams
- Determine if there are diminishing returns
- Check if conversion rate changes with tech level

---

### Starting Colony Values

**Status:** ✅ IMPLEMENTED (placeholder values with reasonable progression)  
**Location:** `game_constants.h`

Current values (indexed by StartingColonyQuality):
- Population: 500k → 750k → 875k → 1M → 1.25M → 1.5M → 2M
- Metal: 5k → 10k → 12.5k → 15k → 18.75k → 22.5k → 30k
- Income: 10k → 20k → 25k → 30k → 37.5k → 45k → 60k

**Notes:**
- These are reasonable progressions but may not match original
- Affects game balance significantly
- Should be verified against original game

**Reverse-Engineering Approach:**
- Start new games with each quality level in original game
- Note initial population, metal, and income values
- Verify the progression pattern
- Check if progression is linear, exponential, or other

---

### Technology Advancement Costs

**Status:** ✅ IMPLEMENTED (quadratic formula)  
**Location:** `game_formulas.cpp`

Current formula: `cost = (level + 1)^2 * 100`

**Notes:**
- All tech streams use same formula
- May need different multipliers per stream
- Base multiplier of 100 is placeholder
- **Key Design Question:** Is the formula the same for all tech streams, or do different streams have different costs?

**Reverse-Engineering Approach:**
- Research each tech stream from level 0 to max in original game
- Note the research point cost for each level
- Determine if formula is quadratic, cubic, exponential, etc.
- Check if different streams have different costs or multipliers
- Verify the base multiplier value

---

### Nova Warning Duration

**Status:** Placeholder (returns 1)  
**Location:** `game_formulas.cpp`

- `calculate_planet_nova_warning_duration()` - Turns until planet destroyed
  - Depends on: DeterministicRNG
  - Notes: Determines how long a planet has before being destroyed
  - Current: Always returns 1
  - Logic: Generate random duration from appropriate distribution

**Implementation Notes:**
- Use DeterministicRNG to generate random value
- Determine appropriate distribution (uniform range? Poisson? other?)
- Ensure reproducible results for multiplayer games

---

## Non-Data-Driven Functions

These have clear logic but need implementation. No reverse-engineering needed.

### Galaxy Shape Distribution Algorithms

**Status:** Placeholder (all currently use random distribution)  
**Location:** `galaxy.cpp`

**Functions needing implementation:**

- `initialize_planets_spiral()` - Spiral arm galaxy distribution
  - Current: Falls back to random distribution
  - Design question: How many spiral arms? How tight? How to distribute planets along arms?
  - Notes: Should ensure suitable home planets are distributed across arms
  - TODO: Rework min planets/homeworld assignment strategy for spiral shape

- `initialize_planets_circle()` - Circular disk galaxy distribution
  - Current: Falls back to random distribution
  - Design question: Uniform circle or concentrated toward center? Poisson disk sampling?
  - Notes: Mentioned using Bridson's algorithm for Poisson disk sampling
  - Suggested approach: Uniform distribution within a circular boundary

- `initialize_planets_ring()` - Ring/torus galaxy distribution
  - Current: Falls back to random distribution
  - Design question: How wide is the ring? How to distribute planets around it?
  - Notes: Should create a visible ring structure
  - Suggested approach: Generate points at varying radii within a ring band

- `initialize_planets_cluster()` - Clustered galaxy distribution
  - Current: Falls back to random distribution
  - Design question: How many clusters? How tight? How to distribute planets within clusters?
  - Notes: Should create distinct clusters of planets
  - TODO: Rework min planets/homeworld assignment strategy for cluster shape
  - Suggested approach: Generate cluster centers, then place planets around each center

**Implementation Considerations:**
- All must respect minimum distance constraints between planets
- All must use DeterministicRNG for reproducible results
- All must handle placement failures gracefully
- All should ensure suitable home planets are available (especially spiral and cluster)
- Consider using Poisson disk sampling for uniform distribution within shapes

**Design Questions to Answer:**
1. **Spiral:** How many arms? Logarithmic or Archimedean spiral? How tightly wound?
2. **Circle:** Uniform throughout? Density gradient? Hollow or filled?
3. **Ring:** Inner and outer radius? Width of ring? Density variation?
4. **Cluster:** Number of clusters? Cluster size? Overlap between clusters?
5. **All:** Should distribution ensure suitable home planets are spread across the shape?

---

### Apparent Gravity/Temperature Perception Conversion

**Status:** ✅ IMPLEMENTED  
**Location:** `game_formulas.cpp`

- `calculate_apparent_gravity()` - Convert true gravity to perceived gravity
  - Depends on: ideal_gravity, true_gravity
  - Notes: Used for planet desirability calculations
  - Current: ✅ IMPLEMENTED - Linear formula: perceived = (best_perceived_gravity / ideal_gravity) * true_gravity
  - Formula: Both measurements agree at zero gravity (0g → 0g); perceived gravity is monotonic with both true_gravity and ideal_gravity
  
- `calculate_apparent_temperature()` - Convert true temperature to perceived temperature
  - Depends on: ideal_temperature, true_temperature
  - Notes: Used for planet desirability calculations
  - Current: ✅ IMPLEMENTED - Linear formula: perceived = (best_perceived_temp / ideal_temp) * true_temp
  - Formula: Both measurements agree on absolute zero (0K); perceived temperature is monotonic with both true_temperature and ideal_temperature

**Implementation Considerations:**
- These functions determine how "suitable" a planet appears to a player
- The conversion formula affects planet desirability and player strategy
- Effects on actual game mechanics (population growth, income) are handled separately
- Consider if perception should be:
  - Linear (e.g., penalty proportional to mismatch)
  - Non-linear (e.g., exponential penalty for large mismatches)
  - Threshold-based (e.g., acceptable range with sharp penalty outside)
  - Asymmetric (e.g., different penalties for too hot vs. too cold)

**Design Questions to Answer:**
1. Should perception be linear or non-linear?
2. Should there be an "acceptable range" where planets are perceived as ideal?
3. How severe should the penalty be for mismatches?
4. Should the formula be the same for gravity and temperature?
5. Should perception be affected by player research level?

---

### Turn Processing

**Status:** Partially implemented  
**Location:** `game.cpp` - `process_turn()` method

**Completed Steps:**
- ✅ Calculate income for each player (planetary, interest, windfall)
- ✅ Apply spending allocations (savings, research, planet development)
- ✅ Process terraforming on each planet
- ✅ Process mining on each planet
- ✅ Calculate population growth for each planet

**Missing Steps:**
1. Process research point accumulation and tech advancement
2. Handle fleet movement and combat
3. Handle special events (nova, etc.)
4. Update player public info snapshots
5. Determine if any player has won

**Implementation Notes:**
- This is the core game loop; order matters
- Income should be calculated before spending allocations
- Population growth affects next turn's income
- Tech advancement should check if research points are sufficient

---

### Fleet Movement & Combat

**Status:** ✅ IMPLEMENTED (fleet movement complete, combat stub)  
**Location:** `player.cpp` - `move_fleet()` method

**Completed:**
- ✅ Calculate travel time based on fleet speed and distance
- ✅ Determine if fleet arrives this turn or next
- ✅ Handle fleet arrival at destination
- ✅ Update fleet state (in_transit, current_planet, destination)
- ✅ FleetTransit architecture for centralized movement state
- ✅ space_planet pattern for in-transit fleet isolation

**Missing Logic:**
- Implement combat resolution if fleets meet
- Handle fleet interception mechanics
- Add waypoint support for multi-leg journeys
- Implement fleet fuel consumption during movement
- Implement fleet visibility during transit

**Implementation Notes:**
- Fleet speed depends on tech_speed level
- Distance calculation uses Euclidean distance
- Combat only occurs if fleets from different players meet
- Need to track partial movement (fleet in transit between turns)

---

## Partially Implemented

Functions that exist but need completion or refinement.

---

### Ship Design Creation

**Status:** Method exists but cost calculation stubbed  
**Location:** `player.cpp` - `create_ship_design()`

**What's Done:**
- Design name validation
- Tech level validation
- Design storage

**What's Missing:**
- Calculate prototype cost (depends on ship_design_prototype_cost formula)
- Validate player has enough money for prototype cost
- Deduct money from player savings
- Possibly: validate metal cost for first ship

**Implementation Notes:**
- Prototype cost is one-time; subsequent ships use build cost
- May need to handle partial design creation if insufficient funds

---

### Fleet Creation

**Status:** Method exists but cost calculation stubbed  
**Location:** `player.cpp` - `create_fleet()`

**What's Done:**
- Fleet ID allocation
- Fleet storage
- Basic validation

**What's Missing:**
- Calculate build cost for requested ship count (uses ship_design_build_cost formula)
- Calculate metal cost for requested ship count (uses ship_design_metal_cost formula)
- Validate player has both money and metal
- Deduct resources from player
- Handle partial fleet creation if insufficient resources

**Implementation Notes:**
- Both money and metal must be available
- Build cost is per-ship; metal cost is per-ship
- May need to handle case where player can only build partial fleet

---

## Infrastructure/Setup

Functions needed for game flow but not core mechanics.

### Turn Ready Tracking

**Status:** Stub  
**Location:** `gamestate_c_api_extensions.cpp`

**Missing:**
- `game_mark_player_turn_ready(player_id)` - Mark player as done with turn
- `game_all_players_ready()` - Check if all players have finished turn
- Internal tracking of which players are ready

**Purpose:** Multiplayer turn coordination

**Implementation Notes:**
- Need to track ready state for each player
- Reset ready flags at start of each turn
- Used by UI to know when to call process_turn()

---

### Player Initial Setup

**Status:** Partially implemented (players created, but resources not initialized)  
**Location:** `game.cpp` - `initialize_players()` and `start_first_turn()` methods

**Completed:**
- ✅ Create player objects with ID, name, gender, type
- ✅ Assign ideal_temperature from truncated Gaussian distribution
- ✅ Placeholder for ideal_gravity (set to 0.0, assigned later in assign_planets_random)
- ✅ Assign gender-appropriate names to computer players from TextAssets
- ✅ Call capture_and_distribute_player_public_info() in start_first_turn()

**What's Missing:**
- Initialize player money_savings with starting colony income value
- Initialize player metal_reserve with starting colony metal value
- Initialize player colonized_planets with homeworld (after planet assignment)
- Initialize player technology levels (all start at 1)
- Initialize player research allocation fractions
- Initialize player money allocation fractions
- Initialize KnowledgeGalaxy for each player
- Populate homeworld with starting population and metal
- Set up initial income calculation for first turn

**Implementation Notes:**
- Called after galaxy generation and planet assignment
- Uses starting colony values from game_constants.h (indexed by StartingColonyQuality)
- Should set up player for first turn with proper initial state
- Currently only creates player objects; resource initialization is incomplete

---

## Summary Table

| Category | Count | Status | Priority |
|----------|-------|--------|----------|
| Data-Driven Values | 5 | Placeholder/Missing | High |
| Non-Data-Driven Functions | 5 | Stub | High |
| Partially Implemented | 3 | Incomplete | Medium |
| Infrastructure | 1 | Stub | Medium |
| **Implemented** | 6 | ✅ Complete | - |
| **Total** | **20** | - | - |

---

## Recommended Implementation Order

### Phase 1 (Foundation - No Reverse-Engineering)
1. Turn ready tracking (multiplayer coordination)
2. Galaxy shape distribution algorithms (design the placement strategies)
3. Fleet combat mechanics (design the combat system)
4. Nova warning duration (determine appropriate distribution)

### Phase 2 (Core Mechanics - Some Reverse-Engineering)
1. Population growth with mismatch penalties (data-driven)
2. Planetary income calculation (data-driven)
3. Planet desirability rating (somewhat data-driven)
4. Research conversion rates (data-driven)
5. Fleet interception and combat

### Phase 3 (Balance - Heavy Reverse-Engineering)
1. Ship design costs (build, prototype, metal) - data-driven
2. Technology advancement costs (verify quadratic formula) - data-driven
3. Starting colony values (verify progression) - data-driven

### Phase 4 (Metrics & Polish)
1. Player metrics (fleet power, victory points) - logic-based
2. Windfall income events - logic-based
3. Knowledge/fog of war system - logic-based

---

**Notes for Next Session:**
- Consider creating a "Game Balance" document with reverse-engineered values as you discover them
- May want to set up testing harness to verify formulas
- Consider creating UI for testing different balance values
- Document any assumptions made during reverse-engineering
- **Key Design Questions to Answer:**
- **Population growth:** How severe are mismatch penalties? Linear or exponential?
- **Planetary income:** What's the base formula? How do conditions affect it?
- **Galaxy shapes:** How to distribute planets for each shape? Poisson disk sampling?
- **Spiral galaxy:** How many arms? How tightly wound? How to ensure suitable homes?
- **Circle galaxy:** Uniform or density gradient? Hollow or filled?
- **Ring galaxy:** Inner/outer radius? Ring width? Density variation?
- **Cluster galaxy:** Number of clusters? Size? Overlap? How to ensure suitable homes?
- **Planet desirability:** What factors matter most? How are they weighted? Numeric or descriptive rating?
- **Desirability factors:** How much weight for gravity vs. temperature? Metal availability? Distance? Tech level effects?
