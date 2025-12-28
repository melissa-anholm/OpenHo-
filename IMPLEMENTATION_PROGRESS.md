# OpenHo C++ Core Implementation Progress

## Phase 2: Design and Construct Deterministic C++ Core and API (C++17)

### Status: FOUNDATION LAYER COMPLETE

This document tracks the implementation progress of the OpenHo C++ core engine.

---

## Completed: Foundation Layer

### 1. Project Structure
- ✅ Created `/src/core/` directory structure with `include/` and `src/` subdirectories
- ✅ Set up CMakeLists.txt with C++17 standard and Boost.Random dependency

### 2. Header Files (API Definitions)

#### `openho_core.h` - Main API Header
- ✅ Fundamental types: `EntityID`, `PlayerID`, `GalaxyCoord`
- ✅ Enums: `ShipType`, `PlanetState`, `PlayerType`
- ✅ Data structures:
  - `TechnologyLevels` - Player technology research levels
  - `ShipTechParameters` - Ship-specific tech parameters
  - `PlanetAllocation` - Per-planet money allocation
  - `MoneyAllocation` - Complete player money allocation (global + per-planet)
  - `PlanetInfo` - Player's knowledge of a planet
  - `Planet` - Actual planet state (host-side)
  - `Ship` - Ship structure with movement state
  - `Player` - Player structure with resources and technology
  - `GalaxyGenerationParams` - Parameters for galaxy generation
  - `Galaxy` - Main galaxy structure
- ✅ C API functions (extern "C" for Objective-C++ bridging):
  - Game initialization: `game_create()`, `game_destroy()`
  - Game state queries: `game_get_current_turn()`, `game_get_num_players()`, etc.
  - Player queries: `game_get_player()`, `game_get_player_money_income()`
  - Planet queries: `game_get_planet()`, `game_get_planet_perceived_values()`
  - Ship queries: `game_get_ship()`
  - Money allocation: `game_set_money_allocation()`, `game_get_money_allocation()`
  - AI RNG: `game_get_ai_rng_seed()`, `game_set_ai_rng_seed()`
  - Turn processing: `game_process_turn()`
  - Serialization: `game_serialize_state()`, `game_deserialize_state()`

#### `rng.h` - Deterministic RNG Class
- ✅ `DeterministicRNG` class with dual RNG engines:
  - `deterministicRNG`: For shared game mechanics (deterministic across all players)
  - `aiRNG`: For AI decisions (seeded each turn by the host)
- ✅ Methods for unbounded random numbers:
  - `nextInt32()`, `nextUInt32()`, `nextInt64()`, `nextUInt64()`, `nextDouble()`
  - `nextAIInt32()`, `nextAIUInt32()`, `nextAIInt64()`, `nextAIUInt64()`, `nextAIDouble()`
- ✅ Methods for bounded random numbers:
  - `nextInt32Range()`, `nextUInt32Range()`, `nextDoubleRange()`
  - `nextAIInt32Range()`, `nextAIUInt32Range()`, `nextAIDoubleRange()`
- ✅ Seed management: `setDeterministicSeed()`, `setAISeed()`, `getAISeed()`

#### `game.h` - GameState Class
- ✅ `GameState` class for managing game state:
  - Galaxy management
  - RNG management
  - Entity access (players, planets, ships)
  - Turn processing
  - Money allocation
  - AI RNG seed management
  - Serialization interface
- ✅ Private helper methods for turn processing:
  - `calculatePlayerIncomes()`
  - `updatePlanetIncomes()`
  - `processMoneyAllocation()`
  - `processResearch()`
  - `processTerriforming()`
  - `processMining()`
  - `processShips()`
  - `processNovae()`

### 3. Implementation Files

#### `rng.cpp` - RNG Implementation
- ✅ `DeterministicRNG` constructor and destructor
- ✅ All RNG methods implemented using Boost.Random (mt19937_64)
- ✅ Bounded random number generation with automatic min/max swapping
- ✅ Separate engines for deterministic and AI randomness

#### `game.cpp` - GameState Implementation
- ✅ Constructor with galaxy initialization
- ✅ Destructor
- ✅ All accessor methods (getGalaxy, getRNG, getPlayer, getPlanet, getShip)
- ✅ Money allocation methods
- ✅ AI RNG seed management
- ✅ Turn processing framework (calls to all sub-systems)
- ✅ Serialization interface (placeholder)
- ✅ Private helper methods:
  - `initializePlanets()` - Creates 10 test planets with random properties
  - `initializePlayers()` - Placeholder for player setup
  - `buildEntityMaps()` - Creates ID-to-index maps for fast lookup
  - `calculatePlayerIncomes()` - Calculates income from owned planets
  - `updatePlanetIncomes()` - Calculates planet income based on owner's ideals
  - All turn processing sub-systems (placeholders for now)

#### `types.cpp` - Type Utilities
- ✅ Created as placeholder for future type utility functions

#### Placeholder Implementation Files
- ✅ `simulation.cpp` - Simulation module placeholder
- ✅ `commands.cpp` - Commands module placeholder
- ✅ `money_allocation.cpp` - Money allocation module placeholder
- ✅ `planet_mechanics.cpp` - Planet mechanics module placeholder
- ✅ `serialization.cpp` - Serialization module placeholder
- ✅ `ai_interface.cpp` - AI interface (C API wrapper) placeholder

---

## Next Steps: Core Logic Implementation

### Phase 2a: Complete C API Wrapper (ai_interface.cpp)
- [ ] Implement `game_create()` - Create GameState instance and return as void*
- [ ] Implement `game_destroy()` - Delete GameState instance
- [ ] Implement all query functions (game_get_*)
- [ ] Implement `game_process_turn()` wrapper
- [ ] Implement serialization functions

### Phase 2b: Complete Turn Processing Logic
- [ ] Implement `processMoneyAllocation()` - Distribute player money
- [ ] Implement `processResearch()` - Advance research progress
- [ ] Implement `processTerriforming()` - Apply terraforming effects
- [ ] Implement `processMining()` - Extract metal resources
- [ ] Implement `processShips()` - Move ships, resolve combat, etc.
- [ ] Implement `processNovae()` - Handle nova warnings and destruction

### Phase 2c: Serialization
- [ ] Implement `serializeState()` - Convert GameState to binary format
- [ ] Implement `deserializeState()` - Restore GameState from binary format
- [ ] Ensure deterministic serialization for multiplayer sync

### Phase 2d: Testing
- [ ] Unit tests for RNG (verify determinism)
- [ ] Unit tests for GameState initialization
- [ ] Unit tests for turn processing
- [ ] Integration tests for multiplayer synchronization

### Phase 3: Objective-C++ Bridging
- [ ] Create Objective-C++ wrapper classes
- [ ] Set up Xcode project with Swift/C++ integration
- [ ] Create SwiftUI UI components
- [ ] Connect C++ core to SwiftUI via Objective-C++ bridge

---

## Key Design Decisions

### Dual RNG Engines
- **Deterministic RNG**: Used for shared game mechanics (planet generation, ship movement, etc.)
  - Ensures all players see the same sequence of events
  - Seeded once at game start
  - Cannot be changed during gameplay
  
- **AI RNG**: Used for AI decisions (ship orders, research choices, etc.)
  - Seeded each turn by the host
  - Allows AI behavior to vary without affecting core mechanics
  - Enables host migration (new host gets AI seed from previous host)

### Entity ID Mapping
- Uses `unordered_map` for O(1) lookup of entities by ID
- Maps built during initialization and updated as entities are created/destroyed
- Enables efficient access to game entities from C API

### Money Allocation Structure
- **Unified `MoneyAllocation` struct** containing:
  - Global allocations (savings, research for each tech)
  - Per-planet allocations (terraforming, mining for each planet)
- Simplifies allocation processing and serialization

### Planet Income Calculation
- Based on population and happiness
- Happiness calculated from proximity to owner's ideal temperature and gravity
- Incentivizes players to terraform planets to their preferences

---

## Technical Notes

### Boost.Random Usage
- Using `mt19937_64` (64-bit Mersenne Twister) for high-quality randomness
- Deterministic: same seed produces same sequence on all platforms
- Supports both bounded and unbounded random number generation

### C API Design
- All functions use `void*` for opaque game state pointer
- Enables Objective-C++ bridging without exposing C++ types to Swift
- Error handling via return codes (TBD)

### Memory Management
- `GameState` uses `std::unique_ptr` for RNG (automatic cleanup)
- Entity containers (planets, ships, players) use `std::vector`
- ID-to-index maps use `std::unordered_map` for fast lookup

---

## File Structure

```
/home/ubuntu/OpenHo/
├── src/core/
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── openho_core.h          # Main API header
│   │   ├── rng.h                  # RNG class definition
│   │   └── game.h                 # GameState class definition
│   └── src/
│       ├── rng.cpp                # RNG implementation
│       ├── game.cpp               # GameState implementation
│       ├── types.cpp              # Type utilities
│       ├── simulation.cpp          # Simulation module (placeholder)
│       ├── commands.cpp            # Commands module (placeholder)
│       ├── money_allocation.cpp    # Money allocation (placeholder)
│       ├── planet_mechanics.cpp    # Planet mechanics (placeholder)
│       ├── serialization.cpp       # Serialization (placeholder)
│       └── ai_interface.cpp        # C API wrapper (placeholder)
├── CORE_DESIGN.md                 # Comprehensive design document
└── IMPLEMENTATION_PROGRESS.md     # This file
```

---

## Compilation

To build the C++ core:

```bash
cd /home/ubuntu/OpenHo
mkdir -p build
cd build
cmake ..
make
```

Requirements:
- CMake 3.10+
- C++17 compiler (GCC, Clang, or MSVC)
- Boost library with Random component

---

## Next Session

Continue with Phase 2a: Complete the C API wrapper functions in `ai_interface.cpp` to enable Objective-C++ bridging.
