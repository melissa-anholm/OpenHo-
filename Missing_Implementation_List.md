# Missing Implementations List

**Purpose:** Track all functions and values that need implementation or reverse-engineering from the original game.

**Last Updated:** November 8, 2026 (fleet-management CLI)

---

> **Economic-kernel status:** The main economic path now applies population growth, refreshes planetary income, calculates interest from beginning-of-turn savings, applies Dip Into Savings, and commits one effective-income value before allocation. Full snapshots, queued actions, and later fleet phases remain future work.

> **Design reminder — double-check later:** Verify the ordering of colonization relative to economic processing. Leave `TurnProcessingOrder.md` unchanged for now.

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
- **Status:** Implemented. Requires final data-driven double-check.
- **Formula:** For negative savings, `interest = 0.15 × savings`; the result is negative.
- **Application:** Calculated from `turn_opening_savings` and added directly to the player's current-turn income pool before effective-income allocation. It is not applied directly to the savings balance at this step.
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
- Current-turn interest is calculated from beginning-of-turn savings before Dip Into Savings and allocation.
- **Rate modifier:** `Player::savings_rate_modifier` now augments the positive base savings rate only: `actual_savings_rate = base_positive_savings_rate × savings_rate_modifier`. It defaults to `1.0` and is not yet serialized. Negative interest is a separate debt rule and is not modified by this factor under the current design.
- **Application:** Calculated interest, whether positive or negative, is added to player income; it is not applied directly to the savings balance.
- **Priority:** Medium-low

#### Mining Efficiency
- **Status:** Provisional functionality.  No idea whether this formula is correct.
- **Formula:** `metal_extracted = 20.0 × sqrt(money_spent)`
- Inverse: `money = (metal / 20.0)²`
- **Resource flow:** Money from the current positive effective-income allocation is spent during economic processing; extracted metal is transferred losslessly from the planet to the player's shared metal reserve. Planetary metal is clamped at zero.
- **Feature:** Depleted-metal refund logic is implemented; the formula and data-driven behavior still require validation.
- **Priority:** Medium-low

#### Terraforming Efficiency
- **Status:** Provisional functionality.  No idea if this is correct.
- **Formula:** `temperature_change_F = 0.0811 × sqrt(money_spent)`
- Inverse: `money = (temperature_change_F / 0.0811)²`
- **Resource flow:** Money from the current positive effective-income allocation is spent during economic processing; terraforming does not consume metal. Temperature changes immediately during economic processing.
- **Feature:** Overshoot prevention and excess-allocation refunds are implemented. The current refund destination is `money_savings`, pending a future design decision.
- **Priority:** Medium-low


---

### Data-Driven Values (Require Reverse-Engineering) - 5 items

#### Negative Income Mechanics
- **Status:** Dip Into Savings and negative-income processing are functionally implemented; debt-limit formulas remain provisional, while intentional-debt enforcement and once-per-turn bankruptcy detection are in place. Bankruptcy consequences remain deferred.
- **Location:** `src/core/src/game.cpp`, `src/core/include/player.h`, `src/core/include/game_constants.h`, and related action APIs
- **Implemented behavior:**
  - The player selects a positive dip during the player-action phase for the following economic phase.
  - Selecting or changing a pending dip immediately adjusts savings; changing it back is lossless. Requests are capped at the currently permitted intentional-debt amount.
  - Separate pending and locked amounts and persistence modes are maintained.
  - The pending amount is locked for the next economic phase without a second deduction.
  - `effective_income = net_income + locked_dip_amount` during economic processing.
  - Persistent pension-plan dips renew automatically and are re-evaluated at renewal; if capacity decreases, the stored recurring request is reduced to the permitted amount. One-time dips apply for only one processed turn.
  - Positive effective income is allocated according to the player's budget; negative effective income funds no categories and is charged directly to savings.
  - A player may recover if later net income becomes positive; one negative-income turn does not immediately cause bankruptcy.
  - Ship construction checks complete money cost against the same intentional-debt capacity.
  - Bankruptcy-limit detection runs once per completed turn; it records the threshold transition but does not yet eliminate the player.
- **Current provisional debt policy:** The exact current equations and function behavior are:

  ```text
  intentional_debt_limit(I, planet_count):
      if I <= 0:
          return 0
      r = GameConstants::Provisional_Debt_Interest_Rate
      N = 10
      repayment_factor = (1 - (1 + r)^(-N)) / r
      return floor(I × repayment_factor)

  bankruptcy_debt_limit(I, planet_count):
      r = GameConstants::Provisional_Debt_Interest_Rate
      multiplier = GameConstants::Provisional_Bankruptcy_Debt_Multiplier
      reference_income = GameConstants::Provisional_Bankruptcy_Reference_Income
      return floor(multiplier × max(I, reference_income) / r)

  available_build_money(current_savings, I, planet_count):
      return max(0, current_savings
                    + intentional_debt_limit(I, planet_count))

  has_reached_bankruptcy_limit(current_savings, I, planet_count):
      return current_savings
             <= -bankruptcy_debt_limit(I, planet_count)
  ```

  Here, `I` is summed planetary income. `planet_count` is accepted explicitly by all four functions but does not affect the current provisional equations; it is reserved for a future data-driven refinement. The rate `r` is referenced by these limit calculations; it is not charged a second time. Actual negative savings interest is calculated separately and added to the player's income, not directly to savings.
- **Current provisional parameters:** `GameConstants::Provisional_Debt_Interest_Rate = 0.15`, `GameConstants::Provisional_Intentional_Debt_Repayment_Turns = 10`, `GameConstants::Provisional_Bankruptcy_Debt_Multiplier = 5`, and `GameConstants::Provisional_Bankruptcy_Reference_Income = 50000`, all defined in `src/core/include/game_constants.h`. `calculate_available_build_money()` uses the income-dependent intentional-debt limit; `has_reached_bankruptcy_limit()` checks the income-dependent bankruptcy limit.
- **Remaining:**
  - [ ] Determine whether and how planet count should affect the intentional-debt and bankruptcy formulas.
  - [ ] Replace the provisional interest rate, repayment horizon, bankruptcy multiplier, and reference income with data-driven rules.
  - [ ] Implement bankruptcy and loss when the larger negative-savings limit is reached.
- **Priority:** Medium-high

#### Planetary Income Calculation
- **Status:** Provisional main-path formula with corrected Phase 1 ordering; standalone helper remains a stub
- **Location:** `src/core/src/game.cpp` and `src/core/src/game_formulas.cpp`
- **Current Implementation:**
  - The main turn path refreshes colony income after population growth using a provisional gravity/temperature condition factor.
  - `calculate_planetary_income()` returns 0 and is not wired into the main path.
  - Income is summed after colony income is refreshed; the formula itself remains provisional.
- **Priority:** Medium-high

#### Ship Design Costs
- **Status:** Provisional functionality implemented; final data-driven values remain future work.
- **Location:** `src/core/src/game_formulas.cpp`, `src/core/include/game_constants.h`, and `src/core/include/game_formulas.h`
- **Authority:** See [Ship Building Cost Specification](#ship-building-cost-specification) below for the current parameters, formulas, rounding policy, prototype accounting, and resource-limit calculations.
- **Implemented Functions:** `calculate_ship_design_prototype_cost()`, `calculate_ship_design_build_cost()`, `calculate_ship_design_metal_cost()`, and `check_ship_design_levels()`.
- **Priority:** Medium

#### Population Growth
- **Status:** Integrated into the Phase 1 economic order, but formula remains a placeholder
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
### Non-Data-Driven Functions (Must Still Choose an Implementation) - 2 items

#### Planet Desirability Rating
- **Status:** Placeholder
- **Location:** `src/core/src/colonized_planet.cpp` and related game logic
- **Current Implementation:**  `update_desirability()` assigns the placeholder value `3` to every colony; no real desirability calculation is implemented.
- **Priority:** Low

#### Player Metrics
- **Status:** Placeholder (2 functions)
- **Location:** `src/core/src/game_formulas.cpp`
- **Functions Needed:**
	1. `calculate_player_fleet_power()` - Public fleet-strength metric
	2. `calculate_player_victory_points()` - Victory condition tracking
- **Priority:** Low

#### Special Events
- **Status:** Deferred low-priority placeholder functionality.
- **Location:** `src/core/src/game_formulas.cpp` and `src/core/src/game.cpp`
- **Current Implementation:** `calculate_planet_nova_warning_duration()` returns `1`; `calculate_windfall_income()` returns `0`; `process_novae()` is a no-op.
- **Expected Behavior:** Eventually implement rare windfall and nova events, including warning duration, event timing, effects, and any distance-dependent rules.
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
  - [x] Player-level allocation fractions must sum to approximately `1.0`
  - [x] Accepted submissions require all per-planet funding fractions to sum to approximately `1.0`
  - [x] Each planet's mining and terraforming fractions must sum to approximately `1.0`
  - [x] Allocation funds come from current income only
  - [ ] Current application/order may require correction; see the turn-processing note above.
- **Allocation hierarchy reference:** The player-level planetary-development fraction is applied first, and each owned colony then receives a fraction of that planetary-development budget:
  ```text
  effective income
  └── player allocation
      ├── savings
      ├── research
      └── planetary development
          └── per-planet funding fraction
              ├── mining fraction
              └── terraforming fraction
  ```
  The corresponding nominal calculation is:
  ```text
  total planetary-development budget = effective income × planets_fraction
  per-planet budget = total planetary-development budget × funding_fraction
  mining budget = per-planet budget × mining_fraction
  terraforming budget = per-planet budget × terraforming_fraction
  ```
  `GameState::submit_economic_action()` rejects incomplete, duplicate, or non-normalized per-planet funding records using the shared machine-epsilon tolerance.
  This is a design reference, not a separate missing feature. Exact current-turn spending and refunds are not presently retained as fields.
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
 - [ ] Diplomacy/trade events (deferred; not required for core game flow)
 - [ ] Special events, including windfalls and novae (deferred; see Special Events above)
 - [ ] Capture canonical Turn N snapshots at the beginning of each turn, with Turn 0/year 2000 initialization
 - [ ] Capture economic after-state and post-processing state for history, diagnostics, and display
 - [ ] Complete end-of-turn state/knowledge updates with public/private filtering from the same snapshot boundary
 - [ ] Serialize and restore the complete mutable mid-game state, including remaining knowledge/history systems and RNG state. Planet-owned scalar `PlanetView` snapshots, heterogeneous fleet state, fleet sightings still not represented in views, and pending fleet actions are serialized; related public-information history remains incomplete. The current bounded between-turn payload includes explicit player elimination status, both RNG engine states, and the previous turn’s cached metal income; the immutable game-definition payload and append-only container are implemented separately
 - [ ] Synchronize critical state at least once per turn for host migration
 - [ ] Enforce readiness in the core turn-processing entry point
 - [ ] Exclude eliminated players from readiness requirements
 - [ ] Require an explicit host decision after a player disconnects; transfer that responsibility on host migration
 - [ ] Allow post-ready actions to make a player unready, with final automatic-versus-explicit re-ready policy still to be chosen
 - [ ] Bound AI turn execution with a configurable time limit
- **Priority:** High

#### Fleet Movement
- **Status:** Queued movement, explicit stationed/transit state, persistence, CLI inspection/management, and basic range/fuel/speed validation are implemented; fuel consumption and final action-order rules remain
- **Location:** `src/core/src/game.cpp`, `src/core/src/player.cpp`, and `src/core/src/fleet.cpp`
- **Completed:**
 - [x] Fleet movement calculation (distance-based)
 - [x] Queue complete fleet movement submissions for the next turn's fleet-processing phase
 - [x] Require valid fleet submissions at the readiness boundary
 - [x] Represent each fleet with one explicit stationed-or-in-transit location state
 - [x] Fleet arrival processing using canonical planet names
 - [x] Serialize and restore fleets, heterogeneous stacks, per-stack fuel, and in-transit state in runtime checkpoints
 - [x] Expose fleet inspection, stationed-fleet listing, queued movement, immediate composition actions, refueling, and scrapping through the portable CLI
 - [x] Display the complete queued movement and pending-scrapping lists from the consolidated `fleets` command
 - [x] Reject movement when effective speed is zero, destination distance exceeds effective range, or effective fuel is insufficient
 - [ ] Consume fuel according to the final travel rule
 - [ ] Complete movement validation against final action-order rules
- **Priority:** Medium-high

#### Force Combat
- **Status:** Not implemented
- **Location:** `src/core/src/game.cpp`, `src/core/src/player.cpp`, and `src/core/src/fleet.cpp`
- **Missing:**
 - [ ] Resolve fleet travel before battle resolution at each destination planet
 - [ ] Detect when fleets and owned planets belonging to two or more non-allied players occupy the same place
 - [ ] Resolve all such forces as participants in one battle rather than separate fleet-versus-fleet and fleet-versus-planet systems
 - [ ] Prevent invading weapons from targeting the planet until all defending ships are destroyed
 - [ ] Allow the planet to fire at attackers before it becomes targetable
 - [ ] Combat resolution for battles involving three or more players
 - [ ] Damage calculation
 - [ ] Define force-turn order and technology effects
 - [ ] Casualty handling
 - [ ] Continue battles until at least one side loses all forces
 - [ ] Handle draws when all opposing sides destroy one another
 - [ ] Destroy planetary population when the planet is targeted and make the colony unowned when its population reaches zero
 - [ ] Restore surviving but damaged ships to full strength after battle
 - [ ] Define victory/defeat determination and any fleet retreat logic
- **Priority:** Medium-low

#### Colonization
- **Status:** Direct colonization and basic automatic colony-ship colonization are implemented; battle ordering, survivor selection, and final observation rules remain
- **Location:** `src/core/src/game.cpp`, `src/core/src/player.cpp`, and `src/core/src/fleet.cpp`
- **Required Behavior:**
  - [x] After the current arrival-processing phase, automatically establish a colony when a fleet containing a colony ship arrives at an uncolonized planet
  - [x] Require the qualifying ship to use the `SHIP_COLONY` design type; ordinary fleets cannot colonize
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
- **Status:** Functional with provisional costs; player-facing selection and final data-driven formulas remain future work.
- **Location:** `src/core/src/player.cpp`, `src/core/src/game.cpp`, and `src/core/src/gamestate_c_api_extensions.cpp`
- **Completed:**
 - [x] Basic ShipDesign object creation
 - [x] Design ID allocation
 - [x] Add design to the player's available design list
- [x] Ensure all direct creation paths apply the same design validation
- [x] Store a `has_been_prototyped` flag directly with each design
- [x] Calculate and store provisional build, prototype, and metal costs
- **Remaining:**
 - [ ] Replace provisional cost formulas with final data-driven formulas
 - [ ] Enforce current technology limits when creating new designs, while allowing already-owned designs to remain buildable even if they exceed the player's current technology access
 - [ ] Support an uncommitted proposed design in the build-selection flow without adding it to the design library until a build occurs
- **Priority:** Medium

#### Ship Construction and Immediate Fleet Creation
- **Status:** Basic construction and fleet scrapping are functionally implemented with provisional costs and refund calibration; the complete player-facing build process remains.
- **Location:** `src/core/src/c_api.cpp`, `src/core/src/player.cpp`, `src/core/src/game_formulas.cpp`, and fleet creation paths
- **Required Behavior:**
 - [ ] Allow the player to choose any already-owned design, or configure a new design whose levels are legal under the player's current technology
 - [ ] Calculate the maximum buildable ship count for the proposed design from shared metal and available savings
 - [x] Add `calculate_available_build_money(current_savings, summed_planetary_income, planet_count)` using the provisional intentional-debt headroom
 - [x] Apply the current permitted-intentional-debt limit when validating the complete money cost
 - [x] Detect the separate larger bankruptcy/default limit once per completed turn
 - [ ] Apply the [Ship Building Cost Specification](#ship-building-cost-specification), including one prototype charge followed by `N × individual resource cost` for an unprototyped design
 - [ ] Allow construction at any planet owned by the player, with no shipyard or planet-specific build limit
 - [x] Build N ships of one design as one immediate fleet
 - [x] Mark newly created fleets as newly built for the remainder of the current turn
 - [x] Charge the individual money and metal costs atomically
 - [x] Add the money-only prototype cost for the first ship built from a design
 - [x] Reject unaffordable actions without partial resource deductions
 - [x] Permit multiple build actions when resources remain sufficient
 - [x] Record prototype completion so later builds from that design omit the prototype cost
 - [x] Track first-time prototype charges as Player-owned, turn-scoped refund candidates
 - [x] Permit full-resource same-phase scrapping for newly built ships/fleets
   - **Status:** Implemented for complete fleets; cancels any queued travel order first and invalidates readiness.
 - [x] Implement later scrapping for metal only at a heavy loss
   - **Provisional formula:** `refunded_metal = floor(0.15 × metal_build_cost_of_fleet)`
   - **Status:** Implemented for stationed fleets; the 15% factor requires data-driven calibration before final deployment.
 - [x] Keep a newly created design in the player's library if all ships built from it are scrapped, but roll back its prototype marker before the player-action phase ends
 - [x] Prevent same-turn prototype rollback from avoiding the prototype charge on a later turn
- **Priority:** High

#### Ship Building Cost Specification

> **Authority:** This section is the authoritative reference for the current provisional ship-building cost model. It is not a final balance specification or permanent implementation requirement.

**Scope and parameters:** Supported ship types are Satellite, Fighter, Scout, Colony ship, Tanker, and Dreadnought. Biological ships and radical technology are deferred. Ordinary technology streams are range, speed, weapons, and shields. Every supported type permits every miniaturization level. Money and metal are separate resources with separate formulas; quantities of the two resources must never be added together or inferred from one another by conversion.

| Ship type | Base money | Base metal | Money tech multiplier | Metal tech multiplier |
| --- | ---: | ---: | ---: | ---: |
| Satellite | 1,000 | 100 | 0.4 | 0.4 |
| Fighter | 6,000 | 600 | 1.0 | 1.0 |
| Scout | 4,000 | 400 | 0.85 | 0.85 |
| Colony ship | 16,000 | 1,600 | 2.2 | 2.2 |
| Tanker | 12,000 | 1,200 | 1.8 | 1.8 |
| Dreadnought | 150,000 | 15,000 | 20.0 | 20.0 |

All values are provisional and should ultimately be centralized as configuration parameters.

**Ordinary technology costs:** Current placeholder increments for a transition from level `N` to `N + 1` are:

| Stream | Money increment | Metal increment |
| --- | ---: | ---: |
| Range | 100 | 10 |
| Speed | 100 | 10 |
| Weapons | 100 | 10 |
| Shields | 100 | 10 |

The eventual increment may depend on resource, stream, ship type, and `N`. Each stream must retain separately replaceable money and metal formulas with `N` explicit. Game-state/API levels are integers and must be converted explicitly to `double` before cost calculations.

For resource `R`, ship type `S`, stream `T`, and final level `L`:

```text
ordinary technology cost(R, S, T, L)
    = sum for N = 0 through L - 1 of
      base increment(R, T, N) × multiplier(R, S, T)
```

The multiplier uses the ship-type/resource default above unless a stream-specific override applies. Technology locks are also supported:

- **Satellite speed:** higher levels have zero money and metal cost; effective multiplier is zero.
- **Satellite range:** locked at level 0; the multiplier is irrelevant for valid designs but the lock should be explicit.

**Cost formulas and miniaturization:** Calculate separate pre-miniaturization totals, then apply miniaturization to each resource independently:

```text
pre-mini money = base money
               + range money + speed money + weapons money + shields money

pre-mini metal = base metal
               + range metal + speed metal + weapons metal + shields metal

final money build cost = ceil(
    pre-mini money × miniaturization money multiplier(M))

final metal build cost = ceil(
    pre-mini metal × miniaturization metal multiplier(M))
```

All intermediate calculations remain `double`; each public final resource cost rounds upward with `ceil`. Miniaturization receives the complete pre-mini total, not merely the base or technology increment, and is applied after all ordinary technology costs.

| Resource | Provisional miniaturization multiplier |
| --- | --- |
| Metal | `0.9^M` (`M=0,1,2` → `1.00, 0.90, 0.81`) |
| Money | `1.0` when `M=0`; `3.0 × M` when `M > 0` (`M=0,1,2` → `1.00, 3.00, 6.00`) |

The `0.9` metal factor and `3.0` positive-level money contribution are adjustable placeholders. The money function will probably become explicitly level-dependent.

**Prototype and multi-ship accounting:** The first ship built from a design incurs a provisional **money-only** prototype charge:

```text
prototype money cost = ceil(10 × unrounded money build-cost expression)
```

The 10× multiplier is placeholder behavior. Prototype calculations also retain `double` precision until their own final upward rounding and must not truncate the underlying money expression. No separate prototype metal charge applies.

For `N` ships, money and metal remain separate:

```text
individual money build cost = final money build cost
individual metal build cost = final metal build cost

required money, unprototyped =
    prototype money cost + N × individual money build cost

required money, prototyped =
    N × individual money build cost

required metal, either state =
    N × individual metal build cost
```

The prototype charge is applied once per build action, never once per ship. A design becomes prototyped only when it is actually used for a build.

For a proposed build:

```text
metal-limited quantity =
    floor(available metal / individual metal build cost)

money-limited quantity, unprototyped =
    floor((available build money - prototype money cost)
          / individual money build cost)

money-limited quantity, prototyped =
    floor(available build money / individual money build cost)

maximum quantity = min(applicable money-limited quantity,
                       metal-limited quantity)
```

Other game limits may further reduce the result.

**Debt boundaries and implementation constraints:** Two distinct negative-savings thresholds are required:

- **Intentional-debt limit:** maximum negative savings permitted deliberately by actions such as ship building or Dip Into Savings.
- **Bankruptcy/default limit:** larger negative-savings threshold; reaching it causes the player to lose by bankruptcy.

The current implementation uses provisional constants. `calculate_available_build_money(current_savings)` represents current savings plus remaining intentional-debt headroom, clamped at zero. A separate bankruptcy helper reports whether the larger threshold has been reached. Both values and their enforcement rules require data-driven replacement.

Implementation constraints:

- Keep money and metal formulas, values, and affordability calculations separate.
- Keep `N` explicit in replaceable level-transition functions.
- Support ship-type defaults, stream-specific overrides, and technology locks.
- Centralize provisional parameters rather than scattering literals.
- Keep intermediate calculations in `double` and round final public resource costs upward.

Final model questions:

- Level-dependent formulas for each ordinary stream and resource.
- Final ship-type multipliers and stream overrides.
- Final metal and money miniaturization functions.
- Final prototype-cost rule.
- Additional technology locks or ship-specific exceptions.
- Final intentional-debt and bankruptcy/default values and enforcement rules.

#### Fleet Composition and Assembly
- **Status:** Immediate merge, split, stack-transfer, runtime checkpoint restoration, and portable CLI commands are implemented
- **Location:** `src/core/src/player.cpp`, `src/core/src/game.cpp`, and `src/core/src/gamestate_c_api_extensions.cpp`
- **Completed:**
 - [x] Basic Fleet object creation
 - [x] Fleet ID allocation
 - [x] Validate fleet parameters through the GameState/API path
	 - [x] Add fleet to the player's fleet list
	 - [x] Query a player's non-empty fleets stationed at a named planet
	 - [x] Represent fleet contents as design-specific ship stacks, while retaining one-stack construction as the current default
	 - [x] Merge two stationed fleets immediately, coalescing matching design stacks
	 - [x] Split a stack into a new stationed fleet immediately
	 - [x] Move part or all of a stack into another stationed fleet immediately
	 - [x] Store fuel per design stack and derive effective fleet fuel, range, and speed
	 - [x] Mark the player unready after a successful composition action
	- **Missing:**
	 - [ ] Add equivalent native AppKit UI commands for fleet inspection and composition
		 - [x] Serialize and restore heterogeneous fleet contents and per-stack fuel
		 - [x] Serialize and restore stationed and in-transit fleet locations
		 - [x] Serialize and restore queued fleet movements and deferred scrapping requests
		 - [x] Implement same-phase scrapping for newly built stationed fleets and deferred scrapping for older stationed fleets
- **Note:** Composition actions require fleets to be stationed at the same planet. Matching-stack transfers and merges use the lower fuel value; newly-built status is combined with logical AND.
- **Dependency:** Future fleet composition relies on sufficiently complete ship-design and ship-construction rules, but is not required for basic construction or travel.
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
   - Special events, including windfall income and nova events
   - Galaxy-shape validation and edge-case testing
