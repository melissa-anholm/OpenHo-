# Missing/Stubbed Implementation List

**Purpose:** Track all functions and values that need implementation or reverse-engineering from the original game.

**Last Updated:** October 27, 2026 (Targeted status audit after galaxy-generation refactor)

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
- [ ] [Planetary Income Calculation](#planetary-income-calculation) - Provisional main-path formula; standalone helper is a stub
- [ ] [Ship Design Costs](#ship-design-costs) - Placeholder (3 functions)
- [ ] [Player Metrics](#player-metrics) - Placeholder (2 functions)
- [ ] [Population Growth](#population-growth) - Placeholder (ignores conditions)

### Non-Data-Driven Functions (Clear Logic, Need Implementation) - 5 items

- [ ] [Galaxy Shape Distribution Algorithms](#galaxy-shape-distribution-algorithms) - Implemented but insufficiently validated (all 6 shapes)
  - [x] RANDOM galaxy distribution - Used by the minimal playable path
  - [x] Circle galaxy distribution - IMPLEMENTED
  - [x] Ring galaxy distribution - IMPLEMENTED
  - [x] Spiral galaxy distribution - IMPLEMENTED but experimental
  - [x] Cluster galaxy distribution - IMPLEMENTED but insufficiently validated
  - [x] GRID galaxy distribution - IMPLEMENTED
- [ ] [Turn Processing](#turn-processing) - Partially implemented; research and basic processing exist, but several systems remain provisional or absent
- [ ] [Fleet Movement & Combat](#fleet-movement--combat) - Movement exists but needs validation; combat is absent
- [ ] [Nova Warning Duration](#nova-warning-duration) - Placeholder
- [ ] [Research Conversion](#research-conversion) - Placeholder (1:1 conversion)

### Partially Implemented (Need Completion) - 3 items

- [ ] [Ship Design Creation](#ship-design-creation) - Object creation and storage exist; costs and validation consistency remain
- [ ] [Fleet Creation](#fleet-creation) - Creation and validation exist; resource deduction remains
- [ ] [Player Initial Setup](#player-initial-setup) - Mostly implemented for the minimal game; explicit initial-state decisions and tests remain

### Infrastructure/Setup (Needed for Game Flow) - 1 item

- [ ] [Turn Ready Tracking](#turn-ready-tracking) - Implemented, but not yet enforced as a turn-advance gate

### Summary

The categories below overlap: a subsystem can be implemented in the current code while still using provisional formulas or lacking validation. Percentages are intentionally not reported here because they would obscure that distinction.

---

## Data-Driven Values

These require playing through the original Spaceward Ho! and reverse-engineering the formulas/values used.

### Planet Desirability Rating

**Status:** Placeholder
**Location:** `src/core/src/colonized_planet.cpp` and related game logic

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

**Status:** Provisional main-path formula; standalone helper remains a stub
**Location:** `src/core/src/game.cpp` and `src/core/src/game_formulas.cpp`

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
**Location:** `src/core/src/game_formulas.cpp`

**Functions Needed:**
1. `calculate_ship_design_prototype_cost()` - Cost to prototype a new ship type
2. `calculate_ship_design_build_cost()` - Cost to build one ship of a design
3. `calculate_ship_design_metal_cost()` - Metal cost associated with a ship design

**Reverse-Engineering Approach:**
- Create new ship designs and observe costs
- Build ships and track resource consumption
- Upgrade designs and record cost changes
- Determine if costs scale with ship capabilities
- Test extreme ship configurations

---

### Player Metrics

**Status:** Placeholder (2 functions)
**Location:** `src/core/src/game_formulas.cpp`

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
**Location:** `src/core/src/game_formulas.cpp`

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

**Status:** Implemented but insufficiently validated (all 6 shapes)
**Location:** `src/core/src/galaxy*.cpp`, `src/core/src/galaxygen_*.cpp`, and `src/core/src/galaxy_home_selection.cpp`

**Implemented shape generators:**

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

- [x] `generate_coordinates_cluster()` - Clustered galaxy distribution - IMPLEMENTED
  - N clusters (N = number of players) arranged in perfect ring around center
  - Random angular offset (0-360 degrees) for varied cluster orientations
  - Clusters similarly sized with random planet placement within cluster circles
  - Density-based spacing: 10% overlap at high density, room for another cluster at low density
  - Simple random sampling within cluster circles (not Poisson)
  - Attempt limit: 10x target planets per cluster
  - Implemented `select_home_planets_cluster()` function
  - Wedge-based home planet selection: galaxy divided into N equal angular wedges
  - One random planet selected from each wedge as home planet
  - Angular offset is carried through `GalaxyGenerationParams` for later home-planet selection

- [x] `generate_coordinates_random()` - Random galaxy distribution - IMPLEMENTED and used by the minimal playable path
- [x] `generate_coordinates_grid()` - Grid galaxy distribution - IMPLEMENTED

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
- Poisson sampling can guarantee minimum distance between planets when used with an appropriate minimum-distance constraint
- Ring galaxies support randomized inner/outer radii within small range
- Spiral galaxies use Fermat spiral for better geometric properties
- Home planet selection is shape-specific (random for circle/ring, arm-end for spiral)

**IMPORTANT: Extensive Testing Required**

All six galaxy shapes now have coordinate generation implemented. However, comprehensive testing is needed to verify:

- Planet distribution correctness for all shapes
- Spatial constraint validation (minimum distances)
- Density consistency across shapes and parameters
- Edge cases (extreme planet counts and density values)
- Home planet placement accuracy per shape
- Seed reproducibility
- Visual inspection for artifacts or anomalies

This testing should be performed before galaxy generation is considered production-ready.

---

### Apparent Gravity/Temperature Perception Conversion

**Status:** IMPLEMENTED
**Location:** `src/core/src/game_formulas.cpp`

- `calculate_apparent_gravity()` - Convert true gravity to perceived gravity
  - Formula: `apparent = 100 * true_gravity` (linear scaling)
  - Returns value as percentage (100 = Earth normal)

- `calculate_apparent_temperature()` - Convert true temperature to perceived temperature
  - Formula: `apparent = true_temperature - ideal_temperature` (linear offset)
  - Returns difference in Kelvin
  - Negative values = too cold, Positive = too hot

---

### Turn Processing

**Status:** Partially implemented; research and basic processing exist, but several systems remain provisional or absent
**Location:** `src/core/src/game.cpp` and `src/core/src/game_formulas.cpp`

**Completed:**
- [x] Income calculation and allocation
- [x] Mining efficiency processing
- [x] Terraforming efficiency processing
- [x] Population growth (placeholder)
- [x] Research-point accumulation and technology advancement (using provisional conversion and cost formulas)
- [x] End-of-turn counter/readiness reset behavior

**Missing:**
- [ ] Fleet combat
- [ ] Nova events and warnings
- [ ] Diplomacy/trade events
- [ ] Complete end-of-turn state/knowledge updates

---

### Fleet Movement & Combat

**Status:** Movement implemented but insufficiently validated; combat is absent
**Location:** `src/core/src/game.cpp`, `src/core/src/player.cpp`, and `src/core/src/fleet.cpp`

**Completed:**
- [x] Fleet movement calculation (distance-based)
- [x] Fleet arrival processing (requires validation of planet-ID to vector-index lookup)

**Missing:**
- [ ] Combat resolution (not implemented)
- [ ] Damage calculation
- [ ] Fleet retreat logic
- [ ] Casualty handling
- [ ] Victory/defeat determination

---

### Nova Warning Duration

**Status:** Placeholder
**Location:** `src/core/src/game_formulas.cpp`

**Current Implementation:**
- Returns 1 as a placeholder; the warning-duration rule is not implemented

**Expected Behavior:**
- Determine how many turns a nova warning is visible before the nova occurs
- Likely based on distance from nova star or other factors

---

### Research Conversion

**Status:** Placeholder (1:1 conversion)
**Location:** `src/core/src/game_formulas.cpp`

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

**Status:** Object creation and storage exist; costs and validation consistency remain
**Location:** `src/core/src/player.cpp`, `src/core/src/game.cpp`, and `src/core/src/gamestate_c_api_extensions.cpp`

**Completed:**
- [x] Basic ShipDesign object creation
- [x] Design ID allocation
- [x] Add design to the player's available design list

**Missing:**
- [ ] Ensure all direct creation paths apply the same design validation
- [ ] Calculate and charge design/prototype costs

---

### Fleet Creation

**Status:** Creation and validation exist; resource deduction remains
**Location:** `src/core/src/player.cpp`, `src/core/src/game.cpp`, and `src/core/src/gamestate_c_api_extensions.cpp`

**Completed:**
- [x] Basic Fleet object creation
- [x] Fleet ID allocation
- [x] Validate fleet parameters through the GameState/API path
- [x] Add fleet to the player's fleet list

**Missing:**
- [ ] Ensure direct and API creation paths apply consistent validation
- [ ] Deduct money/metal according to the intended fleet cost rules

---

### Player Initial Setup

**Status:** Mostly implemented for the minimal game; starting fleets/designs and explicit initialization tests remain
**Location:** `src/core/src/game.cpp`, `src/core/src/game_setup.cpp`, and `src/core/src/game_constants.h`

**Completed:**
- [x] Player object creation
- [x] Basic property initialization
- [x] Initialize the homeworld and `colonized_planets`
- [x] Initialize `KnowledgeGalaxy`
- [x] Assign starting population, income, and metal based on colony quality
- [x] Initialize the default technology, research, and allocation state
- [x] Initialize empty fleet and ship-design lists

**Missing or requiring a decision:**
- [ ] Confirm and document intended initial savings and reserve values
- [ ] Decide whether starting fleets and ship designs should be created
- [ ] Add focused assertions covering every expected initial field

---

## Infrastructure/Setup

### Turn Ready Tracking

**Status:** Implemented, but not yet enforced as a turn-advance gate
**Location:** `src/core/src/game.cpp` and `src/core/src/gamestate_c_api_extensions.cpp`

**Functions:**
1. `game_mark_player_turn_ready()` - Mark player as ready for next turn
2. `game_get_player_turn_ready_status()` - Check if player is ready

**Current Implementation:**
- Readiness is stored per player, can be marked and queried, and is reset after turn processing.

**Expected Behavior:**
- Track which players have completed their turn actions
- Allow game to advance to next turn when all players are ready
- Support for simultaneous turn processing

---

## Implementation Priority

Based on game flow dependencies:

1. **CRITICAL (Minimal Playable Path):**
   - Validate and document player initial-state semantics
   - Resolve provisional income/population update behavior

2. **HIGH (Core Gameplay):**
   - Population Growth and Planetary Income (intended formulas and update ordering)
   - Placeholder ship construction with real state/resource effects
   - Fleet movement validation and exploration/knowledge updates

3. **MEDIUM (Expands Gameplay):**
   - Ship design and fleet resource costs
   - Research conversion and advancement formulas
   - Ship combat

4. **LOW (Polish/Balance):**
   - Planet Desirability Rating (AI decision-making)
   - Player Metrics (scoring/rankings)
   - Nova Warning Duration and nova events
   - Galaxy-shape validation and edge-case testing
