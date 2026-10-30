# Missing Implementations List

**Purpose:** Track all functions and values that need implementation or reverse-engineering from the original game.

**Last Updated:** October 28, 2026 (Status and priority audit)

---

> **Known turn-processing issue — fix later:** `calculate_player_incomes()` currently sums stored planetary income before `update_planet_incomes()` refreshes it, and interest is added after the current turn’s allocations. Reorder the steps later so planetary income is refreshed and summed, interest is calculated, and then total current income is allocated. The intended interest destination remains current income, not savings.

---

## Implementation Checklist


### Implemented; Requires Final Double-Check - 3 items

#### Temperature Unit Conversions
- **Status:** Implemented. Requires double-check.
- Absolute temperatures are stored in Kelvin
- Temperature-change formulas use Fahrenheit with explicit conversion helpers
- **Open Question:** Use true temperature conversion, or approximate conversion for a round "ideal metric temperature"?
- **Priority:** Low

#### Debt Interest
- **Status:** Implemented. Requires double-check, but it's probably ok.
- **Formula:** `interest = 0.15 × savings` for negative savings
- **Open Question:** Current application/order requires correction; see the turn-processing note above.
- **Priority:** Low

#### Apparent Gravity/Temperature Perception Conversion
- **Status:** Implemented. Requires double-check.
- **Location:** `src/core/src/game_formulas.cpp`
- **Functions:**
	1. `calculate_apparent_gravity()`: `apparent_gravity = (1.0 / ideal_gravity) × true_gravity`; gravity remains a normalized fraction, not a percentage.
	2. `calculate_apparent_temperature()`: `apparent_temperature = (295.37223 / ideal_temperature) × true_temperature`; all temperatures are in Kelvin.
- **Current Implementation:** Uses approximation for round ideal metric temperature; see temperature conversion item above.
- **Priority:** Low

---
### Provisional/Placeholder Formulas (Functional, but Likely Incorrect); Needs Data-Driven Update to Calculations - 3 items

#### Savings Interest
- **Status:** Provisional functionality.  No idea whether this formula is correct.
- **Formula:** `interest = 10.0 × sqrt(savings)` for positive savings.
- Current application/order requires correction; see the turn-processing note above
- **Priority:** Medium-low

#### Mining Efficiency
- **Status:** Provisional functionality.  No idea whether this formula is correct.
- **Formula:** `metal_extracted = 20.0 × sqrt(money_spent)`
- Inverse: `money = (metal / 20.0)²`
- **Resource flow:** Money from the current positive income allocation is spent during economic processing; extracted metal is transferred losslessly from the planet to the player's shared metal reserve. Planetary metal is clamped at zero.
- **Feature:** Depleted-metal refund logic is implemented; the formula and data-driven behavior still require validation.
- **Priority:** Medium-low

#### Terraforming Efficiency
- **Status:** Provisional functionality.  No idea if this is correct.
- **Formula:** `temperature_change_F = 0.0811 × sqrt(money_spent)`
- Inverse: `money = (temperature_change_F / 0.0811)²`
- **Resource flow:** Money from the current positive income allocation is spent during economic processing; terraforming does not consume metal. Temperature changes immediately during economic processing.
- **Feature:** Overshoot prevention and excess-allocation refunds are implemented. The current refund destination is `money_savings`, pending a future design decision.
- **Priority:** Medium-low


---

### Data-Driven Values (Require Reverse-Engineering) - 5 items

#### Negative Income Mechanics
- **Status:** Partially specified; implementation and bankruptcy threshold remain future work.
- **Location:** `src/core/src/game.cpp`
- **Current Implementation:**
  - Before **Dip Into Savings** is implemented, `effective_income = net_income`.
  - When effective income is positive, it is distributed according to the player's budget allocations.
  - When effective income is negative, no money is allocated to savings, planetary development, or technology research.
  - The negative effective income is subtracted directly from the player's money savings account, which may become negative.
  - A player may recover later if net income becomes positive; one negative-income turn does not immediately cause bankruptcy.
- **Missing:**
  - [ ] Determine the data-driven lower limit for negative savings.
  - [ ] Implement bankruptcy and loss when the negative-savings limit is reached.
- **Priority:** Medium-high

#### Planetary Income Calculation
- **Status:** Provisional main-path formula; standalone helper remains a stub
- **Location:** `src/core/src/game.cpp` and `src/core/src/game_formulas.cpp`
- **Current Implementation:**
  - The main turn path uses stored colony-income values and a provisional gravity/temperature condition factor.
  - `calculate_planetary_income()` returns 0 and is not wired into the main path.
  - Income is currently summed before colony income is refreshed for the new turn; see the turn-processing note at the top.
- **Priority:** Medium-high

#### Ship Design Costs
- **Status:** Placeholder (3 functions)
- **Location:** `src/core/src/game_formulas.cpp`
- **Confirmed rules:**
  - Building ships costs both money and metal, charged immediately.
  - The first ship of a given type also incurs a money-only prototyping cost, typically about 10x the individual ship's money construction cost; the exact ratio is data-driven.
  - An unaffordable action fails atomically and creates nothing.
  - Ships built during the current player-action phase may be scrapped during that same phase without losing their construction or prototyping resources. After the phase ends, scrapping recovers metal only and at a heavy loss.
- **Functions Needed:**
	1. `calculate_ship_design_prototype_cost()` - Cost to prototype a new ship type
	2. `calculate_ship_design_build_cost()` - Cost to build one ship of a design
	3. `calculate_ship_design_metal_cost()` - Metal cost associated with a ship design
- **Priority:** Medium

#### Population Growth
- **Status:** Placeholder (ignores planet conditions)
- **Location:** `src/core/src/game_formulas.cpp`
- **Current Implementation:**  Returns 10% growth regardless of gravity/temperature mismatch.  Accepts parameters but doesn't use them.
- **Priority:** Medium-high

#### Research Conversion
- **Status:** Provisional 1:1 money-to-research conversion; advancement costs remain provisional
- **Location:** `src/core/src/game_formulas.cpp`
- **Current Implementation:**
 - One unit of money allocated to research produces one research point.
 - Research points are consumed by separate technology-advancement cost formulas.
 - The original-game conversion and advancement rules remain unverified.
- **Priority:** Medium


---
### Non-Data-Driven Functions (Must Still Choose an Implementation) - 3 items

#### Planet Desirability Rating
- **Status:** Placeholder
- **Location:** `src/core/src/colonized_planet.cpp` and related game logic
- **Current Implementation:**  Returns 0 (stub)
- **Priority:** Low

#### Player Metrics
- **Status:** Placeholder (2 functions)
- **Location:** `src/core/src/game_formulas.cpp`
- **Functions Needed:**
	1. `calculate_player_fleet_power()` - Public fleet-strength metric
	2. `calculate_player_victory_points()` - Victory condition tracking
- **Priority:** Low

#### Nova Warning Duration
- **Status:** Placeholder
- **Location:** `src/core/src/game_formulas.cpp`
- **Current Implementation:** Returns 1 as a placeholder; the warning-duration rule is not implemented.
- **Expected Behavior:** Determine how many turns a nova warning is visible before the nova occurs; the rule may depend on distance from the nova star or other factors.
- **Priority:** Lowest

---
### Galaxy Generation

**Status:** Partially implemented but insufficiently validated (all shapes).

**Location:** `src/core/src/galaxy*.cpp`, `src/core/src/galaxygen_*.cpp`, and `src/core/src/galaxy_home_selection.cpp`

#### **Shape Generators:**

**Random:** `generate_coordinates_random()` - Random galaxy distribution.

- **Status:** - Implemented and used by the minimal playable game.  The density still comes out all wrong though (28 Oct 2026), as does the planet count.  Probably need to re-write the whole thing.
- **Priority:** Medium-high

**Grid:** `generate_coordinates_grid()` - Grid galaxy distribution.

- **Status:**  Implemented.  Probably needs small tweaks.
- **Priority:** Medium-low

**Circle:**  `generate_coordinates_circle()` - Circular disk galaxy distribution.

- **Status:**  Functional.  May or may not get the density correct.  (28 Oct. 2026)
- Uses Bridson's Poisson disk sampling for uniform distribution
- Radius calculated from active area: sqrt(active_area / pi)
- `Active area = gal_size^2` where `gal_size = sqrt(n_planets) * (5.0 + 6.4/density)`
- Maintains same planet density as random galaxy
- Home planets randomly distributed among all planets
- **Priority:** Medium-low

**Ring:** `generate_coordinates_ring()` - Ring/torus galaxy distribution

- **Status:**  Implemented.  Or at any case, it's as implemented as the circle galaxy generator algorithm (28 Oct 2026)
- Uses Bridson's Poisson disk sampling within annular region, just like circle galaxy.
- Outer radius randomly selected from calculated range
- Inner radius calculated to maintain correct active area
- Supports inner/outer ratio constraints (50%-80%)
- Maintains same active area as circle galaxy for consistent density.
- **Priority:** Medium-low

**Spiral:** `generate_coordinates_spiral()` - Fermat spiral arm galaxy distribution.

- **Status:** Implemented.  Maybe.  Who even knows?  It possibly still doesn't work correctly.
- Uses Fermat spiral `(r = a√θ)` instead of logarithmic for better radial tangent
- `N spiral arms` with `(N = number of players)`. Evenly spaced angularly.
- Randomized parameters:
- Angular span: `[π/4, π]`
- Spiral tightness: `a = 100.0 / √(Δθ)`
- Outer/core ratio: `[2.0, 6.0]`
- Arc length calculation using Pythagorean approximation
- Central core with Poisson sampling, avoiding arm coordinates
- Overlap between arms and core: `max(6.0, 0.15 × core_radius)`
- Home planet selection: one per arm at calculated arm end
- Implemented `select_home_planets_spiral()` function
- Modified Poisson disk sampling to accept existing coordinates
- **Priority:** Medium-low

**Cluster:** `generate_coordinates_cluster()` - Clustered galaxy distribution.

- **Status:** - Implemented.  Maybe.  Not sure if it actually works.
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
- **Priority:** Medium-low

**Hex:**  Like the grid, but it's a hex.

- **Status:** Not implemented at all.
- **Priority:** Medium-low

#### **Implementation Infrastructure:**
- Region abstract base class with virtual methods (utility.h)
- CircleRegion and RingRegion implementations
- Bridson's Poisson disk sampling algorithm (O(N) complexity)
- Spatial grid acceleration for efficient neighbor checking
- DeterministicRNG for reproducible results
- Failure logging for debugging insufficient planet generation
- Spiral helper functions: fermat_spiral_arc_length(), fermat_spiral_point()

#### **Design Decisions:**
- Active area approach ensures consistent planet density across shapes
- Poisson sampling can guarantee minimum distance between planets when used with an appropriate minimum-distance constraint
- Ring galaxies support randomized inner/outer radii within small range
- Spiral galaxies use Fermat spiral for better geometric properties
- Home planet selection is shape-specific: random for circle/ring, arm-end for spiral, and angular-wedge selection for cluster galaxies

**IMPORTANT: Extensive Testing Required**

Six galaxy shapes now have coordinate generation implemented. However, comprehensive testing is needed to verify:

- Planet distribution correctness for all shapes
- Spatial constraint validation (minimum distances)
- Density consistency across shapes and parameters
- Edge cases (extreme planet counts and density values)
- Home planet placement accuracy per shape
- Seed reproducibility
- Visual inspection for artifacts or anomalies

This testing should be performed before galaxy generation is considered production-ready.


---
### Logistical

#### Income Allocation System
- [-] Possibly implemented. Requires double-check.
  - [x] `savings = income × savings_fraction`
  - [x] `research = income × research_fraction`
  - [x] `planets = income × planets_fraction`
  - [x] Allocation fractions must sum to approximately `1.0`
  - [x] Allocation funds come from current income only
  - [ ] Current application/order may require correction; see the turn-processing note above.
- **Priority:** High

#### Turn Processing
- **Status:** Partially implemented; research and basic processing exist, but several systems remain provisional or absent
- **Location:** `src/core/src/game.cpp` and `src/core/src/game_formulas.cpp`
- **Completed:**
 - [x] Income calculation and allocation
 - [x] Mining efficiency processing
 - [x] Terraforming efficiency processing
 - [x] Population growth (placeholder)
 - [x] Research-point accumulation and technology advancement (using provisional conversion and cost formulas)
 - [x] End-of-turn counter/readiness reset behavior
- **Missing:**
 - [ ] Fleet combat
 - [ ] Nova events and warnings
 - [ ] Diplomacy/trade events (deferred; not required for core game flow)
 - [ ] Capture canonical Turn N snapshots at the beginning of each turn, with Turn 0/year 2000 initialization
 - [ ] Capture economic after-state and post-processing state for history, diagnostics, and display
 - [ ] Complete end-of-turn state/knowledge updates with public/private filtering from the same snapshot boundary
 - [ ] Serialize and restore the complete mid-game state, including pending actions, readiness, knowledge, and RNG state
 - [ ] Synchronize critical state at least once per turn for host migration
 - [ ] Enforce readiness in the core turn-processing entry point
 - [ ] Exclude eliminated players from readiness requirements
 - [ ] Require an explicit host decision after a player disconnects; transfer that responsibility on host migration
 - [ ] Allow post-ready actions to make a player unready, with final automatic-versus-explicit re-ready policy still to be chosen
 - [ ] Bound AI turn execution with a configurable time limit
- **Priority:** High

#### Fleet Movement
- **Status:** Movement implemented but insufficiently validated
- **Location:** `src/core/src/game.cpp`, `src/core/src/player.cpp`, and `src/core/src/fleet.cpp`
- **Completed:**
 - [x] Fleet movement calculation (distance-based)
 - [x] Fleet arrival processing (requires validation of planet-ID to vector-index lookup)
- **Priority:** Medium-high

#### Fleet and Planet Combat
- **Status:** Not implemented
- **Location:** `src/core/src/game.cpp`, `src/core/src/player.cpp`, and `src/core/src/fleet.cpp`
- **Missing:**
 - [ ] Resolve fleet travel before battle resolution at each destination planet
 - [ ] Resolve fleet-versus-fleet battles between non-allied players at the same planet
 - [ ] Resolve fleet-versus-planet battles when a fleet is stationed at a non-ally's planet
 - [ ] Combat resolution for battles involving three or more players
 - [ ] Damage calculation
 - [ ] Casualty handling
 - [ ] Continue battles until at least one side loses all forces
 - [ ] Handle draws when all opposing sides destroy one another
 - [ ] Kill planetary population as applicable and make a colony unowned when its population reaches zero
 - [ ] Restore surviving but damaged ships to full strength after battle
 - [ ] Define victory/defeat determination and any fleet retreat logic
- **Priority:** Medium-low

#### Colonization
- **Status:** Design specified; implementation remains future work
- **Location:** `src/core/src/game.cpp`, `src/core/src/player.cpp`, and `src/core/src/fleet.cpp`
- **Required Behavior:**
  - [ ] After battles resolve, automatically establish a colony when a colony ship arrives at an uncolonized planet
  - [ ] Treat a colony ship as an ordinary ship of the colony-ship design
  - [ ] Resolve non-allied colony-ship battles before colonization
  - [ ] Use colony-ship speed technology to determine which allied colony ship proceeds when allied ships arrive together
  - [ ] Permit a colony ship that survives battle to colonize during the same turn
  - [ ] Establish the colony without consuming the colony ship or additional resources
  - [ ] Update projected income immediately to show the colony's support cost
  - [ ] Calculate exploratory planet desirability for immediate UI use; recalculate and store it in `ColonizedPlanet` when colonized
  - [ ] Recalculate stored desirability during subsequent turn processing
- **Priority:** Medium-high

#### Resource Transfers
- **Status:** Deferred low-priority feature; design specified, implementation absent
- **Location:** `src/core/src/game.cpp`, `src/core/src/player.cpp`, and API action paths
- **Required Behavior:**
  - [ ] Transfer either money or metal between players immediately and without loss
  - [ ] Reject transfers exceeding the sender's metal reserve
  - [ ] Reject transfers that would put the sender below the permitted savings minimum (beyond the permitted debt)
  - [ ] Update both players' persistent balances immediately and expose the result to later same-phase actions
- **Priority:** Lowest

#### Colony Abandonment
- **Status:** Design specified; implementation remains future work
- **Location:** `src/core/src/game.cpp`, `src/core/src/player.cpp`, and colonized-planet state management
- **Required Behavior:**
  - [ ] Queue abandonment during player actions and process it at the beginning of the next turn's economic processing
  - [ ] Remove the colony from the player's owned-colony state and kill its colonists
  - [ ] Preserve any unmined planetary metal for a future colonizer
  - [ ] Leave fleets stationed at the abandoned planet in place
  - [ ] Update displayed/projected income when the abandonment is queued and again when it is applied
  - [ ] Apply the abandonment before calculating the next turn's income
- **Priority:** Medium-high

#### Ship Design Creation
- **Status:** Object creation and storage exist; costs and validation consistency remain
- **Location:** `src/core/src/player.cpp`, `src/core/src/game.cpp`, and `src/core/src/gamestate_c_api_extensions.cpp`
- **Completed:**
 - [x] Basic ShipDesign object creation
 - [x] Design ID allocation
 - [x] Add design to the player's available design list
- **Missing:**
 - [ ] Ensure all direct creation paths apply the same design validation
 - [ ] Calculate and charge design/prototype costs
- **Priority:** Medium

#### Fleet Assembly
- **Status:** Basic fleet objects exist; assembly from built ships and same-turn scrapping rules remain
- **Location:** `src/core/src/player.cpp`, `src/core/src/game.cpp`, and `src/core/src/gamestate_c_api_extensions.cpp`
- **Completed:**
 - [x] Basic Fleet object creation
 - [x] Fleet ID allocation
 - [x] Validate fleet parameters through the GameState/API path
 - [x] Add fleet to the player's fleet list
- **Missing:**
 - [ ] Assemble fleets from ships that have already been built
 - [ ] Ensure direct and API assembly paths apply consistent validation
 - [ ] Track which ships were built during the current player-action phase
 - [ ] Permit full-resource same-phase scrapping for newly built ships
 - [ ] Implement later scrapping for metal only at a heavy loss
- **Dependency:** Fleet assembly relies on sufficiently complete ship-design creation and ship-design cost rules.
- **Priority:** Medium

#### Player Initial Setup
- **Status:** Mostly implemented for the minimal game; starting fleets/designs and explicit initialization tests remain
- **Location:** `src/core/src/game.cpp`, `src/core/src/game_setup.cpp`, and `src/core/src/game_constants.h`
- **Completed:**
 - [x] Player object creation
 - [x] Basic property initialization
 - [x] Initialize the homeworld and `colonized_planets`
 - [x] Initialize `KnowledgeGalaxy`
 - [x] Assign starting population, income, and metal based on colony quality
 - [x] Initialize the default technology, research, and allocation state
 - [x] Initialize empty fleet and ship-design lists
- **Missing or requiring a decision:**
 - [ ] Define all first-turn starting values, including initial savings, metal reserve, population, income, technology, allocations, fleets, and ship designs
 - [ ] Confirm and document intended initial savings and reserve values
 - [ ] Decide whether starting fleets and ship designs should be created
 - [ ] Add focused assertions covering every expected initial field
- **Priority:** High

#### Turn Ready Tracking
- **Status:** Implemented, but not yet enforced as a turn-advance gate
- **Location:** `src/core/src/game.cpp` and `src/core/src/gamestate_c_api_extensions.cpp`
- **Functions:**
 1. `game_mark_player_turn_ready()` - Mark player as ready for next turn.
 2. `game_get_player_turn_ready_status()` - Check if player is ready.
- **Current Implementation:**
 - Readiness is stored per player, can be marked and queried, and is reset after turn processing.
- **Expected Behavior:**
 - Track which players have completed their turn actions
 - Allow game to advance to next turn when all players are ready
 - Support for simultaneous turn processing
- **Priority:** High

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
