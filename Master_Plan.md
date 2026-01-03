# OpenHo Master Plan

**Last Updated:** January 3, 2026 (Session 4)  
**Project Status:** Phase 2 - C++ Core Implementation (In Progress)

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

**Overall Status:** Foundation layer complete, galaxy generation substantially complete, starting planet assignment complete

**Deliverable:** Static library `libOpenHoCore.a` with deterministic game engine

---

#### Phase 2a: Core Data Structures & Foundation

**Status:** ✅ COMPLETE

**Completed:**
- ✅ Project structure with CMakeLists.txt
- ✅ Core data structures (Planet, Player, Ship, Galaxy, etc.)
- ✅ Deterministic RNG with dual engines
- ✅ GameState class framework
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
- ✅ Game formulas for apparent gravity and temperature
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

**Status:** ✅ SUBSTANTIALLY COMPLETE (Session: Jan 2, 2026)

**Objective:** Generate and populate galaxy with planets; implement robust random galaxy generation with dynamic expansion

**Completed (This Session - Jan 2, 2026):**
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

**Remaining (Next Session):**
- [ ] Implement spiral galaxy distribution algorithm
- [ ] Implement circle galaxy distribution algorithm (candidate for Poisson disk sampling)
- [ ] Implement ring galaxy distribution algorithm
- [ ] Implement cluster galaxy distribution algorithm
- [ ] Handle gravity mismatch for START_OUTPOST quality colonies
- [ ] Implement error handling when insufficient suitable planets

**Important Note - Min Planets & Homeworld Assignment Strategy:**
- ⚠️ **TODO:** Current "min planets" and "homeworld assignment" strategy is designed for RANDOM and GRID shapes
- For CLUSTER and SPIRAL galaxies, this strategy will need to be reworked
- The solution can likely be integrated into the planet initialization routines for each shape
- Each shape may have different optimal approaches for ensuring suitable home planets are available
- This should be addressed when implementing cluster and spiral shape initialization

**Future Enhancement (Phase 2c-Extended):**
- [ ] Implement Poisson disk sampling (Bridson's algorithm) for circle/ring/cluster shapes
- [ ] See Documentation/PoissonDiskSamplingOverview.md for algorithm details
- [ ] Benefits: O(N) performance, no placement failures, uniform distribution
- [ ] Will replace current naive random placement for these shapes

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
- ✅ Quality progression (0.5× to 2.0× baseline):
  - START_OUTPOST: Minimal (0.5×)
  - START_BARREN: Low (0.75×)
  - START_BACKWARD: Below normal (0.875×)
  - START_NORMAL: Baseline (1.0×)
  - START_ADVANCED: Above normal (1.25×)
  - START_THRIVING: High (1.5×)
  - START_ABUNDANT: Very high (2.0×)
- ✅ TODO marker for future value adjustments during balance testing

**Architecture Decisions:**
- Player's ideal_gravity set to starting planet's true_gravity (eliminates immutability issues)
- Suitable planet filtering ensures reasonable starting conditions
- Quality-indexed arrays enable easy tuning of starting values
- Deterministic RNG ensures reproducible planet assignment

**Remaining (Next Session):**
- [ ] Implement gravity mismatch for START_OUTPOST (gravity doesn't match ideal)
- [ ] Implement error handling for insufficient suitable planets
- [ ] Differentiate colony values by quality level (currently uniform)
- [ ] Implement metal field in ColonizedPlanet if needed

---

## Implementation Notes

### Key Design Decisions (Updated)

1. **Dual RNG Engines:**
   - `deterministicRNG`: Shared game mechanics (reproducible across all players)
   - `aiRNG`: AI decisions (seeded each turn by host, allows variation)

2. **Galaxy Coordinate System:**
   - Centered at (0, 0) for symmetrical expansion
   - Initial bounds: `[-gal_size/2, gal_size/2]` for both x and y
   - Expands dynamically around center as needed

3. **Starting Planet Assignment:**
   - Gravity range [0.5, 2.0] ensures reasonable starting conditions
   - Player's ideal_gravity set to starting planet's true_gravity
   - Eliminates need to modify immutable true_gravity
   - Ensures apparent_gravity = true_gravity initially (no penalty)

4. **Quality-Indexed Arrays:**
   - All starting colony values indexed by StartingColonyQuality enum
   - Enables easy tuning and differentiation by quality level
   - TODO markers remind to adjust values during balance testing

### Recent Commits (Session 4 - Jan 3, 2026)

1. `6f54cca` - Implement galaxy generation validation and retry logic
2. `0c08966` - Add TODO reminders for cluster and spiral galaxies
3. `cffb6dd` - Implement per-player ideal_gravity assignment with START_OUTPOST randomization

### Previous Commits (Session 3 - Jan 2, 2026)

1. `8ed7cda` - Implement spatial grid for planet collision detection
2. `b184f0c` - Implement improved galaxy size formula for random shape
3. `abeaa9e` - Update gal_size calculation in initialize_planets_random
4. `3ef8118` - Implement galaxy expansion algorithm with dynamic boundaries
5. `d2eb995` - Implement starting planet assignment for random galaxies
6. `9307af8` - Add quality-indexed arrays for starting colony values
7. `c9a05f6` - Add TODO reminder for starting colony value adjustments

---

## Next Steps

**Immediate (Phase 2d continuation - Session 4 Complete):**
- ✅ Implement galaxy generation validation and retry logic (up to 5 attempts)
- ✅ Implement per-player ideal_gravity assignment
- ✅ Implement START_OUTPOST randomization (±0.20, constrained, rounded to 2 decimals)
- ✅ Fix initialization flow and eliminate inefficiency
- [ ] Test starting planet assignment with various configurations
- [ ] Validate START_OUTPOST gravity randomization

**Next (Phase 2d continuation):**
- [ ] Implement gravity mismatch penalty for START_OUTPOST
- [ ] Add validation for edge cases
- [ ] Test with multiple player configurations

**Short-term (Phase 2c continuation):**
- Implement spiral, circle, ring, and cluster galaxy distribution algorithms
- Consider Poisson disk sampling for circle/ring/cluster shapes
- Test all galaxy shapes with various parameters

**Medium-term (Phase 2e - Game Mechanics):**
- Implement turn processing with income/interest calculations
- Implement production and construction systems
- Implement fleet movement and combat mechanics
- Implement technology research system

**Long-term (Phase 3 - UI):**
- Objective-C++ bridging layer
- SwiftUI user interface
- macOS application packaging

---

**This document is the single source of truth for the OpenHo project. It should be updated after each significant session to reflect progress and maintain accuracy for future sessions.**
