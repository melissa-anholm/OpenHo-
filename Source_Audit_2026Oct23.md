# OpenHo Source Implementation Audit

**Audit date:** October 23, 2026  
**Repository:** `melissa-anholm/OpenHo-`  
**Branch:** `main`  
**Audited commit:** `504c520` — `Refactor cluster galaxy implementation and clean up repository`

## Purpose

This document records a read-only source audit comparing the repository's actual C++ implementation with the claims in `Master_Plan.md` and `Missing_Implementation_List.md`. The word **implemented** in the planning documents often means that code exists; it does not necessarily mean that the behavior is complete, faithful to the original game, or covered by tests.

No production source files were changed as part of this audit.

## Executive assessment

OpenHo has a substantial C++17 core architecture, deterministic-RNG infrastructure, entity models, GameState/GameSetup organization, C API layers, and significant galaxy-generation code. However, the project is not yet close to a complete playable game. Several formulas are explicitly placeholders, several game-loop stages are absent, and some implemented features are not wired into the main execution path.

A more useful status vocabulary is:

1. **Verified behavior** — supported by tests or direct validation against requirements/original behavior.
2. **Implemented but unvalidated** — nontrivial code exists, but behavior still needs tests.
3. **Temporary/ad hoc implementation** — code exists primarily to keep the engine moving while the real rule is unresolved.
4. **Explicit placeholder/stub** — source comments or behavior clearly identify missing functionality.
5. **Missing integration** — a feature exists but the primary game path does not use it.

## Explicit placeholders and stubs

### `src/core/src/game_formulas.cpp`

The following functions are explicitly provisional:

- Ship design build, prototype, and metal costs return `1`.
- Player fleet power returns `1`.
- Player victory points return `1`.
- Research conversion is currently one-to-one.
- Planetary income returns `0`.
- Windfall income returns `0`.
- Population growth is fixed at 10%, capped at one million, and ignores temperature/gravity and player-ideal parameters.
- Nova warning duration returns `1`.
- Technology advancement uses a generic quadratic formula for every technology stream; this is not yet established as the original game's rule.

### Other explicit stubs

- `GameState::serialize_state()` returns an empty buffer.
- Turn-ready tracking functions in `gamestate_c_api_extensions.cpp` do not record or query readiness.
- `game_all_players_ready()` always returns false/zero.
- `game_get_current_year()` always returns zero.
- `game_player_set_planet_allocation()` validates input but does not actually set planet allocation.
- `game_build_ship_from_design()` returns success without building a ship or deducting resources.
- Nova processing is an empty method.
- Several player C API accessors and mutators contain TODOs or return placeholder values.

## Player initialization

Player creation and homeworld assignment are partially implemented, but the nine-step completion claim in the checklist remains too strong.

Currently present:

- Player objects are created from `PlayerSetup`.
- Names and genders are assigned.
- Ideal temperature is generated from a truncated normal distribution.
- Home planets are assigned.
- Ideal gravity is assigned.
- Homeworld temperature is changed to the player's ideal temperature.
- Starting population and income are assigned.
- A `ColonizedPlanet` object is created.

Remaining or questionable:

- Homeworld metal is not assigned; `game.cpp` contains `TODO: Set metal based on quality`.
- Savings and metal-reserve initialization is not clearly performed as a dedicated setup step.
- Initial fleets and ship designs are not created.
- Initial technology and research state rely substantially on defaults rather than an explicit, documented initialization sequence.
- Initial income setup is not clearly separated from later recalculation.

## Income and population behavior

`GameState::calculate_player_incomes()` sums the income already stored on each `ColonizedPlanet`.

`GameState::update_planet_incomes()` then applies a temporary happiness formula based on temperature and gravity differences and sets income to `population * happiness * 10`. The source labels this formula as a placeholder. Because the turn loop calls `calculate_player_incomes()` before `update_planet_incomes()`, the newly calculated income is not necessarily the income used by the same turn's allocation phase. This ordering and the duplicated calculation paths need resolution.

Population growth is called from `update_planet_incomes()`, but the formula currently ignores all planet-condition arguments and simply applies 10% growth subject to a one-million cap.

## Turn processing

The turn sequence is structurally present:

1. Capture public information.
2. Calculate player incomes.
3. Update planet incomes.
4. Process money allocation.
5. Apply interest.
6. Process research.
7. Process planets.
8. Process ships.
9. Process novae.
10. Increment turn and year.

But the following are incomplete or provisional:

- Planetary income formula and ordering are unresolved.
- Research conversion and advancement costs are provisional.
- Fleet combat is absent.
- Nova processing is absent.
- Turn-ready gating is absent.
- End-of-turn state handling is incomplete.

## Fleet and ship systems

Fleet and ship-design data structures are present, and basic movement/arrival machinery exists. However:

- Ship-design creation lacks meaningful validation, cost handling, and maximum-count enforcement.
- Ship building is a success-returning stub.
- Combat resolution, damage, casualties, retreat, and victory/defeat handling are missing.
- Fleet creation requires review for parameter validation and resource deduction.
- Deleting a ship design does not yet check whether a fleet uses it.

### Likely fleet arrival indexing bug

`GameState::process_ships()` uses a destination planet ID as a direct vector index:

```cpp
if (dest_id < galaxy->planets.size())
    destination = &galaxy->planets[dest_id];
```

Planet IDs begin at `1`, while vector indices begin at `0`. This likely selects the wrong planet and rejects the last valid ID. The code should use the GameState planet lookup mechanism rather than treating IDs as indices. This should be confirmed with a focused test before changing it.

## Galaxy generation

Galaxy generation is substantially developed, but it should be classified as **implemented but unvalidated**, not production-ready.

### Spiral integration gap

`select_home_planets_spiral()` exists, but the `Galaxy` constructor currently falls back to `select_home_planets_random()` for spiral galaxies because the generated spiral parameters are not passed through to the selector. Thus the intended one-home-planet-per-arm behavior is not currently used by the main constructor path.

### Cluster orientation gap

Cluster generation creates a random angular offset and stores it in `GalaxyGenerationParams`, but cluster-center placement uses:

```cpp
double angle = (2.0 * M_PI * cluster_idx) / n_clusters;
```

The offset is not added to this angle. Therefore the cluster ring itself is not randomly rotated; only the later home-selection wedge origin is rotated. This does not fully match the stated design requirement.

The implementation also uses `const_cast` to modify a `const GalaxyGenerationParams&`, which makes the state flow fragile and deserves redesign during future work.

### Cluster home-planet robustness

The wedge selector only appends a home planet when a wedge is nonempty. It can therefore return fewer home planets than players. The constructor detects this and throws, but there is no retry or fallback strategy.

### Shape and parameter validation

The RNG overloads for circle and ring generation state that density does not affect Poisson-disk packing. Grid generation ignores density and RNG. Spiral generation creates arm candidates and a core but needs exact-count, spacing, and home-selection validation. All shapes require tests for:

- Exact planet count.
- Minimum pairwise distance.
- Density behavior.
- Homeworld count and placement.
- Extreme planet/player/density combinations.
- Seed reproducibility.
- Visual artifacts.

## Repository/documentation inconsistencies

The planning files are historical and internally inconsistent:

- `Missing_Implementation_List.md` is dated February 3 and describes cluster work as complete.
- `Master_Plan.md` is dated January 27 and still lists cluster generation as pending.
- Some sections call the galaxy system “fully implemented” while separately requiring extensive testing.
- The phrase “all four shapes” omits the presence of `RANDOM` and `GRID` in the six-shape enum.

The source audit should be treated as the more accurate current status until the planning documents are revised.

## Recommended next steps

1. Add focused tests for player initialization and assert every expected initial field.
2. Add tests that expose the planet-ID/vector-index issue in fleet arrival.
3. Resolve the income update ordering and replace the temporary income formula only after the intended rule is established.
4. Implement turn-ready tracking before relying on the game loop for multiplayer flow.
5. Decide how spiral parameters are passed into home-planet selection.
6. Apply or deliberately remove cluster angular-offset behavior, then test it.
7. Build a behavior matrix distinguishing source existence from verified game behavior.
8. Reverse-engineer formulas requiring original-game observation before treating them as complete.

## Build verification

**Environment preparation:** Installed `cmake`, `build-essential`, and `libboost-random-dev` through apt.

**Command:**

```sh
cmake -S src/core -B /tmp/openho-build -DCMAKE_BUILD_TYPE=Debug
cmake --build /tmp/openho-build -j2
```

**Initial result:** The first build failed in `src/core/src/text_assets.cpp` because `src/core/include/enums.h` used `INT32_MIN` before the defining header was available. The build also emitted warnings that corroborated the placeholder findings above, including unused parameters in placeholder formulas, unused spiral-generation variables, and an unused serialization parameter.

**Fix applied:** Added the explicit integer header include, corrected the `GameFormulas` namespace mismatch for the apparent-temperature/gravity definitions, and corrected entity-map construction so it uses the newly created galaxy before `GameState` stores it.

**Current result:** The core library and `openho_minimal_game_test` now build successfully. The smoke test constructs a deterministic 40-planet RANDOM galaxy for two players, verifies homeworld population/income/metal and knowledge initialization, exercises turn-readiness tracking, and advances the game through three turns from year 2000 to 2030 without crashing.

The build still emits non-fatal warnings for known placeholder or provisional code. The executable must be run from the repository root, because text assets are currently loaded through the relative path `TextAssets/`.
