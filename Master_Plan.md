# OpenHo Master Plan

**Last Updated:** November 5, 2026 (PlanetView checkpoint persistence)
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

**Economic policy decision:** The provisional intentional-debt limit is calculated from summed planetary income over a ten-turn repayment horizon, with the current 15% per-turn debt-interest rate. The provisional bankruptcy limit is five times the debt at which interest equals `max(summed planetary income, 50,000)`. Debt-limit functions accept summed planetary income and planet count explicitly; planet count is reserved for a future data-driven refinement.

Positive savings-rate changes use a Player-owned modifier to produce an `actual_savings_rate`; the modifier currently defaults to `1.0` and is not yet serialized. It does not alter the separate negative debt-interest rule. Interest in either direction is applied to player income rather than directly to the savings balance.

**Deliverable:** Static library `libOpenHoCore.a` with deterministic game engine

---

#### Phase 2a: Core Data Structures & Foundation

**Status:** Substantially complete for the minimal playable game.

The core entity model, deterministic RNG, GameState structure, C API boundary, colony model, entity mappings, text assets, and basic turn-processing foundation are in place. Several formulas and C API operations remain provisional or incomplete; see `Missing_Implementation_List.md` and the dated source audit.

#### Phase 2b: Fleet System & Ship Design

**Status:** Foundational structures and provisional cost calculations are implemented; the complete player-facing build-selection process remains in progress.

Players own fleet and ship-design collections, fleet movement state exists, and validation/delegation boundaries are established. Provisional ship costs, resource deduction, and immediate single-design fleet creation are implemented. The complete player-facing build-selection process, same-phase scrapping, combat, and some movement validation remain future work. See the [active implementation checklist](Missing_Implementation_List.md#ship-design-creation).

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

**Status:** Phase 1 economic-kernel ordering is implemented and regression-tested; the Dip Into Savings action model requires replacement before this part of the economic system is complete.

The economic path applies population growth, refreshes planetary income using the resulting population, calculates interest from beginning-of-turn savings, commits one effective-income value, and then applies allocations, research, mining, and terraforming. The authoritative Dip Into Savings design is an action-phase selection with an immediate reversible savings adjustment and a locked amount applied to the following turn's effective income. Persistent pension-plan mode is the default, with a separate one-time option. The current implementation applies the dip at the wrong phase and must be replaced with separate pending/locked state. The dip is allowed to exceed current savings for now; the permitted-debt and bankruptcy limit remains a data-driven future rule. Population growth, planetary-income, research, mining, terraforming, and interest formulas remain provisional or require data-driven validation. Full beginning/economic/post-processing snapshots, queued actions, and later fleet phases remain future work. See the [data-driven implementation checklist](Missing_Implementation_List.md#data-driven-values-require-reverse-engineering).

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

**PlanetView Ownership Refactor (Approved November 4, 2026):**
- `Planet` remains the authoritative physical planet owned by `Galaxy`.
- Each `Planet` owns a mapping from stable `PlayerID` values to `PlanetView` records.
- `PlanetView` contains observer-specific, potentially stale knowledge and does not own or contain a `ColonizedPlanet`.
- The authoritative colony record remains the `ColonizedPlanet` stored in the owning `Player`.
- The former `KnowledgePlanet` class has been removed. `KnowledgeGalaxy` currently remains as a temporary compatibility facade over Planet-owned views; individual-planet and whole-galaxy player-view query APIs remain future work.

**PlanetView Checkpoint Persistence (Approved November 5, 2026):**
- Each planet's runtime checkpoint record stores its observer-specific `PlanetView` snapshots alongside the planet's other mutable state.
- Snapshot scalar fields are serialized and restored; identity/geometry fields are reconstructed from the authoritative `Planet` and observer ID.
- Gravity knowledge is represented by a Boolean because true gravity is immutable; the gravity value itself is derived from the authoritative planet once observed.
- Nova state belongs to the authoritative planet and is serialized in the planet runtime record, not in `PlanetView`.
- Fleet information is not part of `PlanetView`; fleet presence and fleet observations belong to the separate fleet/observation systems.

**Population Ownership Decision (Approved November 5, 2026):**
- `Planet::population` is the sole authoritative population value for a colony.
- `ColonizedPlanet` retains its non-owning pointer to the authoritative `Planet` and exposes population only as a convenience access path; it does not store a second population field.
- Economic processing and `PlanetView` observation setters read the population directly from `Planet`.
- Runtime checkpoints serialize population with each planet record and reconstruct player-owned `ColonizedPlanet` records from that authoritative value; population is not serialized in `RuntimeColonyState`.

**Allocation Ownership Decision (Approved November 5, 2026):**
- Player-editable budget allocations are authoritative next-turn action state, not observer knowledge.
- Per-colony funding, mining, and terraforming fractions remain in the owning player's `ColonizedPlanet` record and are the only values used by the next economic phase.
- `PlanetView` no longer stores or serializes allocation fractions. For an owned colony, the UI obtains current allocation choices from `ColonizedPlanet`; `PlanetView` retains only observer-facing snapshot data such as projected income.

**Economic Submission and After-State Records (Implemented November 5, 2026):**
- `EconomicActionSubmission` represents the complete final, non-ordered economic choices for the next turn: player-wide allocations, research-stream allocations, per-colony allocations, and the Dip Into Savings selection.
- `GameState::submit_economic_action()` validates and accepts the complete submission as one unit. Fleet actions, movement, construction ordering, transfers, abandonment, and other ordered actions are intentionally outside this record.
- `EconomicAfterState` records the server-produced result of economic processing, including planetary income, interest, net/effective income, post-processing savings and metal reserve, and per-colony population/income.
- Both records are included in the versioned runtime checkpoint and are restored on load. Allocation values remain authoritative in `Player` and `ColonizedPlanet`; the submission record is the explicit turn-boundary representation of those choices.

**Economic Submission Gate (Implemented November 5, 2026):**
- `current_turn` identifies the next economic turn awaiting processing, including Turn 0. Economic submissions target `current_turn`; after Turn N is processed, the after-state is captured as Turn N before the counter advances to N+1.
- A turn cannot process until every non-eliminated player has a valid economic submission for `current_turn` and has explicitly marked themselves done/ready.
- Marking ready freezes that player's accepted economic submission for the processing pass. Changing an allocation, Dip Into Savings selection, or submitting a replacement action clears readiness and requires readying again.
- Calling `process_turn()` while the gate is incomplete is a no-op; the existing readiness query exposes whether processing may proceed. Fleet and other ordered actions remain outside this gate.

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
- Intentional actions such as ship building and Dip Into Savings share a provisional intentional-debt limit; a separate, larger provisional bankruptcy/default limit causes loss when reached. Both values and their final enforcement rules require data-driven replacement.
- Mining transfers metal losslessly from a planet to the player's shared metal reserve; planetary metal cannot fall below zero. Mining requires positive effective income allocated to planetary development.
- Terraforming consumes money but not metal, changes temperature immediately during economic processing, and uses a nonlinear spending formula. Overshoot is prevented and excess allocation is currently refunded to savings, pending a future refund-destination decision.
- Ship designs are player-owned blueprint objects. Each design stores whether its ship type has already been prototyped; this flag determines whether the next build pays the additional money-only prototyping cost. There is no separate unassembled-ship inventory. Future fleet-composition actions may combine ship types, split fleets, or otherwise rearrange ships, but are separate later functionality.
- Ship construction offers two player-facing selection paths: choose an already-owned design, or configure a new design within the player's currently available technology levels. Current technology limits are enforced when a new design is created, but do not invalidate already-owned designs; future mechanics may grant designs that exceed the player's ordinary technology access. A proposed build also computes its maximum legal ship count from the shared metal reserve and available build money, using the shared intentional-debt policy. For an unprototyped design, the money requirement is one prototype charge plus N individual build costs.
- If a new design is actually built, it is added to the player's design library and its prototype state is established as part of that build. Ships may be built at any planet owned by the player; there are no shipyards, construction facilities, or planet-specific build limits. A build action creates a fleet containing N ships of that design and charges money and metal atomically. There is no separate unassembled-ship inventory. Future fleet-composition actions may combine ship types, split fleets, or otherwise rearrange ships, but are separate later functionality.
- Same-phase scrapping must roll back the prototype state if every ship built from a previously unused design is scrapped before the player-action phase ends; the design remains in the player's library. The implementation must prevent using this rollback to avoid paying a prototype charge on a later turn.
- Unaffordable ship actions fail atomically. Ships built during the current player-action phase may be scrapped during that same phase without losing construction or prototyping resources; after the phase, scrapping recovers metal only at a heavy loss.
- Queued fleet actions first resolve travel. Fleets still in transit have no further queued actions that turn; fleets at planets then resolve battles.
- Whenever forces belonging to two or more non-allied players occupy the same place, they fight; fleets and owned planets are both forces in that battle. An invading weapon cannot target the planet until all defending ships are destroyed, but the planet may fire when its turn comes around. Exact battle rules, including force-turn order and multi-player battles, remain future work.
- When the planet is targeted, its population is destroyed according to the battle rules; when all population is gone, the colony is destroyed and the planet becomes unowned. Surviving damaged ships are restored to full strength after battle, and mutual destruction is a draw.
- Combat population loss immediately reduces projected planetary income, but cannot change the economic income already processed for the current turn. Any unextracted metal remains on a destroyed colony's planet for whoever colonizes it later; there is no separate planet-capture operation.
- After battles resolve, an arriving ship of the colony-ship design automatically and freely establishes a colony at an uncolonized planet without consuming ships or resources. Non-allied colony ships fight before colonization; allied arrivals use speed technology to determine which ship proceeds. A surviving colony ship may colonize during the same turn. Exploration desirability need not be stored until colonization; then it is recalculated, stored with `ColonizedPlanet`, and recalculated during later planet processing.
- Abandonment is queued during player actions but occurs at the beginning of the next turn's economic processing. It is free: the colony and its colonists are removed, unmined planetary metal remains, stationed fleets stay in place, and projected income updates. The abandoned colony is excluded from that next income calculation.
- Resource transfers are immediate and lossless for both participants, may transfer money or metal, and are limited by the sender's available metal or permitted debt level. They are intentionally a very-low-priority implementation feature.
- The canonical snapshot labeled Turn N represents the beginning of Turn N; Turn 0 begins in year 2000. Economic after-state and post-processing state are retained for diagnostics, history, and display.
- The core engine enforces readiness. Players may ready, unready, and act again; actions submitted after ready make the player unready. Eliminated players do not block processing, and a host must explicitly decide whether to proceed after a disconnect.
- AI players become ready after completing their planned actions, subject to a configurable turn-length limit. Complete game state must be serializable for mid-game resume, and critical host-migration state must synchronize at least once per turn.
- **Save/persistence policy:** Autosave once at the end of every completed turn. Do not autosave at every player decision point, since the interface should not incur repeated serialization and file-write overhead. Support an explicit manual save on request, including before graceful application shutdown. Losing less than one incomplete turn after an accidental interruption is acceptable under this policy.
- **Save-file architecture:** Use one versioned append-only save file. Store the immutable game definition once at the beginning, then append complete mutable runtime checkpoints after completed turns and for explicit manual saves. Each checkpoint is independently framed and checksummed; loading uses the newest complete valid record and ignores an incomplete trailing append. Periodic compaction may retain the definition and the newest checkpoint while removing older records. The initial runtime record format is snapshot-based rather than delta/event-based.
- **Runtime snapshot scope:** Serialize authoritative player state—including resources, income stages, player and per-colony allocations, technology, research, Dip Into Savings state, and explicit player elimination status—and the stabilized scalar `PlanetView` snapshots stored alongside each planet. `PlanetView` snapshots do not duplicate owner-controlled allocation fractions. Fleet state, fleet sightings, and related public-information history remain incomplete where they depend on the still-provisional fleet model. Player ideal temperature and gravity are immutable per-player setup data and belong in the immutable definition’s player section, not in every runtime checkpoint.
- **RNG synchronization:** Shared game mechanics use the deterministic RNG stream, whose state must be identical on all synchronized game instances. AI decisions use a separate AI RNG stream so AI randomness does not perturb shared mechanics. A normal new game obtains its deterministic seed from `std::random_device` when no explicit seed is supplied; the AI seed is derived from it (`deterministic_seed + 1`). Explicit seed/state transfer is required for clients and host migration. Runtime checkpoints now include both seeds and both current engine states.
- **Planet/colony serialization boundary:** For mutable runtime checkpoints, read shared physical planet state directly from `Planet`—currently owner, true temperature, remaining metal, nova state, and authoritative population. Read per-turn and colony-specific state—including current income, funding, budget fractions, spending/accounting values, and the player’s cached previous-turn metal income—from the owning player’s `ColonizedPlanet` record or player record as appropriate. Checkpoint loading must rebuild the pointer relationships from player and planet IDs and validate that each colony record agrees with the corresponding planet owner.
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

### Stage 4: Placeholder ship design and construction

- [x] Rewrite the partial ship-design scaffold into a coherent player-owned design system.
- [x] Store a `has_been_prototyped` flag directly with each design, and use it to charge the first-build prototype cost for that design.
- [x] Use placeholder cost formulas initially, but charge money and metal atomically and accurately.
- [x] Make each build action create its own same-design fleet immediately; no separate unassembled-ship inventory is required.
- Defer mixed-design fleet assembly, fleet splitting, and other fleet-composition actions until after basic construction and travel are functional.

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
