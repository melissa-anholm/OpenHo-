# OpenHo Project Structure

This document describes the intended architecture and directory structure of the OpenHo project as it evolves through development phases.

## Current Phase: Phase 2 - C++ Core Implementation

The project is currently focused on building a deterministic C++ game engine with a C API interface.

```
OpenHo/
├── src/core/                          # C++ Game Engine (Phase 2)
│   ├── include/
│   │   ├── openho_core.h              # Main C API header
│   │   ├── game.h                     # GameState class
│   │   ├── rng.h                      # Deterministic RNG
│   │   ├── game_formulas.h            # Game calculation formulas
│   │   ├── money_allocation_calculator.h
│   │   └── ai_interface.h             # AI interface declarations
│   ├── src/
│   │   ├── game.cpp                   # GameState implementation
│   │   ├── rng.cpp                    # RNG implementation
│   │   ├── game_formulas.cpp          # Formula implementations
│   │   ├── money_allocation_calculator.cpp
│   │   ├── ai_interface.cpp           # C API wrapper implementation
│   │   ├── simulation.cpp
│   │   ├── commands.cpp
│   │   ├── money_allocation.cpp
│   │   ├── planet_mechanics.cpp
│   │   ├── serialization.cpp
│   │   └── types.cpp
│   ├── build/                         # CMake build artifacts (not committed)
│   └── CMakeLists.txt
├── prev_Mr_Twave/                     # Archive of previous implementation
└── Documentation files
```

## Future Phase: Phase 3 - macOS Native UI

When Phase 3 begins, the project structure will expand to include:

```
OpenHo/
├── src/
│   ├── core/                          # C++ Game Engine (Phase 2)
│   │   ├── include/
│   │   ├── src/
│   │   ├── build/
│   │   └── CMakeLists.txt
│   │
│   └── macos/                         # Objective-C++ Bridging Layer (Phase 3)
│       ├── OpenHoMacOS.h              # Objective-C++ wrapper classes
│       ├── OpenHoMacOS.mm             # Implementation
│       ├── GameBridge.h
│       ├── GameBridge.mm
│       └── CMakeLists.txt (or Xcode integration)
│
├── macos-app/                         # Native macOS Application (Phase 3)
│   ├── OpenHo.xcodeproj               # Xcode project
│   ├── OpenHo/
│   │   ├── ContentView.swift          # SwiftUI main view
│   │   ├── GameViewController.swift
│   │   ├── Models/
│   │   ├── Views/
│   │   └── ...
│   └── OpenHoTests/
│
├── prev_Mr_Twave/                     # Archive
└── Documentation files
```

## Architecture Overview

### Layer 1: C++ Core (`src/core/`)
- **Language:** C++17
- **Purpose:** Deterministic game engine with all game logic
- **Key Components:**
  - GameState class managing game state
  - Deterministic RNG for reproducible gameplay
  - Turn processing pipeline
  - Serialization for multiplayer sync
- **Output:** Static library (`libOpenHoCore.a`)

### Layer 2: Objective-C++ Bridge (`src/macos/`)
- **Language:** Objective-C++
- **Purpose:** Bridge between C++ core and Swift UI
- **Key Components:**
  - Wrapper classes converting C++ types to Objective-C
  - Memory management and lifecycle handling
  - Event dispatch from UI to game engine
- **Output:** Framework or library for macOS app

### Layer 3: SwiftUI Application (`macos-app/`)
- **Language:** Swift
- **Purpose:** Native macOS user interface
- **Key Components:**
  - Game views and controls
  - Player interface
  - Graphics rendering
  - Input handling
- **Output:** Executable macOS application

## Naming Conventions

Throughout the project:
- **Classes/Structs:** PascalCase (e.g., `GameState`, `DeterministicRNG`)
- **Functions/Methods:** snake_case (e.g., `process_turn()`, `calculate_income()`)
- **Variables/Members:** snake_case (e.g., `current_turn`, `player_money`)
- **Constants:** UPPER_SNAKE_CASE (e.g., `MAX_PLAYERS`, `DEFAULT_GALAXY_SIZE`)

## Build System

### Phase 2 (Current)
- **Build tool:** CMake 3.10+
- **Compiler:** C++17 compatible (GCC, Clang, MSVC)
- **Dependencies:** Boost (Random component)

### Phase 3 (Future)
- **Build tool:** Xcode + CMake (for C++ core integration)
- **Compiler:** Clang (Apple)
- **Dependencies:** Boost, macOS SDK

## Development Workflow

1. **Phase 2:** Focus on C++ core correctness and determinism
   - Unit tests for game logic
   - Serialization verification
   - Multiplayer sync testing

2. **Phase 3:** Integrate with macOS UI
   - Objective-C++ wrapper implementation
   - SwiftUI component development
   - End-to-end testing

## Notes

- The `prev_Mr_Twave/` directory is preserved for reference and should not be modified
- All new development should follow the snake_case naming convention for consistency
- The C API in `openho_core.h` is the primary interface between layers
- Serialization must be deterministic to support multiplayer synchronization
