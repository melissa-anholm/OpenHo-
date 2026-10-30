# OpenHo Master Plan

**Last Updated:** October 27, 2026 (Current-status and history separation)
**Project Status:** Phase 2 - Minimal playable game first; feature completeness follows incrementally

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

**Overall Status:** The minimal playable game is operational through a deterministic RANDOM-galaxy path and multiple turns, but the current RANDOM generator has known planet-count and density defects. Current priority is to correct and validate that path before expanding ship construction, exploration, and combat.

**Deliverable:** Static library `libOpenHoCore.a` with deterministic game engine

---

#### Phase 2a: Core Data Structures & Foundation

**Status:** Substantially complete for the minimal playable game.

The core entity model, deterministic RNG, GameState structure, C API boundary, colony model, entity mappings, text assets, and basic turn-processing foundation are in place. Several formulas and C API operations remain provisional or incomplete; see `Missing_Implementation_List.md` and the dated source audit.

#### Phase 2b: Fleet System & Ship Design

**Status:** Foundational structures complete; gameplay operations remain incomplete.

Players own fleet and ship-design collections, fleet movement state exists, and validation/delegation boundaries are established. Ship costs, resource deduction, real ship building, combat, and some movement validation remain future work. See the [active implementation checklist](Missing_Implementation_List.md#ship-design-creation).

#### Phase 2b-Extended: GameState Architecture & Text Assets

**Status:** Core architecture and text-asset loading are in place; complete serialization, host migration, and some public API operations remain provisional.

GameState coordinates setup, turns, entity lookup, RNG, and player state. Detailed refactoring history is preserved in `Project_Implementation_Log.md`.

#### Phase 2c: Galaxy Initialization & Shape Generation

**Status:** Implemented but unevenly validated.

The staged galaxy flow and six implemented coordinate generators—RANDOM, GRID, CIRCLE, RING, SPIRAL, and CLUSTER—are present. RANDOM remains the current integration path for the minimal playable game, but its planet-count and density behavior currently require correction. The other implemented shapes, especially SPIRAL and CLUSTER, require broader validation of exact counts, spacing, density, home-planet selection, edge cases, and reproducibility. The galaxy implementation is now split across focused source files. See the [Galaxy Generation checklist](Missing_Implementation_List.md#galaxy-generation).

#### Phase 2d: Starting Planet Assignment & Colony Initialization

**Status:** Minimal playable game initialization substantially complete.

Players receive homeworlds, quality-based starting population/income/metal, ideal conditions, colonized-planet state, and per-player knowledge initialization. Empty fleets and ship-design lists are deliberate defaults for the minimal playable game; whether the original game should provide starting fleets or designs remains undecided. Focused assertions for every initial field are still needed.

#### Phase 2e: Player Perception Formulas

**Status:** Formula implementations exist and are used by the current core; original-game validation remains incomplete.

Ideal temperature/gravity assignment and apparent-condition formulas are implemented. Their fidelity to the original game should not be inferred solely from source existence or monotonicity tests.

#### Phase 2f: Economic Mechanics

**Status:** Core provisional mechanics are implemented; several rules remain unresolved.

Interest, mining, terraforming, allocation, and unit conversions are present. Planetary income, population growth, research conversion/advancement costs, desirability, player metrics, and ship costs remain placeholders or provisional formulas. Turn-processing order still needs correction so refreshed planetary income and interest are included before allocation, and negative-income handling remains a required future feature. See the [data-driven implementation checklist](Missing_Implementation_List.md#data-driven-values-require-reverse-engineering).

### Key Design Decisions

**Galaxy Generation Source Organization (Approved October 27, 2026):**
- Keep `galaxy.cpp` focused on Galaxy-level orchestration, shared lifecycle behavior, planet initialization, and other functionality common to all galaxy shapes.
- Extract shared generation support into `galaxy_generation.cpp`; this name is intentional and does not imply a "commonplace" galaxy shape.
- Extract shape-specific coordinate generation into dedicated files such as `galaxygen_random.cpp`, `galaxygen_spiral.cpp`, `galaxygen_circle.cpp`, `galaxygen_ring.cpp`, `galaxygen_cluster.cpp`, and `galaxygen_grid.cpp`.
- Use separate `galaxy_home_selection.cpp` and `galaxy_diagnostics.cpp` files for home-planet selection and generation-failure reporting.
- Preserve public declarations and current behavior during the initial extraction. In particular, do not reorder, remove, or redesign the existing `GameState*` and direct-`DeterministicRNG&` overloads as part of this refactoring.
- Avoid duplicate symbols, duplicate implementations, and circular dependencies by inventorying all definitions before moving code and updating CMake only after each extracted implementation has a single owner.
- Treat this as a mechanical organization refactoring first; algorithmic changes and any future overload simplification require separate rationale and approval.

**Player-GameState Relationship:**
- Player holds a pointer to GameState for fleet ID allocation.
- Player keeps internal bookkeeping opaque to the UI.
- GameState coordinates public validation and state transitions; Player owns player-specific fleet and ship-design operations, with validation responsibilities currently split between the two layers.

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
- Turn order must be corrected so planetary income is refreshed and interest is calculated before allocation
- Negative effective income is charged directly against savings; no research, planetary development, or savings allocation is funded while income is negative. The bankruptcy limit remains data-driven and must be implemented.
- Refunds for insufficient resources (metal, temperature overshoot)
- Square root formulas for diminishing returns on mining/terraforming

**Turn Processing Design:**
- Economic processing occurs first, followed by queued fleet actions and then player actions.
- Immediate-effect player actions are applied in the order specified by the player.
- After economic processing, savings and accumulated metal reserve may update persistently; other recalculated economic values are projections until the next economic phase.
- Negative effective income reduces savings directly; recovery is possible before the data-driven bankruptcy limit is reached.
- Mining transfers metal losslessly from a planet to the player's shared metal reserve; planetary metal cannot fall below zero. Mining requires positive current-turn income allocation until **Dip Into Savings** exists.
- Terraforming consumes money but not metal, changes temperature immediately during economic processing, and uses a nonlinear spending formula. Overshoot is prevented and excess allocation is currently refunded to savings, pending a future refund-destination decision.
- Ship construction is immediate and costs both money and metal. The first ship of a type also incurs a money-only prototyping cost, typically about ten times the individual ship's money construction cost. Fleets are assembled from already-built ships rather than built as independent resource objects.
- Unaffordable ship actions fail atomically. Ships built during the current player-action phase may be scrapped during that same phase without losing construction or prototyping resources; after the phase, scrapping recovers metal only at a heavy loss.
- Queued fleet actions first resolve travel. Fleets still in transit have no further queued actions that turn; fleets at planets then resolve battles.
- Non-allied fleets at the same planet must fight, as must a fleet stationed at a non-ally's planet. Exact battle rules, including multi-player battles, remain future work.
- A planet whose entire population is killed becomes unowned, and surviving damaged ships are restored to full strength after battle. Mutual destruction is a draw.
- Combat population loss immediately reduces projected planetary income, but cannot change the economic income already processed for the current turn. Any unextracted metal remains on a destroyed colony's planet for whoever colonizes it later; there is no separate planet-capture operation.
- After battles resolve, an arriving ship of the colony-ship design automatically and freely establishes a colony at an uncolonized planet without consuming ships or resources. Non-allied colony ships fight before colonization; allied arrivals use speed technology to determine which ship proceeds. A surviving colony ship may colonize during the same turn.
- Abandonment is queued during player actions but occurs at the beginning of the next turn's economic processing. It is free: the colony and its colonists are removed, unmined planetary metal remains, stationed fleets stay in place, and projected income updates. The abandoned colony is excluded from that next income calculation.
- Resource transfers are immediate and lossless for both participants, may transfer money or metal, and are limited by the sender's available metal or permitted debt level. They are intentionally a very-low-priority implementation feature.
- The canonical snapshot labeled Turn N represents the beginning of Turn N; Turn 0 begins in year 2000. Economic after-state and post-processing state are retained for diagnostics, history, and display.
- The core engine enforces readiness. Players may ready, unready, and act again; actions submitted after ready make the player unready. Eliminated players do not block processing, and a host must explicitly decide whether to proceed after a disconnect.
- AI players become ready after completing their planned actions, subject to a configurable turn-length limit. Complete game state must be serializable for mid-game resume, and critical host-migration state must synchronize at least once per turn.
- First-turn starting values are tracked under Player Initial Setup in `Missing_Implementation_List.md`.
- Diplomacy is deliberately deferred until its action set is defined and is not required for the core game flow.

---

## Minimal Playable Game Roadmap

The immediate objective is not feature completeness. It is a small, repeatable minimal playable game that can initialize a game and advance through multiple turns.

### Stage 1: Reliable minimal galaxy

- [x] Use the RANDOM galaxy generator as the first integration path because it is the least structurally risky current generator; its planet-count and density defects remain open.
- [x] Confirm deterministic construction through the main `Galaxy`/`GameState` path with the minimal smoke test.
- [ ] Validate exact counts, home-planet availability, and edge cases across all generator paths.

### Stage 2: Correct player initialization

- [x] Assign homeworlds and starting colony values.
- [x] Initialize the minimal playable game's technology, research, allocations, and knowledge state.
- [x] Initialize deliberate empty defaults for fleets and ship designs for the minimal playable game.
- [ ] Decide whether starting fleets/designs belong in the original-game rules and add focused assertions for every initial field.

### Stage 3: Multi-turn playable loop

- [x] Implement turn-ready tracking and reset behavior.
- [x] Advance a minimal two-player game through multiple turns without crashing.
- [x] Keep provisional economics deterministic and non-crashing.
- [ ] Resolve remaining income/population ordering and state-integrity issues before treating the loop as complete.

### First minimal playable game milestone (October 23, 2026)

The minimal playable game milestone established a deterministic RANDOM-galaxy path, initialized player homeworlds and knowledge state, and advanced a two-player game through three turns in a 40-planet smoke test. Turn-readiness tracking and the provisional economic sequence are operational; starting fleets/designs, full formula fidelity, and broader galaxy validation remain future work. The C++ core builds successfully with CMake.

### Stage 4: Placeholder ship building

- Add a minimal ship-building operation after the multi-turn loop works.
- Initially, the operation may use placeholder economics, but it must create a real fleet/design state and report validation failures accurately.

### Stage 5: Ship travel, exploration, and knowledge

- Implement ship travel through the galaxy.
- Maintain incomplete or stale per-player knowledge of planets and fleets.
- Validate movement, arrival, ownership, and discovery behavior before combat.

### Stage 6: Ship combat

- Implement ship-versus-ship combat only after ship construction and travel are real rather than placeholders.
- Add combat resolution, damage, casualties, retreat, and victory/defeat state.

### Stage 7: Ship-versus-planet combat

- Extend the combat model to planetary attacks and defenses after ship combat is stable.

### Stage 8: Nova events

- Treat novae as the final major subsystem.
- Keep them rare and optional while the rest of the game is being completed; the game can be considered effectively complete before nova processing is finalized.

### Documentation and approval rule

This roadmap is a planning decision, not authorization to modify implementation code. Source changes require explicit approval before work begins. The master plan and the source audit should be updated after each significant milestone.

### Recent Milestone History

Detailed session history and commit-level implementation notes are maintained in `Project_Implementation_Log.md`. The current active backlog is maintained in `Missing_Implementation_List.md`.

---

**This document is the single source of truth for current project direction and approved milestones. Detailed active implementation status is maintained in `Missing_Implementation_List.md`; historical implementation detail is maintained in `Project_Implementation_Log.md`. Update all applicable documents after each significant session.**
