# OpenHo Master Plan

**Last Updated:** January 27, 2026 (Session 15 - Galaxy Shapes & Spiral Implementation)  
**Project Status:** Phase 2 - C++ Core Implementation & UI Architecture Planning (In Progress)

---

## Executive Summary

OpenHo is a re-implementation of the classic game "Spaceward Ho!" as a native macOS application. The project is being developed in three phases:

1. **Phase 2 (Current):** C++ game engine with deterministic gameplay and C API interface
2. **Phase 3a:** Objective-C++ bridging layer for macOS integration
3. **Phase 3b:** Native SwiftUI user interface for macOS

The project uses a command-line build system (no Xcode dependency) with CMake for C++ components and Swift Package Manager for the final application.

---

## Project Vision

**Goal:** Create a fully functional, native macOS version of Spaceward Ho! that:
- Runs natively on macOS 10.14 (Mojave) and later
- Maintains deterministic gameplay for multiplayer synchronization
- Uses modern Swift/SwiftUI for the user interface
- Provides a clean C++ core engine with well-defined API boundaries

**Inspiration:** Based on the original Delta Tao game and informed by the Mr-Twave Open_Spaceward_Ho- project.

---

## Development Phases

### Phase 2: C++ Core Implementation (Current)

**Overall Status:** Foundation layer complete, galaxy generation substantially complete, starting planet assignment complete, player perception formulas complete, C API validation layer implemented, economic mechanics implemented

**Deliverable:** Static library `libOpenHoCore.a` with deterministic game engine

---

#### Phase 2a: Core Data Structures & Foundation

**Status:** ✅ COMPLETE

**Completed:**
- ✅ Project structure with CMakeLists.txt
- ✅ Core data structures (Planet, Player, Ship, Galaxy, etc.)
- ✅ Deterministic RNG with dual engines
- ✅ GameState class framework
- ✅ Error code system (23 error codes)
- ✅ Player C API wrapper with validation layer
- ✅ Planet desirability rating infrastructure (placeholder)
- ✅ Entity ID mapping system
- ✅ Population growth system integrated into turn processing
- ✅ Snake_case naming convention applied throughout
- ✅ ColonizedPlanet structure with population/income/allocation
- ✅ Consolidated process_planets() function
- ✅ Income calculation using colonized_planets vector
- ✅ C API wrapper declarations in openho_core.h
- ✅ Planet constructor requiring id, name, and coordinates
- ✅ ColonizedPlanet refactored to pointer-based composition
- ✅ PlanetSnapshot struct with factory methods (partial_info, full_info)
- ✅ Sentinel value constants for unknown/unowned states
- ✅ Game formulas for apparent gravity and temperature (both complete - linear formulas with monotonicity guarantee)
- ✅ Name-based planet lookup system in GameState
- ✅ Player ideal_gravity and ideal_temperature accessors

---

#### Phase 2b: Fleet System & Ship Design

**Status:** ✅ COMPLETE

**Objective:** Replace individual ship management with fleet-based system; enforce immutability of ship designs; reorganize class responsibilities

**Completed:**
- ✅ Fleet struct with multiple identical ships per fleet
- ✅ Fleet position tracking (current_planet, origin_planet, destination_planet)
- ✅ Fleet movement tracking (distance_to_destination, turns_to_destination)
- ✅ Fleet fuel management (refuel() and partial_refuel() methods)
- ✅ ShipDesign tech levels made private with public getters
- ✅ ShipDesign constructor for initialization during creation
- ✅ Friend function set_ship_design_tech() for controlled initialization
- ✅ Player class updated with fleets vector and next_fleet_id counter
- ✅ GameState fleet management methods (create_fleet, get_fleet, delete_fleet, etc.)
- ✅ Ship class completely eliminated
- ✅ Removed galaxy.ships vector
- ✅ Removed player_ships mapping from GameState
- ✅ Created ship_design.h for dedicated ShipDesign header
- ✅ Moved Galaxy initialization to separate galaxy.cpp file
- ✅ Deferred galaxy initialization until user provides parameters
- ✅ Moved fleet management methods from GameState to Player class
- ✅ Moved ship design management methods from GameState to Player class
- ✅ GameState now acts as coordinator/facade with thin delegation methods
- ✅ Clear separation of concerns: Player owns and manages fleets and designs

---

#### Phase 2b-Extended: GameState Architecture Refactoring & Text Assets
**Status:** ✅ COMPLETE (Session: Jan 1, 2026)

**Objective:** Improve GameState architecture for better separation of concerns; implement text asset management system

**Completed:**
- ✅ Moved `current_turn` from Galaxy to GameState (better ownership semantics)
- ✅ Added `current_year` tracking to GameState (independent from turn numbering)
- ✅ Refactored fleet ID allocation: moved from Player to GameState for global uniqueness
- ✅ Fleet constructor made private with Player as friend class (controlled construction)
- ✅ Fleet::fuel changed from double to int32_t for consistency with tech levels
- ✅ Added reverse enum lookup for ShipType using std::map
- ✅ Created GameSetup orchestration layer for game initialization
- ✅ GameState constructor now takes GameSetup reference for one-step initialization
- ✅ GameState copies setup data (galaxy_params, player_setups) as members
- ✅ Added `start_first_turn()` method for initial turn state setup
- ✅ Implemented RNG state serialization for multiplayer host migration
- ✅ Added ColonyQuality enum with reverse lookup (7 quality levels: Outpost→Abundant)
- ✅ Added starting_colony_quality to PlayerSetup (hardcoded to NORMAL)
- ✅ Created TextAssets system with JSON-based text management
- ✅ Implemented TextAssets class with loaders for 5 JSON files
- ✅ TextAssets integrated into GameState constructor
- ✅ Renamed ColonyQuality to StartingColonyQuality with START_ prefix
- ✅ Added backward compatibility aliases for existing code

---

#### Phase 2c: Galaxy Initialization & Random Galaxy Generation

**Status:** ✅ SUBSTANTIALLY COMPLETE (Session: Jan 2-3, 2026)

**Objective:** Generate and populate galaxy with planets; implement robust random galaxy generation with dynamic expansion

**Completed (Session: Jan 3, 2026 - Galaxy Refactoring):**
- ✅ Refactored galaxy generation to use staged coordinate/parameter approach
- ✅ New architecture: generate coordinates → select homes → generate parameters
- ✅ Implemented `generate_planet_coordinates()` dispatcher method
- ✅ Implemented `generate_coordinates_random()` with full expansion logic
- ✅ Implemented `generate_coordinates_grid()` with deterministic spacing
- ✅ Implemented `select_home_planet_coordinates()` with Fisher-Yates shuffle
- ✅ Implemented `generate_planet_parameters()` with home planet special handling
- ✅ Updated Galaxy constructor to use new staged flow
- ✅ Changed `initialize_galaxy()` to return void (was returning vector)
- ✅ Moved `assign_planets_random()` call inside `initialize_galaxy()`
- ✅ Simplified GameState constructor initialization flow
- ✅ All planets now owned by GameState through Galaxy
- ✅ Home planets pre-selected during coordinate generation (not discovered after)
- ✅ Stubbed remaining coordinate methods (spiral, circle, ring, cluster) with TODOs

**Benefits of New Architecture:**
- Clearer separation of concerns (coordinates → selection → parameters)
- Home planets known from start, not discovered after generation
- Easier to add new galaxy shapes
- No throwaway vectors between functions
- More maintainable and testable code

**Completed (Session: Jan 2, 2026 - Previous):**
- ✅ Improved gal_size formula: `sqrt(n_planets) * (5.0 + 6.4 / density)`
- ✅ Galaxy-centered coordinate system (centered at 0,0 instead of corner)
- ✅ Dynamic galaxy expansion algorithm:
  - Initial 15% reduction to minimize wasted space
  - Expands by 5% around center when placement becomes difficult
  - Attempts per expansion phase: `floor(sqrt(n_planets) / 2)`
  - Max attempts per planet: `n_planets`
  - Persistent expanded boundaries across planets
- ✅ CheckDistanceSpatialGrid class for O(N) spatial collision detection
- ✅ Minimum 4.0 unit distance enforcement between all planets
- ✅ Shape-specific initialization methods (random fully implemented, others templated)
- ✅ Grid galaxy shape with deterministic spacing
- ✅ Poisson Disk Sampling documentation created (PoissonDiskSamplingOverview.md)
- ✅ TODO markers for future Poisson disk implementation in circle/ring/cluster shapes
- ✅ Summary output showing original vs actual galaxy size after expansion

**Previously Completed (Session: Jan 1, 2026):**
- ✅ Randomized planet name generation using generate_randomized_subset() utility
- ✅ Generic utility function for randomized subset selection with wraparound naming
- ✅ Planet name generation with suffix numbering (e.g., "Alpha 2", "Beta 3")
- ✅ GalaxyGenerationParams struct with n_planets, density, shape, seed
- ✅ GalaxyShape enum (RANDOM, SPIRAL, CIRCLE, RING, CLUSTER, GRID)
- ✅ Galaxy constructor with deferred initialization
- ✅ Galaxy::initialize_planets() fully implemented for RANDOM shape
- ✅ Ownership refactored: Players owned by GameState, not Galaxy
- ✅ Immutable entity mappings (planet_id_to_index, player_id_to_index, etc.)
- ✅ Mutable entity mappings (player_fleets, player_planets)
- ✅ GameSetup orchestration layer ready
- ✅ GameState initialization complete and tested
- ✅ Text assets system ready for use

**New Constants Added:**
- `Galaxy_Size_Scale_Base = 5.0`
- `Galaxy_Size_Scale_Density = 6.4`
- `Galaxy_Size_Reduction_Factor = 0.15`
- `Galaxy_Expansion_Factor = 0.05`
- `Planet_Placement_Max_Attempts = 10`
- `Planet_Placement_Debug_Threshold = 5`
- `min_planet_distance = 4.0`

**Completed (Session 14 - Jan 26, 2026):**
- [x] Implement Poisson disk sampling (Bridson's algorithm) in utility.h/cpp
- [x] Implement circle galaxy distribution algorithm using Poisson sampling
- [x] Implement ring galaxy distribution algorithm with randomized inner/outer radii
- [x] Add galaxy generation failure logging with detailed reports
- [x] Ensure circle and ring galaxies maintain same active area as random galaxy
- [x] Implement Region abstract base class with CircleRegion and RingRegion

**Completed (Session 15 - Jan 27, 2026 - Spiral Galaxy Implementation):**
- [x] Implement Fermat spiral galaxy distribution algorithm with N arms
- [x] Implement spiral parameter selection (angular span, tightness, ratio)
- [x] Implement arc length calculation using Pythagorean approximation
- [x] Implement spiral arm generation with Poisson-like distribution
- [x] Implement central core generation with existing coordinate avoidance
- [x] Implement home planet selection for spiral galaxies (one per arm end)
- [x] Modify Poisson disk sampling to accept and avoid existing coordinates
- [x] Add spiral helper functions (fermat_spiral_arc_length, fermat_spiral_point)

**Remaining (Next Session):**
- [ ] Implement cluster galaxy distribution algorithm (N clusters in ring arrangement)
- [ ] Test spiral galaxy generation with various parameters
- [ ] Integrate spiral home planet selection into Galaxy constructor
- [ ] Handle gravity mismatch for START_OUTPOST quality colonies
- [ ] Implement error handling when insufficient suitable planets

**Important Note - Min Planets & Homeworld Assignment Strategy:**
- ⚠️ **TODO:** Current "min planets" and "homeworld assignment" strategy is designed for RANDOM and GRID shapes
- For CLUSTER and SPIRAL galaxies, this strategy will need to be reworked
- The solution can likely be integrated into the planet initialization routines for each shape
- Each shape may have different optimal approaches for ensuring suitable home planets are available
- This should be addressed when implementing cluster and spiral shape initialization

---

#### Phase 2d: Starting Planet Assignment & Colony Initialization
**Status:** ✅ COMPLETE (Session: Jan 2, 2026)

**Objective:** Assign starting planets to players; initialize colony values based on quality levels

**Completed (This Session):**
- ✅ Renamed ColonyQuality enum to StartingColonyQuality with START_ prefix
- ✅ Added backward compatibility aliases (COLONY_* names still work)
- ✅ Updated STARTING_COLONY_QUALITY_NAMES map and getter functions
- ✅ Added gravity range constants:
  - `Starting_Planet_Min_Gravity = 0.5`
  - `Starting_Planet_Max_Gravity = 2.0`
- ✅ Created quality-indexed arrays for starting colony values:
  - `Starting_Colony_Population[7]`: 500k to 2M based on quality
  - `Starting_Colony_Metal[7]`: 5k to 30k based on quality
  - `Starting_Colony_Income[7]`: 10k to 60k based on quality
- ✅ Implemented `generate_suitable_home_planets()` method:
  - Filters planets by gravity range [0.5, 2.0]
  - Ensures planets are unowned
  - Returns O(n) lookup of suitable planets
- ✅ Implemented `assign_planets_random()` method:
  - Validates sufficient suitable planets for all players
  - Randomly shuffles planet list using deterministic RNG
  - Assigns one planet per player
  - Sets `player.ideal_gravity = starting_planet.true_gravity`
  - Creates ColonizedPlanet with quality-based values
  - Updates player_planets mapping
  - Logs assignment details

---

#### Phase 2e: Player Ideal Temperature & Gravity Perception
**Status:** ✅ COMPLETE (Session: Jan 3, 2026)

**Objective:** Implement linear perception formulas for temperature and gravity; assign ideal values to players

**Completed (This Session):**
- ✅ Implemented linear temperature perception formula:
  - `perceived_temperature = (best_perceived_temperature_K / ideal_temperature) × true_temperature`
  - Passes through absolute zero (0K → 0K)
  - Monotonic with both true_temperature and ideal_temperature
  - Added constant `best_perceived_temperature_K = 295.37223` (approximately 72°F)
  - Added constant `ideal_temp_range = 55.0` (one-sided range in Kelvin)

- ✅ Implemented linear gravity perception formula:
  - `perceived_gravity = (best_perceived_gravity / ideal_gravity) × true_gravity`
  - Passes through zero gravity (0g → 0g)
  - Monotonic with both true_gravity and ideal_gravity
  - Added constant `best_perceived_gravity = 1.0` (Earth gravity)

- ✅ Implemented player ideal temperature assignment:
  - Truncated Gaussian distribution centered on `best_perceived_temperature_K`
  - σ = `ideal_temp_range` (55K)
  - Range: [240.37K, 350.37K]
  - Assigned during `initialize_players()`

- ✅ Implemented player ideal gravity assignment:
  - Already done in `assign_planets_random()` (set to starting planet's true_gravity)
  - Now also supports gravity range constants for home planets
  - Updated `Starting_Planet_Min_Gravity = 0.7` and `Starting_Planet_Max_Gravity = 1.4`

- ✅ Updated homeworld temperature assignment:
  - Homeworld temperature set to match player's ideal_temperature
  - Ensures players start on planets perfectly suited to their preferences

- ✅ Added RNG normal distribution support:
  - Added `nextNormal(mean, sigma)` method to RNG class
  - Added `nextNormalTruncated(mean, sigma, min, max)` with rejection sampling

- ✅ Improved RNG seed management:
  - Added `getDeterministicSeed()` getter method
  - Fixed `getAISeed()` to return stored value (not placeholder 0)
  - Added seed storage member variables
  - Updated GameState to use `std::random_device` for non-deterministic seeding
  - Eliminated hardcoded 0 seed values

**Formulas Verified:**
- Both formulas create straight lines through origin and (ideal_value, best_perceived_value)
- Tested with ideal values [0.7, 1.0, 1.3] for gravity and [270, 290, 310] for temperature
- All plots confirmed monotonicity and correct behavior

---

#### Phase 2f: Economic Mechanics Implementation
**Status:** ✅ COMPLETE (Session: Jan 26, 2026)

**Objective:** Implement core economic formulas for interest, mining, terraforming, and income allocation

**Completed (This Session):**

**Interest System:**
- ✅ Implemented interest calculation with dual rates:
  - Negative savings (debt): 15% interest (0.15 × savings)
  - Positive savings: Square root formula (10.0 × sqrt(savings))
  - Removed old constant-based interest rates
  - Interest applied to player income, not savings directly

**Mining Mechanics:**
- ✅ Implemented mining formula: `metal_extracted = 20.0 × sqrt(money_spent)`
- ✅ Created inverse function: `money_needed = (metal / 20)²`
- ✅ Mining depletes planet metal reserves
- ✅ Added refund logic: when planet has insufficient metal, player is charged only for available metal and excess money is refunded to savings

**Income Allocation System:**
- ✅ Fixed critical bug where income was double-counted
- ✅ Income now properly distributed according to allocation fractions:
  - `savings_fraction` → added to money_savings
  - `research_fraction` → used for research
  - `planets_fraction` → used for mining/terraforming
- ✅ Money for mining/terraforming comes from income allocation, not savings

**Terraforming Mechanics:**
- ✅ Implemented terraforming formula: `delta_temp = 0.0811 × sqrt(money_spent)` in Fahrenheit
- ✅ Created inverse function for cost calculation
- ✅ Added overshoot prevention: terraforming stops at ideal temperature
- ✅ Refund logic for excess allocation
- ✅ Temperature changes converted between Fahrenheit (formula) and Kelvin (internal storage)

**Temperature Utilities Refactoring:**
- ✅ Reorganized TemperatureUtils namespace in utility.h
- ✅ Added clear distinction between absolute temperature and temperature change conversions
- ✅ Added constants: `BEST_PERCEIVED_TEMPERATURE_K = 295.37223`, `_C = 22.22223`, `_F = 72.0`
- ✅ Renamed offset constants for clarity:
  - `CELSIUS_OFFSET` → `OFFSET_K_TO_C` (273.15)
  - `FAHRENHEIT_OFFSET` → `OFFSET_C_TO_F` (32.0)
- ✅ Added `OFFSET_K_TO_F = 459.67` for direct K-F conversions
- ✅ Optimized K-F conversion functions to use direct offset instead of intermediate C conversion

**Code Refactoring:**
- ✅ Extracted terraforming logic into `process_planet_terraforming()` function
- ✅ Extracted mining logic into `process_planet_mining()` function
- ✅ Simplified `process_planets()` to coordinate planet processing
- ✅ Improved code organization and maintainability
- ✅ Each function has single responsibility

**Recent Commits (Session 13):**
1. `bce41d5` - Rename temperature offset constants for clarity
2. `7894d22` - Add OFFSET_K_TO_F constant and optimize K-F conversions
3. `5955b2f` - Refactor terraforming and mining into separate functions

---

#### Phase 2g: Galaxy Shape Distribution Algorithms (Circle & Ring)
**Status:** COMPLETE (Session: Jan 26, 2026)

**Objective:** Implement Poisson disk sampling and circle/ring galaxy shape generators

**Completed (This Session):**
- [x] Implemented Bridson's Poisson disk sampling algorithm in utility.h/cpp
- [x] Implemented circle galaxy shape generator using Poisson sampling
- [x] Implemented ring galaxy shape generator with randomized inner/outer radii
- [x] Added galaxy generation failure logging with detailed reports
- [x] Renamed select_home_planet_coordinates to select_home_planets_random

**Design Highlights:**
- Active area approach ensures consistent planet density across shapes
- Ring galaxies support inner/outer ratio constraints (50%-80%)
- Poisson sampling guarantees minimum distance between planets
- Failure logging enables debugging of insufficient planet generation

---

### Key Design Decisions

**Player-GameState Relationship:**
- Player holds pointer to GameState for fleet ID allocation
- Keeps internal bookkeeping opaque to UI
- GameState owns all validation logic

**Validation Pattern:**
- game_can_player_X() - Check validity (returns ErrorCode)
- game_player_X() - Execute action (re-validates, then delegates)
- Prevents invalid state transitions

**Planet Desirability:**
- 1-3 scale (Poor, Moderate, Excellent)
- Stored with ColonizedPlanet
- Recalculated at start of turn processing
- Placeholder: all planets maximally desirable

**Player Perception Model:**
- Linear formulas for temperature and gravity perception
- Both pass through zero/origin point
- Monotonic with true values
- Ensures consistent player experience across different ideal values

**Temperature Unit Handling:**
- Internal storage: Kelvin (absolute temperature)
- Formulas: Fahrenheit (for game balance)
- Conversions: Explicit, with clear function names distinguishing absolute vs. change

**Economic System:**
- Interest applied to income, not savings directly
- Income allocated by fractions before spending
- Refunds for insufficient resources (metal, temperature overshoot)
- Square root formulas for diminishing returns on mining/terraforming

---

### Recent Commits

**Session 14 (Jan 26, 2026) - Galaxy Shapes & Poisson Sampling:**
1. `3d32985` - Rename select_home_planet_coordinates to select_home_planets_random
2. `b56d59c` - Implement Poisson disk sampling and Circle/Ring galaxy generators

**Session 13 (Jan 26, 2026) - Economic Mechanics & Code Refactoring:**
1. `bce41d5` - Rename temperature offset constants for clarity
2. `7894d22` - Add OFFSET_K_TO_F constant and optimize K-F conversions
3. `5955b2f` - Refactor terraforming and mining into separate functions

---

**This document is the single source of truth for the OpenHo project. It should be updated after each significant session to reflect progress and maintain accuracy for future sessions.**
