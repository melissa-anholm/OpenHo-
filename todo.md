# OpenHo Project TODO

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
