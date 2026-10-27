# OpenHo: Start Here

This document is the practical entry point for the OpenHo project. It explains the current development state, the immediate direction, how to build the present C++ core, and where to find the authoritative project information.

## Current project state

OpenHo is a native re-implementation of *Spaceward Ho!* with a C++17 game core and a planned native macOS interface. The current priority is a minimal, inspectable game loop rather than feature completeness.

The core currently has:

- A CMake-built C++17 library with deterministic random-number support.
- Galaxy construction through the main `GameState` path.
- Six galaxy-generation paths: RANDOM, SPIRAL, CIRCLE, RING, CLUSTER, and GRID.
- Galaxy-generation source organized by shared orchestration, shape, home selection, and diagnostics responsibilities.
- Player homeworld assignment and initial colony state.
- Knowledge-galaxy initialization.
- A minimal two-player game that advances through multiple turns.
- A consolidated spiral-generation diagnostic and a separate Python/pybind11 wrapper for galaxy experimentation.

The RANDOM galaxy path is the current reliable minimal-game path. The other generators exist and can be exercised independently, but they still require broader validation of exact planet counts, minimum spacing, density behavior, home-planet placement, extreme parameters, reproducibility, and visual output.

Several systems remain provisional or incomplete, including parts of planetary income, population growth, ship design and construction, serialization, fleet movement and combat, and nova processing. Existing source code is not automatically considered complete merely because a function exists or the project compiles.

## Immediate development direction

The approved development sequence is:

1. Keep the minimal multi-turn game loop stable and observable.
2. Replace or complete provisional economics and population rules as their intended behavior is established.
3. Implement placeholder ship construction as a real, inspectable state transition.
4. Implement ship travel, exploration, and incomplete per-player knowledge.
5. Implement ship combat and then ship-versus-planet combat.
6. Treat nova events as the final major subsystem.

The current galaxy-generation file split is an organization refactoring. Future work should preserve the existing overloads and behavior unless a separate rationale and approval establish otherwise.

## Build and smoke test

From the repository root:

```sh
./build.sh
./src/core/build/openho_minimal_game_test
./clean.sh
```

`build.sh` builds the existing C++ core with CMake. It does not attempt to build the future Objective-C++ or SwiftUI layers, which do not yet exist in this checkout.

The smoke test must currently be run from the repository root because the core loads text assets through the relative `TextAssets/` path. A successful run reports that a minimal RANDOM-galaxy game advanced through three turns.

The Python wrapper has additional Python, pybind11, Boost, and Python-development-header requirements. Its usage and build instructions are in [`Python Galaxy Tests/Usage_Guide.md`](Python%20Galaxy%20Tests/Usage_Guide.md).

## Which document answers which question?

The project deliberately does not use a generic `README.md` as its development authority.

| Question | Document |
|---|---|
| What is the project and why does it exist? | [`About.md`](About.md) |
| What direction has been approved for development? | [`Master_Plan.md`](Master_Plan.md) |
| What remains to be implemented or reverse-engineered? | [`Missing_Implementation_List.md`](Missing_Implementation_List.md) |
| What does the source actually do at a particular point in time? | [`Source_Audit_2026Oct23.md`](Source_Audit_2026Oct23.md) |
| Why were earlier design decisions made? | [`Project_Implementation_Log.md`](Project_Implementation_Log.md) |
| How does the galaxy Python wrapper work? | [`Python Galaxy Tests/Usage_Guide.md`](Python%20Galaxy%20Tests/Usage_Guide.md) and [`Python Galaxy Tests/ARCHITECTURE.md`](Python%20Galaxy%20Tests/ARCHITECTURE.md) |
| How does Poisson disk sampling work mathematically? | [`Technical_Refs/PoissonDiskSamplingOverview.md`](Technical_Refs/PoissonDiskSamplingOverview.md) |
| What future AI-opponent approaches have been considered? | [`AI_DEVELOPMENT_CONSIDERATIONS.md`](AI_DEVELOPMENT_CONSIDERATIONS.md) |
| What future build/platform architecture has been proposed? | [`BUILD_SYSTEM_PLAN.md`](BUILD_SYSTEM_PLAN.md) |

## Document authority and status vocabulary

The documents have different roles:

- `Master_Plan.md` is the authority for approved roadmap direction and architectural decisions.
- `Missing_Implementation_List.md` is the active backlog, but its status claims should be interpreted carefully until it is refreshed against the current source.
- `Source_Audit_2026Oct23.md` is a dated audit snapshot. It is authoritative for the commit and environment named inside it, not an automatically current status report.
- `Project_Implementation_Log.md` is historical. It records sessions, investigations, and rationale rather than serving as a current checklist.
- `About.md` is background and human-interest material, not a technical planning document.
- Documents under `Technical_Refs/` are supporting technical references. They do not override the roadmap or backlog.

When describing implementation status, prefer these distinctions over a simple implemented/not-implemented label:

- **Verified behavior:** covered by a test or direct validation.
- **Implemented but unvalidated:** nontrivial source exists, but coverage is incomplete.
- **Temporary/ad hoc implementation:** present to keep the engine moving while the intended rule is unresolved.
- **Explicit placeholder/stub:** source behavior clearly stands in for missing functionality.
- **Missing integration:** a feature exists but is not used by the primary game path.

## Repository orientation

```text
src/core/                  C++17 game core and CMake project
Python Galaxy Tests/       Python/pybind11 galaxy-generation experiments
Technical_Refs/              Supporting technical references
TextAssets/                 Runtime text assets used by the core
START_HERE.md               Current project orientation and documentation map
About.md                    Project background and human-interest context
Master_Plan.md              Roadmap and architectural decisions
Missing_Implementation_List.md  Active implementation backlog
Source_Audit_2026Oct23.md      Dated source audit
Project_Implementation_Log.md   Historical development record
```

Before changing implementation code, consult `Master_Plan.md` for direction, `Missing_Implementation_List.md` for the active backlog, and the latest available source-audit information for the difference between code existence and verified behavior.
