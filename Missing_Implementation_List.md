# Missing/Stubbed Implementation List

**Purpose:** Track all functions and values that need implementation or reverse-engineering from the original game.

**Last Updated:** January 27, 2026 (Session 15 - Spiral Galaxy Implementation Complete)

---

## Table of Contents & Implementation Checklist

### Economic Mechanics (Implemented) - 6 items

- [x] [Interest Rates](#interest-rates) - IMPLEMENTED
  - [x] Debt interest (15% on negative savings)
  - [x] Savings interest (10.0 × sqrt(savings))
- [x] [Mining Efficiency](#mining-efficiency) - IMPLEMENTED
  - [x] Formula: metal = 20.0 × sqrt(money)
  - [x] Inverse: money = (metal / 20)²
  - [x] Refund logic for insufficient metal
- [x] [Terraforming Efficiency](#terraforming-efficiency) - IMPLEMENTED
  - [x] Formula: delta_temp = 0.0811 × sqrt(money) in Fahrenheit
  - [x] Inverse: money = (delta_temp / 0.0811)²
  - [x] Overshoot prevention at ideal temperature
  - [x] Refund logic for excess allocation
- [x] [Income Allocation System](#income-allocation-system) - IMPLEMENTED
  - [x] Fraction-based distribution (savings, research, planets)
  - [x] Fixed double-counting bug
  - [x] Proper money flow from income to allocations
- [x] [Apparent Gravity/Temperature Perception](#apparent-gravitytemperature-perception) - IMPLEMENTED
  - [x] Linear gravity formula
  - [x] Linear temperature formula
- [x] [Temperature Unit Conversions](#temperature-unit-conversions) - IMPLEMENTED
  - [x] OFFSET_K_TO_C (273.15)
  - [x] OFFSET_C_TO_F (32.0)
  - [x] OFFSET_K_TO_F (459.67)
  - [x] Absolute temperature conversions
  - [x] Temperature change conversions

### Data-Driven Values (Require Reverse-Engineering) - 5 items

- [ ] [Planet Desirability Rating](#planet-desirability-rating) - Placeholder
- [ ] [Planetary Income Calculation](#planetary-income-calculation) - Placeholder
- [ ] [Ship Design Costs](#ship-design-costs) - Placeholder (3 functions)
- [ ] [Player Metrics](#player-metrics) - Placeholder (2 functions)
- [ ] [Population Growth](#population-growth) - Placeholder (ignores conditions)

### Non-Data-Driven Functions (Clear Logic, Need Implementation) - 5 items

- [x] [Galaxy Shape Distribution Algorithms](#galaxy-shape-distribution-algorithms) - PARTIALLY IMPLEMENTED (3 of 4 shapes)
  - [x] Circle galaxy distribution - IMPLEMENTED
  - [x] Ring galaxy distribution - IMPLEMENTED
  - [x] Spiral galaxy distribution - IMPLEMENTED
  - [ ] Cluster galaxy distribution
- [ ] [Turn Processing](#turn-processing) - Partially implemented (5 missing steps)
- [ ] [Fleet Movement & Combat](#fleet-movement--combat) - Partially implemented (combat stub)
- [ ] [Nova Warning Duration](#nova-warning-duration) - Placeholder
- [ ] [Research Conversion](#research-conversion) - Placeholder (1:1 conversion)

### Partially Implemented (Need Completion) - 3 items

- [ ] [Ship Design Creation](#ship-design-creation) - Incomplete (3 missing steps)
- [ ] [Fleet Creation](#fleet-creation) - Incomplete (3 missing steps)
- [ ] [Player Initial Setup](#player-initial-setup) - Incomplete (9 missing steps)

### Infrastructure/Setup (Needed for Game Flow) - 1 item

- [ ] [Turn Ready Tracking](#turn-ready-tracking) - Stub (2 functions)

### Summary

**Total Items:** 20 functions/systems  
**Implemented:** 13 (65%)  
**Partially Implemented:** 4 (20%)  
**Placeholder/Stub:** 3 (15%)

---

## Data-Driven Values

These require playing through the original Spaceward Ho! and reverse-engineering the formulas/values used.

### Planet Desirability Rating

**Status:** Placeholder  
**Location:** `game_formulas.cpp`

**Current Implementation:**
- Returns 0 (stub)

**Reverse-Engineering Approach:**
- Observe which planets players prefer to colonize
- Test planets with various gravity/temperature combinations
- Determine weighting factors for gravity vs. temperature
- Check if metal deposits affect desirability
- Test extreme values to find breaking points

---

### Planetary Income Calculation

**Status:** Placeholder  
**Location:** `game_formulas.cpp`

**Current Implementation:**
- Returns 0 (stub)

**Reverse-Engineering Approach:**
- Create colonies with known population and resources
- Observe income generated per turn
- Test with various planet conditions (gravity, temperature)
- Determine if income scales linearly or non-linearly with population
- Check if planet conditions affect income multiplier
- Test with different technology levels

---

### Ship Design Costs

**Status:** Placeholder (3 functions)  
**Location:** `game_formulas.cpp`

**Functions Needed:**
1. `calculate_ship_design_cost()` - Cost to design a new ship type
2. `calculate_ship_build_cost()` - Cost to build one ship of a design
3. `calculate_ship_upgrade_cost()` - Cost to upgrade a ship design

**Reverse-Engineering Approach:**
- Create new ship designs and observe costs
- Build ships and track resource consumption
- Upgrade designs and record cost changes
- Determine if costs scale with ship capabilities
- Test extreme ship configurations

---

### Player Metrics

**Status:** Placeholder (2 functions)  
**Location:** `game_formulas.cpp`

**Functions Needed:**
1. `calculate_player_score()` - Overall player ranking
2. `calculate_player_victory_points()` - Victory condition tracking

**Reverse-Engineering Approach:**
- Play through a game and track score changes
- Observe what actions increase/decrease score
- Determine weighting of different achievements
- Check if score affects game balance or AI behavior

---

### Population Growth

**Status:** Placeholder (ignores planet conditions)  
**Location:** `game_formulas.cpp`

**Current Implementation:**
- Returns 10% growth regardless of gravity/temperature mismatch
- Accepts parameters but doesn't use them

**Reverse-Engineering Approach:**
- Create planets with various gravity/temperature mismatches
- Observe population growth rates
- Test with ideal conditions vs. poor conditions
- Determine penalty formula for mismatch (linear? exponential? threshold?)
- Check if there's a minimum/maximum growth rate
- Test extreme mismatches to find breaking points

---

## Non-Data-Driven Functions

These have clear logic but need implementation. No reverse-engineering needed.

### Galaxy Shape Distribution Algorithms

**Status:** PARTIALLY IMPLEMENTED (Circle, Ring, and Spiral complete; Cluster pending)  
**Location:** `galaxy.cpp`, `utility.h/cpp`

**Completed (Session 14):**

- [x] `generate_coordinates_circle()` - Circular disk galaxy distribution - IMPLEMENTED
  - Uses Bridson's Poisson disk sampling for uniform distribution
  - Radius calculated from active area: sqrt(active_area / pi)
  - Active area = gal_size^2 where gal_size = sqrt(n_planets) * (5.0 + 6.4/density)
  - Maintains same planet density as random galaxy
  - Home planets randomly distributed among all planets

- [x] `generate_coordinates_ring()` - Ring/torus galaxy distribution - IMPLEMENTED
  - Uses Bridson's Poisson disk sampling within annular region
  - Outer radius randomly selected from calculated range
  - Inner radius calculated to maintain correct active area
  - Supports inner/outer ratio constraints (50%-80%)
  - Maintains same active area as circle galaxy for consistent density

**Completed (Session 15):**

- [x] `generate_coordinates_spiral()` - Fermat spiral arm galaxy distribution - IMPLEMENTED
  - Uses Fermat spiral (r = a√θ) instead of logarithmic for better radial tangent
  - N spiral arms (N = number of players), evenly spaced angularly
  - Randomized parameters:
    - Angular span: [π/4, π]
    - Spiral tightness: a = 100.0 / √(Δθ)
    - Outer/core ratio: [2.0, 6.0]
  - Arc length calculation using Pythagorean approximation
  - Central core with Poisson sampling, avoiding arm coordinates
  - Overlap between arms and core: max(6.0, 0.15 × core_radius)
  - Home planet selection: one per arm at calculated arm end
  - Implemented `select_home_planets_spiral()` function
  - Modified Poisson disk sampling to accept existing coordinates

**Remaining (Next Session):**

- [ ] `generate_coordinates_cluster()` - Clustered galaxy distribution
  - Design: N clusters (N = number of players) arranged in ring around center
  - Clusters similarly sized, each with player's home planet
  - TODO: Implement home planet selection for cluster (one per cluster)
  - TODO: Rework min planets/homeworld assignment strategy for cluster shape

**Implementation Infrastructure:**
- Region abstract base class with virtual methods (utility.h)
- CircleRegion and RingRegion implementations
- Bridson's Poisson disk sampling algorithm (O(N) complexity)
- Spatial grid acceleration for efficient neighbor checking
- DeterministicRNG for reproducible results
- Failure logging for debugging insufficient planet generation
- Spiral helper functions: fermat_spiral_arc_length(), fermat_spiral_point()

**Design Decisions:**
- Active area approach ensures consistent planet density across shapes
- Poisson sampling guarantees minimum distance between planets
- Ring galaxies support randomized inner/outer radii within small range
- Spiral galaxies use Fermat spiral for better geometric properties
- Home planet selection is shape-specific (random for circle/ring, arm-end for spiral)

**Next Steps for Cluster:**
1. Implement cluster generation with ring arrangement
2. Implement shape-specific home planet selection for clusters
3. Handle gravity mismatch for START_OUTPOST quality colonies
4. Implement error handling when insufficient suitable planets

---

### Apparent Gravity/Temperature Perception Conversion

**Status:** IMPLEMENTED  
**Location:** `game_formulas.cpp`

- `calculate_apparent_gravity()` - Convert true gravity to perceived gravity
  - Formula: `apparent = 100 * true_gravity` (linear scaling)
  - Returns value as percentage (100 = Earth normal)

- `calculate_apparent_temperature()` - Convert true temperature to perceived temperature
  - Formula: `apparent = true_temperature - ideal_temperature` (linear offset)
  - Returns difference in Kelvin
  - Negative values = too cold, Positive = too hot

---

### Turn Processing

**Status:** Partially implemented (5 missing steps)  
**Location:** `game.cpp`

**Completed:**
- [x] Income calculation and allocation
- [x] Mining efficiency processing
- [x] Terraforming efficiency processing
- [x] Population growth (placeholder)

**Missing:**
- [ ] Fleet movement and combat
- [ ] Research point accumulation and tech advancement
- [ ] Nova events and warnings
- [ ] Diplomacy/trade events
- [ ] End-of-turn cleanup and state updates

---

### Fleet Movement & Combat

**Status:** Partially implemented (combat stub)  
**Location:** `game.cpp`

**Completed:**
- [x] Fleet movement calculation (distance-based)
- [x] Fleet arrival detection

**Missing:**
- [ ] Combat resolution (currently returns stub)
- [ ] Damage calculation
- [ ] Fleet retreat logic
- [ ] Casualty handling
- [ ] Victory/defeat determination

---

### Nova Warning Duration

**Status:** Placeholder  
**Location:** `game_formulas.cpp`

**Current Implementation:**
- Returns 0 (stub)

**Expected Behavior:**
- Determine how many turns a nova warning is visible before the nova occurs
- Likely based on distance from nova star or other factors

---

### Research Conversion

**Status:** Placeholder (1:1 conversion)  
**Location:** `game_formulas.cpp`

**Current Implementation:**
- 1 research point = 1 technology level increase (1:1 ratio)

**Reverse-Engineering Approach:**
- Play through research in original game
- Observe how many research points needed for each tech level
- Determine if conversion rate changes with game progression
- Check if different tech trees have different costs

---

## Partially Implemented Functions

These have some implementation but are missing key steps.

### Ship Design Creation

**Status:** Incomplete (3 missing steps)  
**Location:** `game.cpp`

**Completed:**
- [x] Basic ShipDesign object creation
- [x] Design ID allocation

**Missing:**
- [ ] Validate design parameters (armor, weapons, engines, etc.)
- [ ] Calculate design cost
- [ ] Add design to player's available designs

---

### Fleet Creation

**Status:** Incomplete (3 missing steps)  
**Location:** `game.cpp`

**Completed:**
- [x] Basic Fleet object creation
- [x] Fleet ID allocation

**Missing:**
- [ ] Validate fleet parameters (location, design, quantity)
- [ ] Deduct resources from colony
- [ ] Add fleet to player's fleet list

---

### Player Initial Setup

**Status:** Incomplete (9 missing steps)  
**Location:** `game.cpp`

**Completed:**
- [x] Player object creation
- [x] Basic property initialization

**Missing:**
- [ ] Initialize money_savings and metal_reserve
- [ ] Initialize colonized_planets with homeworld
- [ ] Initialize all technology levels
- [ ] Initialize research and money allocation fractions
- [ ] Initialize KnowledgeGalaxy
- [ ] Set up initial income calculation
- [ ] Assign starting colony values based on quality
- [ ] Initialize fleet list
- [ ] Initialize ship design list

---

## Infrastructure/Setup

### Turn Ready Tracking

**Status:** Stub (2 functions)  
**Location:** `gamestate_c_api_extensions.cpp`

**Functions:**
1. `game_mark_player_turn_ready()` - Mark player as ready for next turn
2. `game_get_player_turn_ready_status()` - Check if player is ready

**Current Implementation:**
- Both functions are stubs (return without doing anything)

**Expected Behavior:**
- Track which players have completed their turn actions
- Allow game to advance to next turn when all players are ready
- Support for simultaneous turn processing

---

## Implementation Priority

Based on game flow dependencies:

1. **CRITICAL (Blocks Game Loop):**
   - Player Initial Setup (9 missing steps)
   - Turn Ready Tracking (2 functions)

2. **HIGH (Core Gameplay):**
   - Population Growth (needs proper formula)
   - Planetary Income Calculation (needed for economy)
   - Fleet Movement & Combat (core mechanic)

3. **MEDIUM (Enhances Gameplay):**
   - Ship Design Creation (3 missing steps)
   - Fleet Creation (3 missing steps)
   - Research Conversion (needs proper formula)

4. **LOW (Polish/Balance):**
   - Planet Desirability Rating (AI decision-making)
   - Player Metrics (scoring/rankings)
   - Nova Warning Duration (event system)
   - Ship Design Costs (economy balance)
   - Cluster Galaxy (shape variety)

