# OpenHo Master Plan

**Last Updated:** December 31, 2025  
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

## Technical Architecture

### Overall Structure

```
OpenHo/
├── src/core/                          # Phase 2: C++ Game Engine
│   ├── include/                       # Public headers
│   │   ├── openho_core.h              # Main C API
│   │   ├── game.h                     # GameState class
│   │   ├── game_formulas.h            # Game calculations
│   │   ├── rng.h                      # Deterministic RNG
│   │   ├── money_allocation_calculator.h
│   │   └── ai_interface.h
│   ├── src/                           # Implementation
│   │   ├── game.cpp
│   │   ├── game_formulas.cpp
│   │   ├── rng.cpp
│   │   ├── ai_interface.cpp
│   │   ├── money_allocation_calculator.cpp
│   │   ├── simulation.cpp
│   │   ├── commands.cpp
│   │   ├── money_allocation.cpp
│   │   ├── planet_mechanics.cpp
│   │   ├── serialization.cpp
│   │   └── types.cpp
│   ├── build/                         # CMake build artifacts (not committed)
│   └── CMakeLists.txt
│
├── src/macos/                         # Phase 3a: Objective-C++ Bridge (Future)
│   ├── include/
│   │   ├── OpenHoMacOS.h
│   │   └── GameBridge.h
│   ├── src/
│   │   ├── OpenHoMacOS.mm
│   │   └── GameBridge.mm
│   ├── build/                         # CMake build artifacts (not committed)
│   └── CMakeLists.txt
│
├── macos-app/                         # Phase 3b: SwiftUI App (Future)
│   ├── Package.swift                  # Swift Package Manager manifest
│   ├── Sources/OpenHo/
│   │   ├── main.swift
│   │   ├── ContentView.swift
│   │   ├── GameViewController.swift
│   │   ├── Models/
│   │   ├── Views/
│   │   └── ...
│   ├── Tests/
│   └── .build/                        # SPM build artifacts (not committed)
│
├── prev_Mr_Twave/                     # Archive of previous implementation (DO NOT MODIFY)
│
├── Documentation/
│   ├── MASTER_PLAN.md                 # This file - single source of truth
│   ├── PROJECT_STRUCTURE.md           # Directory layout and phases
│   ├── BUILD_SYSTEM_PLAN.md           # Build architecture and scripts
│   ├── IMPLEMENTATION_PROGRESS.md     # Detailed phase status
│   └── README.md                      # User-facing project description
│
├── Build Scripts/
│   ├── build.sh                       # Master build script
│   └── clean.sh                       # Cleanup script
│
└── Version Control/
    ├── .gitignore                     # Ignore build artifacts
    └── (git history)
```

### Naming Conventions

- **Classes/Structs:** PascalCase (e.g., `GameState`, `ColonizedPlanet`)
- **Functions/Methods:** snake_case (e.g., `process_turn()`, `calculate_income()`)
- **Variables/Members:** snake_case (e.g., `current_turn`, `player_money`)
- **Constants:** UPPER_SNAKE_CASE (e.g., `MAX_PLAYERS`)

### Key Design Decisions

1. **Dual RNG Engines:**
   - `deterministicRNG`: Shared game mechanics (reproducible across all players)
   - `aiRNG`: AI decisions (seeded each turn by host, allows variation)

2. **Nested MoneyAllocation:**
   - `MoneyAllocation` struct nested within `Player` class
   - Contains global allocations (research, savings) and per-planet allocations (terraforming, mining)

3. **ColonizedPlanet Architecture (Refactored):**
   - No longer inherits from Planet (pointer-based composition)
   - Holds `Planet*` pointer to base planet (not owned)
   - Holds `Player*` pointer to owner player
   - Adds player-specific data: `population`, `income`, `allocation`
   - Calculates `apparent_gravity` and `apparent_temperature` based on player ideals
   - Owned directly by `Player::colonized_planets` vector

4. **Consolidated Planet Processing:**
   - Single `process_planets()` method handles terraforming and mining
   - Operates on `player.colonized_planets` vector

5. **C API Interface:**
   - All functions use `void*` for opaque game state pointer
   - Enables Objective-C++ bridging without exposing C++ types to Swift
   - Clean separation between core engine and UI layers

6. **Planet Identification:**
   - Planet name is the primary identifier (used for lookups)
   - Planet ID is assigned by GameState but secondary to name
   - Both are required and assigned together at planet creation

7. **Sentinel Values:**
   - `UNKNOWN_DOUBLE_VALUE = -500.0` for unknown double fields
   - `UNKNOWN_INT_VALUE = -500` for unknown int32_t fields
   - `INCOME_UNKNOWN = INT32_MIN` for unknown income
   - `NOT_OWNED = 0` for unowned planets
   - `OWNER_UNKNOWN = UINT32_MAX` for unknown owner
   - Player IDs must never be 0 (reserved for NOT_OWNED)

8. **PlanetSnapshot:**
   - Represents a player's knowledge of a planet at a specific observation time
   - Two factory methods: `partial_info()` and `full_info()`
   - Tracks which player created the snapshot (`as_seen_by`)
   - Stores apparent values (temperature, gravity) based on observer's ideals

---

## Development Phases

### Phase 2: C++ Core Implementation (Current)

**Status:** Foundation layer complete, core logic in progress

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

**In Progress:**
- [ ] Initialize planetKnowledge vector for all players at game start
- [ ] Implement planet observation system (update PlanetSnapshot when observed)
- [ ] Complete C API wrapper implementation (ai_interface.cpp)
- [ ] Implement turn processing logic:
  - [ ] Money allocation distribution
  - [ ] Research advancement
  - [ ] Terraforming effects
  - [ ] Mining extraction
  - [ ] Ship movement and combat
  - [ ] Nova handling
- [ ] Save/Load game state (binary or JSON format)
- [ ] Serialization system for multiplayer sync
- [ ] Unit tests for core systems

**Deliverable:** Static library `libOpenHoCore.a` with deterministic game engine

### Phase 2b: Debugging AI (Future)

**Status:** Not started

**Purpose:** Quick implementation of a simple AI opponent for testing and debugging the base game mechanics. This AI makes random/semi-random decisions to explore the game's action space and find bugs/edge cases.

**Planned:**
- [ ] Create RandomAI class with basic decision-making
- [ ] Implement random action selection (colonize, build ships, research, attack)
- [ ] Add action validation (only valid actions)
- [ ] Create test harness for running multiple games
- [ ] Implement logging and error tracking
- [ ] Run stress tests to find game mechanic bugs

**Characteristics:**
- Not strategically sophisticated
- Explores game action space broadly
- Finds edge cases and bugs
- Quick to implement (~1-2 days)
- Builds infrastructure for Phase 4 AI
- Can run 100+ games for regression testing

**Deliverable:** RandomAI class integrated into game engine for testing

### Phase 3a: Objective-C++ Bridge (Future)

**Status:** Not started

**Planned:**
- [ ] Create Objective-C++ wrapper classes
- [ ] Implement GameBridge for UI-to-engine communication
- [ ] Memory management and lifecycle handling
- [ ] Event dispatch system
- [ ] Framework linking against C++ core

**Deliverable:** Static library `libOpenHoMacOS.a` bridging C++ to Swift

### Phase 3b: SwiftUI Application (Future)

**Status:** Not started

**Planned:**
- [ ] Create Swift Package Manager project structure
- [ ] Implement main SwiftUI views
- [ ] Game view controller
- [ ] Player interface and controls
- [ ] Graphics rendering
- [ ] Input handling
- [ ] Settings and preferences

**Deliverable:** Native macOS application `OpenHo.app`

### Phase 4: Sophisticated AI Development (Future)

**Status:** Planning/Design phase

**Purpose:** Develop a powerful, self-improving AI opponent through a multi-layered approach combining spatial analysis, behavioral cloning, and generative adversarial networks.

**Architecture Overview:**

1. **Influence Maps (Spatial Analysis)**
   - Analyze galaxy topology and player positions
   - Calculate strategic zones, threats, opportunities
   - Provides interpretable spatial reasoning

2. **Expert Behavior Cloning**
   - Record expert human gameplay
   - Train neural network to recognize expert decision patterns
   - Provides "expert taste" to discriminator
   - Data sources: player recordings, synthetic expert data from heuristics

3. **GAN Training (Self-Play Improvement)**
   - **Generator:** Creates novel strategies and move sequences
   - **Discriminator:** Initialized with expert behavior model, evaluates move quality
   - **Training:** Self-play games between generator and discriminator
   - Result: AI learns to generate expert-like but novel strategies
   - No explicit reward function needed (discriminator provides feedback)

4. **Difficulty Levels**
   - Different training iterations = different skill levels
   - Different discriminator weights = different personalities
   - Seeded randomness for variety (not fully deterministic)

**Development Phases:**

**Phase 4a: Influence Maps & Strategic Framework**
- [ ] Implement spatial analysis system
- [ ] Define influence map calculations
- [ ] Create heuristic decision-making framework
- [ ] Baseline AI using influence maps and heuristics

**Phase 4b: Behavioral Cloning**
- [ ] Design expert data collection system
- [ ] Record expert games (player recordings)
- [ ] Extract features from game states
- [ ] Train neural network on expert decisions
- [ ] Validate cloning model accuracy

**Phase 4c: GAN Training Infrastructure**
- [ ] Design generator network architecture
- [ ] Integrate discriminator with cloning model
- [ ] Implement self-play training loop
- [ ] Create training harness for offline training
- [ ] Train multiple AI agents with different parameters

**Phase 4d: Difficulty & Personality**
- [ ] Create difficulty level progression
- [ ] Design personality profiles (if desired)
- [ ] Tune discriminator weights for variety
- [ ] Validate AI quality at each difficulty level

**Key Design Decisions:**
- AI is non-deterministic (seeded randomness for variety)
- Training happens offline before release
- Runtime AI is fast (just evaluates learned patterns)
- Training can be computationally expensive
- Self-play eliminates need for explicit reward function

**Dependencies:**
- Complete Phase 2 game mechanics
- Stable game state serialization
- RandomAI framework from Phase 2b (reusable infrastructure)
- Expert gameplay data

**Estimated Effort:** Very High (significant ML/AI work)

**Deliverable:** Trained AI models for multiple difficulty levels

---

## Build System

### Philosophy
- No Xcode dependency - pure command-line development
- Reproducible builds across machines
- Modular compilation with independent phases
- CI/CD friendly with automated testing

### Build Tools
- **Phase 2:** CMake 3.10+ with Clang C++17
- **Phase 3a:** CMake 3.10+ with Clang Objective-C++
- **Phase 3b:** Swift Package Manager with Swift 5.5+

### Build Commands

**Current (Phase 2 only):**
```bash
./build.sh                    # Build C++ core
./clean.sh                    # Clean artifacts
```

**Future (all phases):**
```bash
./build.sh                    # Build all available phases
```

### Dependencies

**Phase 2:**
- Boost.Random (via Homebrew: `brew install boost`)
- Xcode Command Line Tools

**Phase 3a:**
- macOS frameworks (Foundation, AppKit)
- Phase 2 library

**Phase 3b:**
- Swift standard library
- SwiftUI framework
- Phase 3a library

---

## Requirements & Constraints

### User Requirements
- ✅ Mac OS native implementation (not SDL2)
- ✅ Snake_case naming convention throughout
- ✅ Population and income are properties of colonized planets only
- ✅ MoneyAllocation nested within Player struct
- ✅ colonized_planets vector owned directly by Player
- ✅ Consolidated planet development processing
- ✅ Command-line build system (no Xcode dependency)

### Technical Constraints
- macOS 10.14 (Mojave) or later
- C++17 standard
- Deterministic gameplay for multiplayer
- Clean API boundaries between layers

### Project Constraints
- Do not modify `prev_Mr_Twave/` directory
- All new code follows snake_case naming
- Build artifacts excluded from version control

---

## Current Session Progress

**Session Date:** December 31, 2025

### Completed This Session
1. ✅ Removed unused `turns_until_nova` member from Planet class
2. ✅ Added explicit `#include "enums.h"` to planet.h
3. ✅ Refactored Planet class with proper constructor
4. ✅ Refactored ColonizedPlanet to pointer-based composition (not inheritance)
5. ✅ Added apparent_gravity and apparent_temperature to ColonizedPlanet
6. ✅ Created game formulas for calculating apparent values
7. ✅ Renamed Planet members: gravity→true_gravity, temperature→true_temperature
8. ✅ Added public getters to Player class (ideal_gravity, ideal_temperature)
9. ✅ Implemented PlanetSnapshot struct with factory methods
10. ✅ Created sentinel value constants (UNKNOWN_DOUBLE_VALUE, INCOME_UNKNOWN, etc.)
11. ✅ Moved sentinel constants to enums.h
12. ✅ Implemented Planet constructor in planet.cpp
13. ✅ Implemented ColonizedPlanet constructor with apparent value calculation
14. ✅ Added name-based planet lookup system (planet_name_to_index map)
15. ✅ Updated GameState to support both id-based and name-based lookups
16. ✅ Documented save/load game state requirement
17. ✅ Established workflow: ask once for approval, no double-confirmation
18. ✅ Established workflow: only commit/push after dramatic changes or accumulation

### Commits This Session
- `9dabed6` - Remove unused turns_until_nova member
- `a2d389a` - Add explicit enums.h include to planet.h
- `38aed47` - Refactor BudgetSplit as private nested struct with accessors
- `8f22b03` - Create planet.cpp with BudgetSplit implementation
- `8eb5131` - Rename PlanetState to PlanetNovaState and update references
- `63a4235` - Refactor ColonizedPlanet to pointer-based composition
- `ead89d2` - Add Planet constructor and name-based planet lookup
- `69a7b2a` - Add PlanetSnapshot factory methods and sentinel constants
- `95998c2` - Move sentinel constants to enums.h

---

## Next Steps (Priority Order)

### Critical Path Dependencies

Before implementing higher-level features, these foundational systems must be in place:

**Phase 2c: Galaxy Initialization (CRITICAL)**
- [ ] Generate/populate galaxy with planets
- [ ] Assign starting planets to players
- [ ] Initialize player resources and technology
- [ ] Distribute starting ships
- Prerequisite for: all gameplay

**Phase 2d: Ship Movement (CRITICAL)**
- [ ] Define ship movement mechanics and orders
- [ ] Implement pathfinding/navigation
- [ ] Collision detection and avoidance
- [ ] Execute movement orders each turn
- Prerequisite for: combat, exploration, planet observation

**Phase 2e: Battle Mechanics (DEFERRED)**
- [ ] Combat resolution when fleets meet
- [ ] Damage calculation and ship destruction
- [ ] Victory conditions and surrender
- [ ] Retreat mechanics
- Status: Deferred until core mechanics stable
- Note: Placeholder/stub implementation needed for turn processing

### Immediate (Next Session)
1. **Galaxy Initialization** - Set up initial game state with planets and players
2. **Ship Movement** - Implement ship movement system
3. Initialize planetKnowledge vector for all players at game start
4. Implement planet observation system (update PlanetSnapshot when observed)
5. Write unit tests for RNG determinism

### Short Term
1. Complete turn processing logic (money allocation, research, terraforming, mining, ships, novae)
2. Implement remaining C API wrapper functions in ai_interface.cpp
3. Implement save/load game state (binary format preferred, JSON as fallback)
4. Serialization system for multiplayer sync
5. Comprehensive testing and validation
6. **Phase 2b:** Implement RandomAI for game mechanic testing and debugging

### Medium Term
1. Begin Phase 3a: Objective-C++ bridge layer
2. Create GameBridge for UI communication
3. Memory management and lifecycle handling
4. Run RandomAI stress tests to validate game mechanics

### Long Term
1. Begin Phase 3b: SwiftUI application
2. Implement game views and controls
3. Graphics rendering and input handling
4. Final integration and testing
5. **Phase 4:** Develop sophisticated AI with influence maps, behavioral cloning, and GAN training

---

## Testing Strategy

### Phase 2 (Current)
- Unit tests for RNG (verify determinism)
- Unit tests for GameState initialization
- Unit tests for turn processing
- Integration tests for multiplayer synchronization

### Phase 3
- Bridge layer tests
- SwiftUI component tests
- End-to-end gameplay tests

---

## Known Issues & Limitations

### Deferred Features
- **Save/Load Game State:** Requirement documented, implementation deferred to Phase 2
  - Format: Binary preferred, JSON as alternative
  - No database unless absolutely necessary
  - Must support mid-game persistence for human players

- **Battle Mechanics:** Phase 2e feature, deferred until core systems stable
  - Combat resolution when fleets meet
  - Damage calculation and ship destruction
  - Victory conditions and surrender
  - Placeholder implementation needed for turn processing
  - Full implementation after Phase 2 core is complete

- **Sophisticated AI:** Phase 4 feature, deferred until Phase 2 game mechanics are complete
  - Uses influence maps, behavioral cloning, and GAN training
  - Requires expert gameplay data for behavioral cloning
  - Training happens offline before release
  - Runtime AI is fast and non-deterministic

### Design Notes
- Player IDs must never be 0 (reserved for NOT_OWNED sentinel)
- Planet names must be unique (used as primary identifier)
- ColonizedPlanet uses pointer-based composition to avoid data duplication
- Phase 2b RandomAI provides infrastructure for Phase 4 sophisticated AI
- GAN training uses discriminator initialized with expert behavior model
- Galaxy initialization must happen before any gameplay
- Ship movement is prerequisite for combat and exploration
- Battle mechanics can be stubbed initially, fully implemented later

---

## File Manifest

### Documentation
- `MASTER_PLAN.md` - This file (single source of truth)
- `PROJECT_STRUCTURE.md` - Directory layout and future phases
- `BUILD_SYSTEM_PLAN.md` - Build architecture and CI/CD
- `IMPLEMENTATION_PROGRESS.md` - Detailed technical progress
- `README.md` - User-facing project description

### Build System
- `build.sh` - Master build script
- `clean.sh` - Cleanup script
- `src/core/CMakeLists.txt` - C++ core build configuration
- `.gitignore` - Version control exclusions

### Source Code
- `src/core/include/` - C++ headers
- `src/core/src/` - C++ implementation

### Archive
- `prev_Mr_Twave/` - Previous implementation (do not modify)

---

## Communication & Collaboration

**Repository:** https://github.com/melissa-anholm/OpenHo-  
**Build:** Command-line only (no Xcode)  
**Editor:** Any text editor or IDE  
**Version Control:** Git with regular commits

---

## Appendix: Quick Reference

### Build
```bash
./build.sh              # Full build
./clean.sh              # Clean artifacts
```

### Git
```bash
git status              # Check status
git diff                # Review changes
git add -A && git commit -m "message"  # Commit
git push origin main    # Push to GitHub
git pull origin main    # Pull from GitHub
```

### Project Structure
- C++ core: `src/core/`
- Future bridge: `src/macos/`
- Future app: `macos-app/`
- Archive: `prev_Mr_Twave/` (read-only)

### Key Files
- `openho_core.h` - Main C API
- `game.h` - GameState class
- `game.cpp` - Core implementation
- `rng.h/cpp` - Deterministic RNG

---

## Document History

| Date | Version | Changes |
|------|---------|----------|
| 2025-12-31 | 1.3 | Add critical path dependencies: Galaxy Initialization, Ship Movement, Battle Mechanics (deferred) |
| 2025-12-31 | 1.2 | Add Phase 2b (Debugging AI) and Phase 4 (Sophisticated AI with GAN training) |
| 2025-12-31 | 1.1 | Major Planet/ColonizedPlanet refactoring, PlanetSnapshot implementation, save/load requirement |
| 2025-12-29 | 1.0 | Initial consolidation of all project plans |

---

**This document is the single source of truth for the OpenHo project. It should be updated after each significant session to reflect progress and maintain accuracy for future sessions.**
