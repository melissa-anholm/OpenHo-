# OpenHo Master Plan

**Last Updated:** January 3, 2026 (Session 5 - Extended - Continued)  
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

**Overall Status:** Foundation layer complete, galaxy generation substantially complete, starting planet assignment complete, player perception formulas complete, C API validation layer implemented

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

---

### Recent Commits (Session 5 Extended - Continued)

1. `e6710c6` - Refactor galaxy generation to use staged coordinate/parameter approach
2. `bd27e5a` - Implement gravity perception formula and update gravity constants
3. `fe7287b` - Implement player ideal temperature assignment and temperature perception formula
4. `cffb6dd` - Implement per-player ideal_gravity assignment with START_OUTPOST randomization
5. `69b8b8b` - Rename MASTER_PLAN.md to Master_Plan.md and update with Session 4 progress
6. `586c644` - Implement Player C API wrapper and GameState validation layer
7. `88d01a9` - Consolidate duplicate error codes
8. `d4231f9` - Add validation methods to GameState and getter methods to Player
9. `c5a14ae` - Update Missing_Implementation_List.md with corrected categorizations

---

**This document is the single source of truth for the OpenHo project. It should be updated after each significant session to reflect progress and maintain accuracy for future sessions.**


---

### Session 6: Fleet Architecture Refactoring & Research Optimization (Jan 4, 2026)

**Status:** ✅ COMPLETE

**Objective:** Refactor fleet creation architecture for cleaner ownership semantics; optimize research processing with cost caching; plan fleet/knowledge map system

**Completed:**

#### 1. Fleet Creation Architecture Refactoring ✅
- ✅ Eliminated redundant `GameState::find_suitable_home_planets()` function
- ✅ Removed `GameState::create_fleet()` - Player now owns the entire creation process
- ✅ Added `GameState* game_state` member to Player class
- ✅ Split `Player::create_fleet()` into three methods:
  - `validate_fleet()`: Validates design_id, ship_count, planet_id
  - `build_fleet()`: Creates Fleet after validation and fleet_id allocation
  - `create_fleet()`: Orchestrates both steps, requests fleet_id from GameState
- ✅ Updated `GameState::initialize_players()` to pass `this` to Player constructor
- ✅ Updated C API wrapper `game_player_build_fleet()` to call `Player::create_fleet()` directly
- ✅ Cleaner architecture: Player initiates and owns fleet creation, GameState provides services (fleet_id allocation, planet lookup)

**Benefits:**
- Single source of truth for fleet creation logic
- Clear ownership: Player owns Fleet objects and creation process
- Straightforward call flow: no back-and-forth between Player and GameState
- Easier to maintain and extend

#### 2. RNG Seed Naming Refactoring ✅
- ✅ Renamed `deterministicSeed` → `det_seed` (local variable)
- ✅ Renamed `aiSeed` → `ai_seed` (local variable)
- ✅ Renamed `get_deterministic_seed()` → `get_det_seed()`
- ✅ Renamed `set_deterministic_seed()` → `set_det_seed()`
- ✅ Renamed `get_ai_rng_seed()` → `get_ai_seed()`
- ✅ Renamed `set_ai_rng_seed()` → `set_ai_seed()`
- ✅ Updated C API function names accordingly
- ✅ More consistent snake_case naming throughout

#### 3. Research Cost Caching Optimization ✅
- ✅ Added six `std::vector<int64_t>` cache vectors to GameState (one per tech stream)
- ✅ Cache indexed directly by tech level: `research_cost_range[i]` = cost to reach level i
- ✅ Implemented `initialize_research_cost_caches()`:
  - Initializes with zeroes for starting levels (0 and 1)
  - Pre-calculates costs for levels 2-21 (20 levels ahead)
  - Reduces startup overhead
- ✅ Implemented `ensure_research_costs_available(max_tech_level)`:
  - Extends cache dynamically as players advance
  - Extends by 20 levels at a time
  - Prevents repeated calculations for high-level techs
- ✅ Updated `process_research_stream()`:
  - Removed repeated calls to `calculate_tech_*_advancement_cost()`
  - Now looks up costs directly from cache vectors
  - Calls `ensure_research_costs_available()` before advancement loop
  - Checks cache bounds and extends if needed within loop

**Benefits:**
- O(1) lookup replaces function call overhead
- Self-documenting: vector length shows max available tech level
- Automatically scales as tech levels increase
- Significant performance improvement for research-heavy turns

#### 4. Fleet/Knowledge Map Architecture Planning ✅
- ✅ Analyzed fleet storage options and their tradeoffs
- ✅ Decided against storing Fleet pointers in Planet (dangling pointer risk)
- ✅ Planned hybrid approach: Fleet IDs in Planet with safe getters
- ✅ Designed future Player Knowledge Map system:
  - Player maintains KnowledgePlanet structure for each real planet
  - Stores player's known info about that planet
  - Organizes player's own fleets by this knowledge structure
  - Stores enemy fleet visibility info (updated each turn)
  - Provides fast, UI-aligned queries
- ✅ Identified this as future refactoring (not immediate)
- ✅ Documented design for future implementation

**Architecture Decision:**
- **Immediate:** Simple Planet getters for fleet queries (safe, minimal complexity)
- **Future:** Player Knowledge Map system (solves fleet lookup + fog of war + UI alignment)

#### 5. Player Knowledge Map Architecture Design (Detailed) ✅

**Game Mechanics Context:**
- Players always have some information about all planets
- Initial knowledge (game start): name, position, planet_id
- Homeworld starts with full knowledge
- Knowledge expands through exploration and colonization
- All KnowledgePlanets initialized at game start (no gaps in vector)

**Data Structure:**
```cpp
class KnowledgePlanet {
    uint32_t real_planet_id;
    Planet* real_planet;  // Points to GameState's Planet (set at init, never changes)
    
    // Known information (initially limited, expands over time)
    std::string name;           // Always known
    double x, y;                // Position - always known
    double known_temperature;   // UNKNOWN initially, updated when explored
    double known_gravity;       // UNKNOWN initially, updated when explored
    int32_t known_metal;        // UNKNOWN initially, updated when explored
    // ... other properties
    
    // Fleet information
    std::vector<uint32_t> my_fleet_ids;           // Player's own fleets at this planet
    std::vector<FleetVisibleInfo> enemy_fleets;   // Enemy fleets (updated each turn)
    
    // Colonization (optional)
    std::unique_ptr<ColonizedPlanet> colonization;  // Non-null if player colonized this
};

class Player {
private:
    std::vector<KnowledgePlanet> knowledge_map;  // One per planet, indexed by planet_id
    
public:
    // Access knowledge planets
    KnowledgePlanet* get_knowledge_planet(uint32_t planet_id);
    const KnowledgePlanet* get_knowledge_planet(uint32_t planet_id) const;
    
    // Get colonized planets (filtered from knowledge map)
    std::vector<ColonizedPlanet*> get_colonized_planets();
    std::vector<const ColonizedPlanet*> get_colonized_planets() const;
    
    // Get specific colonized planet
    ColonizedPlanet* get_colonized_planet(uint32_t planet_id);
    
    // Fleet queries
    std::vector<Fleet*> get_colonized_planet_fleets(uint32_t planet_id, GameState* game_state);
    std::vector<Fleet*> get_all_fleets(GameState* game_state);
};
```

**Ownership Structure:**
```
GameState
  └─ Galaxy
      └─ std::vector<Planet>  (all planets in the galaxy)

Player
  └─ std::vector<KnowledgePlanet>  (player's knowledge of all planets)
      ├─ Planet* real_planet  (points to GameState's Planet)
      └─ std::unique_ptr<ColonizedPlanet>  (if colonized)
```

**Key Design Decisions:**
1. **Vector storage** - Direct indexing by planet_id, no unordered_map complexity
2. **Planet pointer** - Safe because planets are never destroyed mid-game
3. **Option 2 composition** - KnowledgePlanet contains optional ColonizedPlanet (not inheritance)
4. **Single source of truth** - All player knowledge in one structure
5. **No inheritance** - ColonizedPlanet is separate from Planet, not derived

**Initialization (at game start):**
- Create KnowledgePlanet for every planet in galaxy
- Set real_planet pointer, name, position
- Set initial knowledge to UNKNOWN for all properties except position
- For homeworld: call `update_knowledge_from_planet()` to populate full knowledge
- Create ColonizedPlanet for homeworld

**Knowledge Updates:**
- When exploring a planet: call `kp.update_knowledge_from_planet()`
- When colonizing: create ColonizedPlanet in the KnowledgePlanet
- Each turn: update enemy fleet visibility info

**Benefits:**
- ✅ Single source of truth for all player knowledge
- ✅ Natural representation: colonized planets are KnowledgePlanets with extra data
- ✅ UI-aligned: queries return exactly what UI needs
- ✅ Fog of war ready: knowledge structure supports incomplete information
- ✅ Fleet lookup: fast queries by planet
- ✅ No dangling pointers: Planet pointers are safe
- ✅ Clean iteration: simple vector iteration, no structured bindings needed

**Future Implementation Notes:**
- Migrate from current `std::vector<ColonizedPlanet>` storage
- Update all code that iterates colonized planets
- Add helper methods for common queries
- Implement fog of war rules during knowledge updates

**Recent Commits (Session 6):**
1. `e87708c` - Fix syntax error in allocate_fleet_id() function declaration
2. `f4d1a07` - Refactor fleet creation: Player now owns the process
3. `576c5bd` - Implement research cost caching to eliminate repeated calculations

---

**This document is the single source of truth for the OpenHo project. It should be updated after each significant session to reflect progress and maintain accuracy for future sessions.**


---

## Session 7: File Restructuring & KnowledgeGalaxy Implementation (January 5, 2026)

**Status:** ✅ COMPLETE

**Objective:** Reorganize codebase for clarity and implement KnowledgeGalaxy as the foundation for player galaxy knowledge system

### Part 1: File Restructuring & Class Promotion

**Completed:**
- ✅ Created `colonized_planet.h/cpp` - Moved ColonizedPlanet class to dedicated files
- ✅ Created `knowledge_planet.h/cpp` - Moved PlanetSnapshot (now KnowledgePlanet) to dedicated files
- ✅ Updated `planet.h/cpp` - Now contains only Planet class
- ✅ Promoted Planet to full class (was struct)
- ✅ Promoted PlanetSnapshot to full class (was struct)
- ✅ Updated all includes throughout codebase
- ✅ CMakeLists.txt updated with new source files

**Architecture Improvements:**
- Clear separation of concerns: Planet, ColonizedPlanet, KnowledgePlanet in separate files
- Better code organization for future maintenance
- Foundation ready for KnowledgePlanet feature development

### Part 2: PlanetSnapshot → KnowledgePlanet Refactoring

**Completed:**
- ✅ Added sentinel constants to enums.h:
  - `OBSERVATION_YEAR_UNKNOWN = -1`
  - `PROFITABILITY_UNKNOWN = -1`
  - `PERCEIVED_VALUE_UNKNOWN = -1`
- ✅ Refactored PlanetSnapshot constructor:
  - Converted `partial_info()` static method to default constructor
  - Constructor: `KnowledgePlanet(const Planet& planet, PlayerID player_id)`
  - Initializes with partial info (id, name, coordinates only)
  - All other fields set to appropriate unknown sentinel values
- ✅ Kept `full_info()` as factory method for complete information snapshots
- ✅ Added `nova_state` member (type: PlanetNovaState, initialized to PLANET_NORMAL)
- ✅ Added `observe_planet()` method:
  - Signature: `void observe_planet(const Planet& planet, const Player* observer, int32_t current_year)`
  - Updates observable fields from current planet state
  - Calculates apparent temperature and gravity based on observer's ideals
  - Updates observation_year to current_year
  - Does NOT modify nova_state
  - Uses `const Player*` for read-only access
- ✅ Resolved const member issue by removing const qualifiers:
  - Fields `id`, `name`, `x`, `y`, `as_seen_by` no longer const
  - Documented that these fields should not be modified after construction
  - Allows vector storage without assignment operator issues
- ✅ Renamed PlanetSnapshot → KnowledgePlanet throughout codebase
- ✅ Updated Player to use `std::vector<KnowledgePlanet> planetKnowledge`

**Key Design Decisions:**
1. **Default constructor for partial info** - More intuitive than static factory method
2. **Sentinel values at -1** - Consistent, easy to check for unknown state
3. **Non-const fields with documentation** - Pragmatic approach, relies on convention
4. **observe_planet() method** - Encapsulates knowledge update logic
5. **nova_state independence** - Not updated by observations, managed separately

### Part 3: KnowledgeGalaxy Implementation

**Completed:**
- ✅ Created `knowledge_galaxy.h` - KnowledgeGalaxy class definition
- ✅ Created `knowledge_galaxy.cpp` - KnowledgeGalaxy implementation
- ✅ Updated Player class:
  - Replaced `std::vector<KnowledgePlanet> planetKnowledge`
  - Added `KnowledgeGalaxy* knowledge_galaxy` member
  - Added include for knowledge_galaxy.h
- ✅ Updated CMakeLists.txt with knowledge_galaxy.cpp
- ✅ Updated openho_core.h to include knowledge_galaxy.h

**KnowledgeGalaxy Architecture:**

```cpp
class KnowledgeGalaxy {
private:
    const Galaxy* real_galaxy;  // Reference to real galaxy (for edge cases)
    std::vector<KnowledgePlanet> planets;  // Indexed by planet_id
    PlayerID player_id;

public:
    // Constructor - initializes with partial info for all planets
    KnowledgeGalaxy(const Galaxy& galaxy, PlayerID player_id);
    
    // Accessors
    KnowledgePlanet* get_planet(uint32_t planet_id);
    const KnowledgePlanet* get_planet(uint32_t planet_id) const;
    
    size_t get_planet_count() const { return planets.size(); }
    
    // Update player's knowledge of a planet
    void observe_planet(uint32_t planet_id, const Planet& real_planet, 
                       const Player* observer, int32_t current_year);
    
    // Access to real galaxy (for edge cases)
    const Planet* get_real_planet(uint32_t planet_id) const;
};
```

**Key Features:**
1. **Self-sufficient validation** - Uses player's knowledge for most game logic
2. **Reference to real galaxy** - Available for edge cases, not primary validation path
3. **Vector storage** - Direct indexing by planet_id for O(1) access
4. **Initialization at game start** - Creates KnowledgePlanets for all planets with partial info
5. **Extensible design** - Foundation for fog of war and future game logic

**Ownership Structure:**
```
GameState
  └─ Galaxy
      └─ std::vector<Planet>  (all planets in the galaxy)

Player
  └─ KnowledgeGalaxy
      └─ std::vector<KnowledgePlanet>  (player's knowledge of all planets)
          ├─ const Galaxy* real_galaxy  (reference, for edge cases)
          └─ KnowledgePlanet[i]
              ├─ Planet* real_planet  (points to GameState's Planet)
              ├─ Observable fields (temperature, gravity, metal, etc.)
              ├─ Fleet information (my_fleet_ids, enemy_fleets)
              └─ Optional ColonizedPlanet (if colonized)
```

### Session 7 Commits:

1. `8243611` - Rename PlanetSnapshot to KnowledgePlanet
   - Complete rename throughout codebase
   - Updated class definitions, implementations, and references
   
2. `b22fdb9` - Implement KnowledgeGalaxy class for player galaxy knowledge
   - Created knowledge_galaxy.h/cpp
   - Updated Player to use KnowledgeGalaxy
   - Updated build system and includes

### Updated Future Plans:

**Next Steps (Phase 2d - KnowledgePlanet Integration):**
1. [ ] Integrate KnowledgeGalaxy initialization into GameState constructor
2. [ ] Initialize homeworld with full knowledge
3. [ ] Implement initial knowledge updates for all players
4. [ ] Add helper methods to Player for common KnowledgeGalaxy queries
5. [ ] Update game logic to use KnowledgeGalaxy for planet queries
6. [ ] Implement fog of war rules for knowledge updates
7. [ ] Add observation mechanics (exploration, colonization, etc.)
8. [ ] Implement fleet visibility tracking in KnowledgePlanet

**Phase 2e - Future Enhancements:**
- [ ] Implement fog of war based on fleet positions
- [ ] Add sensor range calculations for fleet observation
- [ ] Implement knowledge decay/staleness tracking
- [ ] Add knowledge sharing between allied players
- [ ] Implement espionage/scouting mechanics

### Benefits of Current Implementation:

✅ **Clear architecture** - Separate files for each concept (Planet, ColonizedPlanet, KnowledgePlanet)  
✅ **Self-sufficient player view** - KnowledgeGalaxy provides most validation independently  
✅ **Extensible design** - Foundation for fog of war and game logic  
✅ **Type safety** - Full classes instead of structs provide better encapsulation  
✅ **Maintainability** - Well-organized codebase with clear responsibilities  
✅ **Performance** - Vector indexing provides O(1) access by planet_id  
✅ **Code compiles successfully** - All changes integrated and tested

