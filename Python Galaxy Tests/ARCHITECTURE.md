# OpenHo Galaxy Python Wrapper Architecture

This document records the design information formerly spread across `IMPLEMENTATION_SUMMARY.md` and `MODULE_STRUCTURE.md`. It describes the current wrapper rather than the historical `python/` directory name used by earlier notes.

## Scope

The wrapper is a self-contained development module for generating galaxy coordinates from Python. It links against the OpenHo C++ core but does not create a complete game state, select home planets, or run game turns.

The wrapper is intentionally optional. Removing it should not require changes to the core game's domain model or turn-processing code, apart from removing any wrapper-specific build configuration.

## Directory and target structure

```text
Python Galaxy Tests/
├── openho_galaxy_api.h          C-compatible wrapper interface
├── openho_galaxy_api.cpp        C-compatible wrapper implementation
├── openho_galaxy_bindings.cpp   pybind11 module and Python-facing API
├── CMakeLists.txt               Extension build configuration
├── setup.py                     setuptools-to-CMake integration
├── Usage_Guide.md               Current user and developer guide
├── ARCHITECTURE.md              This design document
├── .gitignore                   Wrapper build-artifact rules
└── examples/
    ├── visualize_galaxy.py     Shape and density visualization
    └── tinier_example.py        Minimal usage example
```

The actual directory name is `Python Galaxy Tests`; older historical documentation referred to it as `python/`.

## Three-layer architecture

```text
Python code
    ↓
pybind11 bindings: openho_galaxy_bindings.cpp
    ↓
C-compatible wrapper: openho_galaxy_api.h/.cpp
    ↓
OpenHo C++ core: Galaxy coordinate generators
```

### Layer 1: Python and pybind11

`openho_galaxy_bindings.cpp` defines the `openho_galaxy` Python module. It:

- Converts shape strings to the wrapper's shape enum.
- Accepts Python arguments and an optional seed.
- Calls the C-compatible wrapper.
- Copies the returned flat coordinate buffer into a NumPy array.
- Frees the C buffer after copying.
- Raises Python exceptions for wrapper errors.
- Exposes the six shape constants and module version.

The Python-facing function is:

```python
generate_coordinates(
    n_planets,
    n_players,
    density,
    shape,
    seed=None,
)
```

### Layer 2: C-compatible wrapper

`openho_galaxy_api.h` defines the small ABI-oriented interface:

- `GalaxyParamsC` for scalar generation parameters.
- Integer shape constants.
- `generate_galaxy_coords()`.
- `free_galaxy_coords()`.
- `get_last_error()`.

`openho_galaxy_api.cpp` performs the conversion from C-compatible values to C++ objects. It:

- Validates positive planet/player counts.
- Validates density and shape ranges.
- Constructs `GalaxyGenerationParams`.
- Constructs a `DeterministicRNG` directly, without a `GameState`.
- Dispatches to the RNG overload for the selected generator.
- Converts `std::vector<PlanetCoord>` into a heap-allocated flat `double` array.
- Stores a human-readable error message when generation fails.

The C wrapper uses a static error buffer. It is not a general thread-safe API; the current Python usage is serialized by the Python GIL as documented by the implementation.

### Layer 3: C++ core

The wrapper links against `OpenHoCore`, built from `src/core/CMakeLists.txt`. It calls the direct-RNG overloads declared in `galaxy.h`:

- `generate_coordinates_random()`
- `generate_coordinates_spiral()`
- `generate_coordinates_circle()`
- `generate_coordinates_ring()`
- `generate_coordinates_cluster()`
- `generate_coordinates_grid()`

This means the wrapper is useful for coordinate experiments but does not necessarily exercise the same sequence as the main `Galaxy` constructor, which also performs naming, planet-parameter generation, home-planet selection, distance-matrix construction, and player assignment.

## Why use a C-compatible boundary?

The C boundary is deliberate even though direct pybind11 bindings to C++ classes would be shorter.

Benefits:

- Python does not depend directly on the layout of C++ domain classes.
- The wrapper exposes only scalar parameters and coordinate buffers.
- C++ implementation changes can remain internal if the small wrapper contract is preserved.
- The same style of boundary may later be useful to Objective-C++ or Swift integration.
- Boost and other C++ implementation details do not leak into the Python API.
- Ownership of the returned buffer is explicit through `free_galaxy_coords()`.

Costs:

- Coordinates are copied from the C buffer into NumPy.
- New features require changes in three places: C header, C implementation, and pybind11 bindings.
- The C buffer/error-buffer design needs care if concurrent use is introduced.

## Build flow

The wrapper has its own CMake project. Its build flow is:

1. Find Python interpreter and development components.
2. Find pybind11, or fetch version `v2.11.1` through CMake FetchContent if unavailable.
3. Find Boost.Random.
4. Add `../src/core` as a subdirectory.
5. Build `OpenHoCore` with position-independent code enabled.
6. Build the `openho_galaxy` Python module from the two wrapper source files.
7. Link the module against `OpenHoCore` and Boost.Random.
8. Place the resulting library in the configured CMake output directory.

The core library uses `POSITION_INDEPENDENT_CODE ON` because a static library linked into a shared Python extension must be compiled as position-independent code on the supported platforms.

## Core/wrapper separation

The core remains responsible for:

- Coordinate algorithms.
- RNG behavior.
- Galaxy-generation parameters.
- Region and spacing helpers.
- Main-game galaxy construction.

The wrapper remains responsible for:

- Python argument conversion.
- C-compatible ownership and error handling.
- NumPy conversion.
- Python-facing documentation and examples.

The wrapper should not acquire game-state responsibilities merely to make coordinate experiments convenient. If home-planet selection or planet-parameter generation is exposed later, those should be explicit additions to the wrapper API rather than hidden side effects of `generate_coordinates()`.

## Adding a wrapper function

For a new Python-visible operation:

1. Add the scalar or buffer-oriented declaration to `openho_galaxy_api.h`.
2. Implement conversion, validation, dispatch, allocation, and error handling in `openho_galaxy_api.cpp`.
3. Add the pybind11-facing function in `openho_galaxy_bindings.cpp`.
4. Add Python docstrings and an example where appropriate.
5. Rebuild the extension with `pip3 install --force-reinstall .`.
6. Run a deterministic smoke check.
7. Update `Usage_Guide.md` and this document if the public contract or architecture changes.

For a new galaxy shape:

1. Add the C++ enum value and generator implementation.
2. Add the direct-RNG overload if standalone use is intended.
3. Add dispatch in the wrapper API.
4. Add the shape string conversion in the pybind11 layer.
5. Add documentation and visualization coverage.
6. Validate exact count, spacing, density behavior, reproducibility, and extreme parameters before calling it complete.

## Current validation status

The wrapper can call all six current coordinate generators. However, source availability is not equivalent to behavioral validation.

Current known limitations include:

- The spiral diagnostic has produced significant over-generation and minimum-spacing violations.
- The wrapper does not select home planets.
- Density behavior is not uniform across shapes.
- The wrapper's coordinate results should be checked for exact count rather than assumed to match the request.
- The main playable path currently prioritizes RANDOM galaxy generation.

The consolidated exploratory diagnostic is `src/core/test_spiral_galaxy.cpp`. It is intentionally outside the normal CMake build until the spiral algorithm has been stabilized enough for a proper automated test.

## Alternatives considered

### Direct pybind11 binding to C++ classes

Rejected for the current wrapper because it would expose implementation details, make C++ refactoring more disruptive to Python, and complicate ownership and Boost-related types.

### Separate wrapper repository

Rejected because it would create version-synchronization problems between galaxy algorithms and visualization tools.

### Mixing Python-specific code into `src/core`

Rejected because it would clutter the core library and make the wrapper harder to remove or build independently.

### Current self-contained directory

Chosen because it keeps wrapper-specific code and documentation together while allowing the core to remain usable without Python.

## Removing the wrapper

If the project no longer needs Python-based galaxy analysis:

1. Remove the `Python Galaxy Tests` directory.
2. Remove its references from project documentation and any top-level build orchestration.
3. Remove or reconsider `POSITION_INDEPENDENT_CODE` in the core CMake configuration if no other shared-library consumer requires it.
4. Keep the core C++ generators and tests that are still useful independently.

## Historical implementation notes

The wrapper was originally documented as a completed three-layer implementation and was historically described using a `python/` directory. Those notes have been consolidated here and corrected to reflect the current directory name and the current experimental status of some generator algorithms.

The wrapper's purpose remains rapid prototyping, visualization, reproducibility checks, and algorithm analysis while the native game remains under development.
