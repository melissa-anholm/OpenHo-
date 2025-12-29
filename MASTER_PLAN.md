# OpenHo Master Plan

**Last Updated:** December 29, 2025  
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

3. **ColonizedPlanet Structure:**
   - Inherits from `Planet` base class
   - Adds player-specific data: `population`, `income`, `allocation`
   - Owned directly by `Player::colonized_planets` vector

4. **Consolidated Planet Processing:**
   - Single `process_planets()` method handles terraforming and mining
   - Operates on `player.colonized_planets` vector

5. **C API Interface:**
   - All functions use `void*` for opaque game state pointer
   - Enables Objective-C++ bridging without exposing C++ types to Swift
   - Clean separation between core engine and UI layers

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

**In Progress:**
- [ ] Complete C API wrapper implementation (ai_interface.cpp)
- [ ] Implement turn processing logic:
  - [ ] Money allocation distribution
  - [ ] Research advancement
  - [ ] Terraforming effects
  - [ ] Mining extraction
  - [ ] Ship movement and combat
  - [ ] Nova handling
- [ ] Serialization system for multiplayer sync
- [ ] Unit tests for core systems

**Deliverable:** Static library `libOpenHoCore.a` with deterministic game engine

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

**Session Date:** December 29, 2025

### Completed This Session
1. ✅ Refactored game.cpp with complete snake_case conversion
2. ✅ Moved population/income from Planet to ColonizedPlanet
3. ✅ Implemented process_planets() consolidating terraforming/mining
4. ✅ Updated income calculation to use colonized_planets vector
5. ✅ Fixed build_entity_maps() to initialize ColonizedPlanet objects
6. ✅ Removed duplicate function definitions
7. ✅ Project compiles successfully
8. ✅ Saved checkpoint (commit 38d3576)
9. ✅ Pushed to GitHub
10. ✅ Created PROJECT_STRUCTURE.md documenting future layering
11. ✅ Created BUILD_SYSTEM_PLAN.md with build architecture
12. ✅ Created build.sh and clean.sh scripts
13. ✅ Consolidated all documentation into MASTER_PLAN.md

### Commits This Session
- `38d3576` - Refactor: Complete snake_case conversion and consolidate planet processing
- `a70e132` - docs: Add PROJECT_STRUCTURE.md and clean up README files
- `20dba06` - build: Add command-line build system plan and scripts
- (current) - docs: Consolidate all planning into MASTER_PLAN.md

---

## Next Steps (Priority Order)

### Immediate (Next Session)
1. Implement remaining C API wrapper functions in ai_interface.cpp
2. Write unit tests for RNG determinism
3. Test turn processing pipeline

### Short Term
1. Complete turn processing logic (money allocation, research, terraforming, mining, ships, novae)
2. Implement serialization system
3. Comprehensive testing and validation

### Medium Term
1. Begin Phase 3a: Objective-C++ bridge layer
2. Create GameBridge for UI communication
3. Memory management and lifecycle handling

### Long Term
1. Begin Phase 3b: SwiftUI application
2. Implement game views and controls
3. Graphics rendering and input handling
4. Final integration and testing

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

None currently documented. Issues will be tracked here as they arise.

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
|------|---------|---------|
| 2025-12-29 | 1.0 | Initial consolidation of all project plans |

---

**This document is the single source of truth for the OpenHo project. It should be updated after each significant session to reflect progress and maintain accuracy for future sessions.**
