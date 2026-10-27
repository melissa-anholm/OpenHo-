# OpenHo Project Implementation Log

This document records chronological development sessions, completed implementation work, architectural decisions, and outstanding implementation items. It is a historical working log; the current roadmap and implementation status are maintained separately in `Master_Plan.md`, `Missing_Implementation_List.md`, and `Source_Audit_2026Oct23.md`.

## Session 7 - Distance Matrix Implementation

### Distance Matrix Caching (Session 7 - Current)
- [x] Implement distance matrix in Galaxy class
  - [x] Add `distance_matrix` member (2D vector)
  - [x] Add `compute_distance_matrix()` method
  - [x] Add `get_distance(from_id, to_id)` method
  - [x] Call `compute_distance_matrix()` in Galaxy constructor
  - [x] Use Euclidean distance formula with rounding to integer
  
- [x] Implement distance matrix in KnowledgeGalaxy
  - [x] Add `distance_matrix` member (local copy)
  - [x] Copy distance matrix from Galaxy in constructor
  - [x] Add `get_distance(from_id, to_id)` method
  - [x] Provide O(1) local access with no network latency
  
- [x] Verify compilation
  - [x] Project compiles successfully
  - [x] No new errors introduced

### Future Work

#### Phase 2d - KnowledgePlanet Integration (Session 8 - Current)
- [x] Integrate KnowledgeGalaxy initialization into GameState constructor
- [x] Initialize homeworld with full knowledge
- [x] Implement initial knowledge updates for all players
- [ ] Add helper methods to Player for common KnowledgeGalaxy queries
- [ ] Update game logic to use KnowledgeGalaxy for planet queries
- [ ] Implement fog of war rules for knowledge updates
- [ ] Add observation mechanics (exploration, colonization, etc.)
- [ ] Implement fleet visibility tracking in KnowledgePlanet

#### Phase 2e - Future Enhancements
- [ ] Implement fog of war based on fleet positions
- [ ] Add sensor range calculations for fleet observation
- [ ] Implement knowledge decay/staleness tracking
- [ ] Add knowledge sharing between allied players
- [ ] Implement espionage/scouting mechanics

#### Phase 3 - UI Integration
- [ ] Create distance query UI components
- [ ] Implement route planning using distance matrix
- [ ] Add distance display in planet information panels
- [ ] Create fleet movement UI using distance calculations

#### Phase 4 - Testing & Optimization
- [ ] Write unit tests for distance calculations
- [ ] Verify distance matrix accuracy
- [ ] Profile memory usage for various galaxy sizes
- [ ] Optimize distance matrix if needed

## Architecture Notes

### Distance Matrix Design
- **Storage**: 2D vector `std::vector<std::vector<double>>` in both Galaxy and KnowledgeGalaxy
- **Calculation**: Euclidean distance `sqrt((x2-x1)² + (y2-y1)²)` rounded to nearest integer
- **Access Pattern**: O(1) lookup via `get_distance(from_id, to_id)`
- **Initialization**: Computed once in Galaxy constructor, copied to each player's KnowledgeGalaxy
- **Network Impact**: Zero network latency for distance queries (all local lookups)

### Key Benefits
✅ O(1) distance lookups for UI responsiveness  
✅ No network latency for common queries  
✅ Single source of truth in Galaxy  
✅ Each player has independent copy for self-sufficiency  
✅ Euclidean distance with integer rounding for consistency  



---

## Session 8 - KnowledgeGalaxy Initialization (January 5, 2026)

**Status:** ✅ COMPLETE

**Objective:** Initialize KnowledgeGalaxy for each player in GameState constructor with homeworld having full knowledge and ColonizedPlanet.

### Changes Made:

**1. KnowledgePlanet Enhancement**
- Added `colonization` member: `std::unique_ptr<ColonizedPlanet>`
- Added `#include <memory>` to knowledge_planet.h
- Allows KnowledgePlanet to own colonization data for colonized planets

**2. GameState Initialization**
- Added `initialize_player_knowledge()` method to GameState
- Called from GameState constructor after galaxy initialization
- For each player:
  - Creates KnowledgeGalaxy with reference to Galaxy
  - Finds homeworld from colonized_planets
  - Calls `observe_planet()` to populate full knowledge
  - Creates ColonizedPlanet in homeworld's KnowledgePlanet

**3. Knowledge State at Game Start**
- **Homeworld**: Full knowledge (all fields populated via observe_planet)
- **Homeworld Colonization**: ColonizedPlanet created with population and income
- **Other Planets**: Minimal info (id, name, coordinates only)
- **Network Impact**: All queries are local, no network latency

### Implementation Details:

```cpp
void GameState::initialize_player_knowledge()
{
    for (auto& player : players)
    {
        // Create KnowledgeGalaxy for this player
        player.knowledge_galaxy = new KnowledgeGalaxy(*galaxy, player.id);
        
        // Find homeworld and observe it
        if (!player.colonized_planets.empty())
        {
            const ColonizedPlanet& homeworld_colonized = player.colonized_planets[0];
            Planet* homeworld = get_planet(homeworld_colonized.get_id());
            
            // Populate full knowledge of homeworld
            player.knowledge_galaxy->observe_planet(homeworld->id, *homeworld, &player, current_year);
            
            // Create ColonizedPlanet in KnowledgePlanet
            KnowledgePlanet* homeworld_knowledge = player.knowledge_galaxy->get_planet(homeworld->id);
            if (homeworld_knowledge)
            {
                homeworld_knowledge->colonization = std::make_unique<ColonizedPlanet>(
                    homeworld,
                    &player,
                    homeworld_colonized.get_population(),
                    homeworld_colonized.get_income()
                );
            }
        }
    }
}
```

### Compilation:
✅ Project compiles successfully  
✅ No new errors introduced  
✅ Only pre-existing warnings about unused parameters  

### Architecture Benefits:

✅ **Self-sufficient players** - Each player has complete KnowledgeGalaxy  
✅ **Organic fog of war** - Emerges from observation logic, not artificial rules  
✅ **Homeworld knowledge** - Full info available at game start  
✅ **Minimal other planets** - Unknown planets have only basic info  
✅ **Colonization tracking** - KnowledgePlanet owns ColonizedPlanet for owned planets  
✅ **No network latency** - All queries are local lookups  

### Next Steps (Phase 2d):
- Add helper methods to Player for common KnowledgeGalaxy queries
- Update game logic to use KnowledgeGalaxy for planet queries
- Implement observation mechanics (exploration, colonization, etc.)
- Implement fleet visibility tracking in KnowledgePlanet



---

## Session 9 - KnowledgePlanet & Planet Refactoring (January 5, 2026)

**Status:** ✅ COMPLETE

**Objective:** Refactor KnowledgePlanet and Planet classes, remove player_planets from GameState, and prepare for turn processing with unified colonized planets list.

### Changes Made:

**1. KnowledgePlanet Refactoring**
- Removed `income` member (access via ColonizedPlanet if owned)
- Renamed `population` → `apparent_population`
- Set `apparent_population = -1` as sentinel for "unknown"
- Rationale: Only relevant for unowned planets; owned planets use ColonizedPlanet

**2. Planet Enhancement**
- Added `population` member to Planet class
- Initialized to 0 in constructor (default for uncolonized planets)
- Set to starting population for homeworlds in assign_planets_random()
- Provides source of truth for KnowledgePlanet::apparent_population

**3. GameState Cleanup**
- Removed `player_planets` map (was unused)
- Removed `get_player_planets()` method (never called)
- Removed updates from `build_entity_maps()`
- Removed updates from `assign_planets_random()`
- Rationale: Redundant with Player::colonized_planets

**4. KnowledgePlanet Initialization**
- Constructor sets `apparent_population = -1` (unknown)
- `observe_planet()` copies `planet.population` to `apparent_population`
- Homeworld gets full population knowledge at game start

**5. Turn Processing Foundation**
- Designed for lazy construction of colonized planets list during turn processing
- Each turn: iterate players' colonized_planets to build temporary list
- Advantages: Always correct, no maintenance burden, self-contained

### Implementation Details:

**KnowledgePlanet Constructor:**
```cpp
KnowledgePlanet::KnowledgePlanet(const Planet& planet, PlayerID player_id)
{
    // ... other fields ...
    apparent_population = -1;  // -1 means unknown
}
```

**KnowledgePlanet::observe_planet():**
```cpp
void KnowledgePlanet::observe_planet(const Planet& planet, const Player* observer, int32_t current_year)
{
    // ... other updates ...
    apparent_population = planet.population;  // Copy actual population
}
```

**Homeworld Population Setup:**
```cpp
// In assign_planets_random()
planet->population = GameConstants::Starting_Colony_Population[setup.starting_colony_quality];
```

### Architecture Benefits:

✅ **Cleaner separation** - Income only in ColonizedPlanet, population in both  
✅ **Consistent naming** - apparent_* for observed values  
✅ **Unknown sentinel** - -1 clearly indicates unobserved population  
✅ **Source of truth** - Planet::population is authoritative  
✅ **Removed redundancy** - No more player_planets map  
✅ **Turn processing ready** - Foundation for lazy list construction  
✅ **Compilation** - No errors, only pre-existing warnings  

### Investigation Results:

**player_planets Analysis:**
- Declared but never called (get_player_planets was unused)
- Built during initialization but then sat idle
- Redundant with Player::colonized_planets
- Would require maintenance on colonization/conquest
- **Decision:** Remove entirely

**Unified Colonized Planets List:**
- Will be built fresh during each turn processing
- Guarantees correctness (no stale data)
- Simplifies maintenance (single source of truth)
- Performance negligible (O(n) rebuild in O(n) process)

### Next Steps (Phase 2d):

The foundation is now in place for:
- Implementing turn processing with unified colonized planets list
- Updating populations during turn processing
- Calculating income for all colonized planets
- Adding observation mechanics for population discovery
- Implementing fog of war based on observation logic

**Commit:** `<pending>` - Refactor KnowledgePlanet and Planet, remove player_planets



---

## Session 10 - Fleet Visibility in KnowledgePlanet (January 5, 2026)

**Status:** ✅ COMPLETE

**Objective:** Implement fleet visibility tracking in KnowledgePlanet with storage for player's own fleets and visible enemy fleets.

### Changes Made:

**1. FleetVisibleInfo Struct**
- Created new struct for representing visible enemy fleets
- Fields: fleet_id, owner (PlayerID), ship_count
- TODO: Add more observable properties (design type, fuel level, etc.)
- Placed in knowledge_planet.h for easy access

**2. KnowledgePlanet Enhancement**
- Added `my_fleets` vector: `std::vector<Fleet*>`
  - Stores player's own fleets stationed at this planet
  - Direct Fleet pointers for quick access
- Added `enemy_fleets` vector: `std::vector<FleetVisibleInfo>`
  - Stores visible enemy fleets at this planet
  - Uses FleetVisibleInfo struct for observable data

**3. Fleet Management Methods**
- `add_my_fleet(Fleet* fleet)` - Add player's own fleet with duplicate prevention
- `remove_my_fleet(uint32_t fleet_id)` - Remove player's own fleet by ID
- `add_visible_enemy_fleet(FleetVisibleInfo)` - Add visible enemy fleet with duplicate prevention
- `remove_visible_enemy_fleet(uint32_t fleet_id)` - Remove visible enemy fleet by ID
- `clear_enemy_fleets()` - Clear all enemy fleets (for turn updates)

**4. Implementation Details**
- Used std::find_if for safe removal by fleet_id
- Duplicate prevention in add methods
- Empty vectors initialized automatically in constructor
- Added necessary includes: `#include <vector>` and `#include <algorithm>`

### Design Rationale:

**Why Option A (Methods in KnowledgePlanet)?**
- Most natural: Fleets are associated with planets
- Encapsulated: KnowledgePlanet manages its own fleet data
- Cleaner: No need for external visibility update logic
- UI-aligned: Queries return exactly what UI needs

**Why Store Fleet Pointers?**
- Direct access to Fleet objects (not just IDs)
- Efficient lookups for fleet information
- Safe: Planets are never destroyed mid-game
- Avoids indirection through GameState

**Why FleetVisibleInfo for Enemy Fleets?**
- Encapsulates observable properties only
- Prevents accidental access to non-observable data
- Extensible: Easy to add more observable properties
- Type-safe: Separate struct prevents confusion

### Architecture Benefits:

✅ **Natural organization** - Fleets grouped by their location  
✅ **Fog of war ready** - Only visible fleets stored  
✅ **UI-aligned** - Queries return exactly what UI needs  
✅ **Single source of truth** - All fleet knowledge in KnowledgePlanet  
✅ **No dangling pointers** - Fleet pointers are safe  
✅ **Efficient lookups** - O(n) where n = fleets at planet (typically small)  
✅ **Compilation** - No errors, only pre-existing warnings  

### Future Implementation (Not Yet Done):

**Phase 1: Fleet Movement**
- [ ] Implement process_ships() to handle fleet movement
- [ ] Move fleets from origin_planet to destination_planet
- [ ] Update KnowledgePlanet fleet lists when fleets move

**Phase 2: Visibility Updates**
- [ ] Update fleet visibility during turn processing
- [ ] Clear old visibility data at start of turn
- [ ] Add fleets to KnowledgePlanets based on current positions
- [ ] Implement sensor range calculations

**Phase 3: Sensor Range**
- [ ] Calculate sensor range based on tech level
- [ ] Only show enemy fleets within sensor range
- [ ] Track fleets in transit if visible

### Compilation:
✅ Project compiles successfully  
✅ No new errors introduced  
✅ Only pre-existing warnings about unused parameters  

### Next Steps (Phase 2d):
- Implement process_ships() to handle fleet movement
- Update KnowledgePlanet fleet lists when fleets move
- Implement fleet visibility updates during turn processing
- Add sensor range calculations for visibility rules
- Track fleets in transit (if visible)

**Commit:** `<pending>` - Implement fleet visibility tracking in KnowledgePlanet


---

## Session 11 - Fleet Movement Refactoring (January 5, 2026)

**Status:** ✅ COMPLETE

**Objective:** Move fleet movement logic from GameState::move_fleet() to Fleet::move_to() method, and integrate distance matrix for accurate distance calculations.

### Changes Made:

**1. Fleet Class Enhancement**
- Added `move_to(Planet* destination, const KnowledgeGalaxy* knowledge_galaxy)` method
- Moved all movement setup logic from GameState to Fleet
- Fleet now owns its own movement logic
- Added forward declaration for KnowledgeGalaxy in fleet.h

**2. Fleet::move_to() Implementation**
- Validates inputs (destination, knowledge_galaxy, current_planet)
- Checks if already at destination
- Sets up movement state:
  - `origin_planet = current_planet`
  - `destination_planet = destination`
  - `current_planet = nullptr`
  - `in_transit = true`
- **Uses distance matrix** for distance calculation:
  - `distance = knowledge_galaxy->get_distance(origin_id, destination_id)`
  - Replaces manual Euclidean distance calculation
- Calculates turns based on ship speed:
  - `turns_to_destination = ceil(distance / ship_speed)`

**3. GameState::move_fleet() Refactoring**
- Simplified to validation and delegation
- Gets fleet, destination planet, and player
- Calls `fleet->move_to(destination, player->knowledge_galaxy)`
- Removed all manual distance calculations
- Much cleaner and more maintainable

**4. Added Necessary Includes**
- `#include "knowledge_galaxy.h"` in fleet.cpp
- `#include <cmath>` in fleet.cpp (for std::ceil)

### Design Rationale:

**Why Move to Fleet Class?**
- ✅ Fleet should own its movement logic
- ✅ More object-oriented design
- ✅ Cleaner separation of concerns
- ✅ Easier to test fleet movement independently

**Why Use Distance Matrix?**
- ✅ Avoids duplicate distance calculations
- ✅ Uses pre-computed, accurate distances
- ✅ O(1) lookup instead of manual calculation
- ✅ Consistent with game design

**Why Pass KnowledgeGalaxy?**
- ✅ Fleet has access to player's distance matrix
- ✅ Avoids passing GameState around
- ✅ Cleaner API
- ✅ Fleet can operate independently

### Architecture Benefits:

✅ **Better encapsulation** - Fleet owns its movement logic  
✅ **Cleaner GameState** - Less responsibility in GameState  
✅ **Distance matrix integration** - Uses pre-computed distances  
✅ **More testable** - Fleet movement can be tested independently  
✅ **Consistent API** - Fleet methods for fleet operations  
✅ **No code duplication** - Single source of movement logic  

### Code Comparison:

**Before (GameState::move_fleet):**
```cpp
// 40+ lines of manual distance calculation
double dx = destination->x - origin->x;
double dy = destination->y - origin->y;
distance = std::sqrt(dx * dx + dy * dy);
```

**After (Fleet::move_to):**
```cpp
// 1 line using distance matrix
double distance = knowledge_galaxy->get_distance(origin_id, destination_id);
```

### Compilation:
✅ Project compiles successfully  
✅ No new errors introduced  
✅ Only pre-existing warnings  

### Next Steps (Future):

**Immediate:**
- [ ] Implement process_ships() to move fleets when turns_to_destination reaches 0
- [ ] Update KnowledgePlanet fleet lists when fleets move
- [ ] Handle fleet arrival at destination

**Future:**
- [ ] Implement fuel consumption during travel
- [ ] Add waypoint/multi-leg journey support
- [ ] Implement fleet interception mechanics
- [ ] Add fleet combat in transit

**Commit:** `<pending>` - Refactor fleet movement to Fleet::move_to() with distance matrix

---

## Master Plan Detail Relocated (October 27, 2026)

The following material was formerly embedded in `Master_Plan.md`. It is preserved here as historical implementation detail so that the Master Plan can remain focused on current direction and high-level milestone status. The status claims below reflect the historical document state at the time they were written and should not override the current backlog or source audits.

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

### Recent Commits

**Session 14 (Jan 26, 2026) - Galaxy Shapes & Poisson Sampling:**
1. `3d32985` - Rename select_home_planet_coordinates to select_home_planets_random
2. `b56d59c` - Implement Poisson disk sampling and Circle/Ring galaxy generators

**Session 13 (Jan 26, 2026) - Economic Mechanics & Code Refactoring:**
1. `bce41d5` - Rename temperature offset constants for clarity
2. `7894d22` - Add OFFSET_K_TO_F constant and optimize K-F conversions
3. `5955b2f` - Refactor terraforming and mining into separate functions
