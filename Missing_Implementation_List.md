# Missing/Stubbed Implementation List

**Purpose:** Track all functions and values that need implementation or reverse-engineering from the original game.

**Last Updated:** January 3, 2026 (Session 5)

---

## Table of Contents

1. [Data-Driven Values (Require Reverse-Engineering)](#data-driven-values)
2. [Non-Data-Driven Functions (Clear Logic, Need Implementation)](#non-data-driven-functions)
3. [Partially Implemented (Need Completion)](#partially-implemented)
4. [Infrastructure/Setup (Needed for Game Flow)](#infrastructure-setup)

---

## Data-Driven Values

These require playing through the original Spaceward Ho! and reverse-engineering the formulas/values used.

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

**Status:** Placeholder (1% per turn)  
**Location:** `game_formulas.cpp`

- `calculate_population_growth()` - Population change per turn
  - Depends on: current_population, planet_temperature, planet_gravity, ideal_temperature, ideal_gravity
  - Notes: Critical for economy; affects income generation
  - Current: Simple 1% growth regardless of conditions
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

**Status:** Placeholder (1 money = 0.01 temperature change)  
**Location:** `game_formulas.cpp`

- `calculate_temperature_change()` - Temperature adjustment from spending
  - Depends on: money_spent, current_temperature, target_temperature
  - Notes: Affects how quickly planets can be terraformed
  - Current: Linear conversion with overshoot prevention
  - **Key Design Question:** Does efficiency change based on distance from target? Diminishing returns?

**Reverse-Engineering Approach:**
- Allocate money to terraforming on a planet
- Observe temperature change per turn
- Test with different starting temperatures and distances from target
- Determine if efficiency is constant or changes with distance
- Check for diminishing returns as you approach target
- Test with extreme temperatures

---

### Mining Efficiency

**Status:** Placeholder (1 money = 1 metal)  
**Location:** `game_formulas.cpp`

- `calculate_metal_mined()` - Metal extracted from planet per turn
  - Depends on: money_spent, metal_remaining
  - Notes: Affects resource accumulation
  - Current: 1:1 conversion with cap at remaining metal
  - **Key Design Question:** Does efficiency decrease as metal depletes? Constant rate or diminishing returns?

**Reverse-Engineering Approach:**
- Allocate money to mining on a planet with known metal
- Observe metal extraction rate
- Test with different metal amounts and depletion levels
- Determine if efficiency is constant or decreases as metal depletes
- Check for maximum extraction rate per turn
- Test mining a planet to depletion

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

**Status:** Placeholder values with reasonable progression  
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

### Interest Rates

**Status:** Implemented with placeholder values  
**Location:** `game_constants.h`

- `Money_Interest_Rate_Positive` = 0.15 (15% per turn on savings)
- `Money_Interest_Rate_Debt` = 0.30 (30% per turn on debt)

**Notes:**
- These seem high but may be intentional
- Affects strategy for money management
- Should verify against original
- **Key Design Question:** Are these rates per turn or per year? What's the intended balance?

**Reverse-Engineering Approach:**
- Save money and observe interest accumulation over several turns
- Go into debt and observe interest cost
- Verify rates match original game
- Determine if rates scale with game difficulty

---

### Technology Advancement Costs

**Status:** Implemented with quadratic formula  
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

---

### Apparent Gravity/Temperature Perception

**Status:** Placeholder (returns true value)  
**Location:** `game_formulas.cpp`

- `calculate_apparent_gravity()` - How player perceives planet's gravity
  - Depends on: ideal_gravity, true_gravity
  - Notes: Used for planet desirability calculations
  - Current: Returns true_gravity unchanged
  - **Key Design Question:** How do we convert from true gravity to perceived gravity for a given player? Linear, or something else?
  
- `calculate_apparent_temperature()` - How player perceives planet's temperature
  - Depends on: ideal_temperature, true_temperature
  - Notes: Used for planet desirability calculations
  - Current: Returns true_temperature unchanged
  - **Key Design Question:** How do we convert from true temperature to perceived temperature for a given player? Linear, or something else?

**Implementation Notes:**
- These functions determine how "suitable" a planet appears to a player
- The conversion formula affects planet desirability and strategy
- Effects on actual game mechanics (population growth, income) are handled separately in those formulas
- Consider if perception should be linear (e.g., penalty proportional to mismatch) or non-linear

---

### Turn Processing

**Status:** Stub exists but incomplete  
**Location:** `game.cpp` - `process_turn()` method

**Missing Steps:**
1. Calculate income for each player (planetary, interest, windfall)
2. Apply spending allocations (savings, research, planet development)
3. Process research point accumulation and tech advancement
4. Process terraforming on each planet
5. Process mining on each planet
6. Calculate population growth for each planet
7. Handle fleet movement and combat
8. Handle special events (nova, etc.)
9. Update player public info snapshots
10. Determine if any player has won

**Implementation Notes:**
- This is the core game loop; order matters
- Income should be calculated before spending allocations
- Population growth affects next turn's income
- Tech advancement should check if research points are sufficient

---

### Fleet Movement & Combat

**Status:** Stub exists but incomplete  
**Location:** `player.cpp` - `move_fleet()` method

**Missing Logic:**
- Calculate travel time based on fleet speed and distance
- Determine if fleet arrives this turn or next
- Handle fleet arrival at destination
- Implement combat resolution if fleets meet
- Update fleet state (in_transit, current_planet, destination)

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

**Status:** Stub  
**Location:** `game.cpp` - `initialize_galaxy()` comment

**Missing:**
- Populate each player with initial state based on player_setups
- Set initial money, metal, tech levels
- Possibly: set initial ship designs or fleets

**Implementation Notes:**
- Called after galaxy generation and planet assignment
- Uses starting colony values from game_constants.h
- Sets up player for first turn

---

## Summary Table

| Category | Count | Status | Priority |
|----------|-------|--------|----------|
| Data-Driven Values | 9 | Placeholder | High |
| Non-Data-Driven Functions | 4 | Stub | High |
| Partially Implemented | 2 | Incomplete | Medium |
| Infrastructure | 2 | Stub | Medium |
| **Total** | **17** | - | - |

---

## Recommended Implementation Order

### Phase 1 (Foundation - No Reverse-Engineering)
1. Turn ready tracking (multiplayer coordination)
2. Player initial setup (populate with starting values)
3. Apparent gravity/temperature perception (design the conversion formula)

### Phase 2 (Core Mechanics - Some Reverse-Engineering)
1. Population growth with mismatch penalties (data-driven)
2. Planetary income calculation (data-driven)
3. Terraforming efficiency (data-driven)
4. Mining efficiency (data-driven)
5. Fleet movement and combat basics

### Phase 3 (Balance - Heavy Reverse-Engineering)
1. Ship design costs (build, prototype, metal) - data-driven
2. Research conversion rates - data-driven
3. Technology advancement costs (verify quadratic formula) - data-driven
4. Starting colony values (verify progression) - data-driven
5. Interest rates (verify against original) - data-driven

### Phase 4 (Metrics & Polish)
1. Player metrics (fleet power, victory points) - logic-based
2. Nova warning duration - logic-based
3. Windfall income events - logic-based

---

**Notes for Next Session:**
- Consider creating a "Game Balance" document with reverse-engineered values as you discover them
- May want to set up testing harness to verify formulas
- Consider creating UI for testing different balance values
- Document any assumptions made during reverse-engineering
- Key design questions to answer:
  - Apparent gravity/temperature: Linear conversion or something else?
  - Population growth: How severe are mismatch penalties?
  - Planetary income: What's the base formula?
  - Terraforming/mining: Constant efficiency or diminishing returns?

