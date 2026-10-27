# OpenHo Galaxy Python Wrapper

The `Python Galaxy Tests` directory contains a Python extension for generating and visualizing galaxy coordinate layouts with the same C++ galaxy-generation code used by OpenHo.

This is a development and analysis tool. It currently exposes coordinate generation only; it does not create a complete `GameState`, select home planets, or run turns.

## Features

- C++17 coordinate-generation backend.
- NumPy arrays returned from Python.
- Six currently exposed shapes: `RANDOM`, `SPIRAL`, `CIRCLE`, `RING`, `CLUSTER`, and `GRID`.
- Reproducible generation with an explicit seed.
- Optional visualization through the example scripts.
- A small C-compatible wrapper between the C++ core and pybind11.

## Current limitations

The wrapper exposes the standalone RNG overloads of the coordinate generators. It does not exercise every behavior of the main `Galaxy`/`GameState` constructor path.

In particular:

- The spiral generator is still experimental. A diagnostic run requesting 150 planets produced 761 coordinates and minimum-spacing violations, so its output requires further work.
- Coordinate generation does not perform main-game home-planet selection.
- The `n_players` parameter influences generator behavior, especially spiral arms and clusters, but the wrapper does not return player assignments.
- Density semantics are shape-specific and remain subject to validation.
- The wrapper must be rebuilt after C++ changes.

For the first playable game path, the project currently uses the RANDOM generator rather than relying on the experimental spiral path.

## Requirements

### System dependencies

- CMake 3.10 or newer.
- A C++17 compiler.
- Boost.Random.
- Python 3.6 or newer with development headers.
- pybind11, either installed in the Python environment or fetched by CMake.

### Python dependencies

- NumPy >= 1.15.0.
- Matplotlib >= 3.0.0 for visualization examples.

## Installation and build

Run the commands from the repository root:

```bash
cd "Python Galaxy Tests"
pip3 install .
```

For an editable development installation:

```bash
cd "Python Galaxy Tests"
pip3 install -e .
```

The package's CMake configuration builds `OpenHoCore` as a subdirectory, compiles the wrapper sources, and produces the `openho_galaxy` extension module.

On Ubuntu/Debian, the core system dependencies can be installed with:

```bash
sudo apt-get update
sudo apt-get install build-essential cmake libboost-random-dev python3-dev
pip3 install pybind11 numpy
```

On macOS, install a C++17 toolchain, CMake, Boost, and Python using the package manager appropriate to the system, then install pybind11 and NumPy with `pip3`.

## Basic usage

```python
import openho_galaxy as og

coords = og.generate_coordinates(
    n_planets=100,
    n_players=4,
    density=0.5,
    shape="RANDOM",
    seed=42,
)

print(coords.shape)  # (number_generated, 2)
print(coords[:3])    # x/y coordinate pairs
```

The return value is a NumPy array with two columns: `x` and `y`. The number of returned coordinates should be checked rather than assumed to equal the requested count, especially for experimental shapes.

### Available shapes

```python
og.generate_coordinates(100, 4, 0.5, "RANDOM", 42)
og.generate_coordinates(100, 4, 0.5, "CIRCLE", 42)
og.generate_coordinates(100, 4, 0.5, "RING", 42)
og.generate_coordinates(100, 4, 0.5, "GRID", 42)
og.generate_coordinates(100, 4, 0.5, "SPIRAL", 42)
og.generate_coordinates(100, 4, 0.5, "CLUSTER", 42)
```

### Parameters

- `n_planets`: Requested number of planets. The wrapper rejects zero; practical tested values are currently in the approximate range 5–500.
- `n_players`: Positive player count used by shape algorithms. It does not assign home planets in this wrapper.
- `density`: Must be greater than 0 and no greater than 1 for the wrapper API. Shape-specific interpretation varies.
- `shape`: One of `RANDOM`, `SPIRAL`, `CIRCLE`, `RING`, `CLUSTER`, or `GRID`.
- `seed`: Optional integer seed. The same inputs and seed should reproduce the same output.

Invalid parameters raise a Python exception containing the C++ wrapper's error message.

## Visualization examples

The main example compares shapes and explores density settings:

```bash
cd "Python Galaxy Tests"
python3 examples/visualize_galaxy.py
```

A small programmatic example is available at:

```bash
cd "Python Galaxy Tests"
python3 examples/tinier_example.py
```

The visualization script requires Matplotlib. The coordinate API itself only requires the extension's runtime dependencies.

## API reference

The Python module exposes:

```python
generate_coordinates(
    n_planets: int,
    n_players: int,
    density: float,
    shape: str,
    seed: int | None = None,
) -> numpy.ndarray
```

The module also exposes the string constants `RANDOM`, `SPIRAL`, `CIRCLE`, `RING`, `CLUSTER`, and `GRID`, plus `__version__`.

## Rebuilding after core changes

The extension contains compiled C++ code. Rebuild it after changes to the core or wrapper:

```bash
cd "Python Galaxy Tests"
pip3 install --force-reinstall .
```

For a development install:

```bash
cd "Python Galaxy Tests"
pip3 install -e .
```

## Troubleshooting

### CMake cannot find Boost

Set `BOOST_ROOT` to the Boost installation prefix and rebuild. Examples include `/opt/local` for MacPorts or `/usr/local` for a Homebrew-style installation.

### CMake cannot find pybind11

Install it into the active Python environment:

```bash
pip3 install pybind11
```

The wrapper CMake configuration also has a FetchContent fallback, which may download pybind11 during configuration.

### The module cannot be imported

Confirm that the package was installed into the active Python environment:

```bash
python3 -c "import sys; print(sys.executable)"
pip3 show openho-galaxy
python3 -c "import openho_galaxy as og; print(og.__version__)"
```

### Text assets cannot be found

The coordinate-only wrapper does not require the game's `TextAssets` directory. The full C++ game does use text assets and should be run from the repository root or supplied with an appropriate asset path.

## Development smoke check

After installation, run a deterministic coordinate call:

```bash
cd "Python Galaxy Tests"
python3 -c "import openho_galaxy as og; print(og.generate_coordinates(10, 2, 0.5, 'GRID', 42))"
```

For the core game's current minimal playable smoke test, use the separate CMake build under `src/core`; the Python wrapper is not a replacement for that test.

## Architecture overview

The wrapper uses three layers:

```text
Python code
    ↓
pybind11 bindings
    ↓
C-compatible wrapper
    ↓
OpenHo C++ galaxy-generation implementation
```

The rationale and maintenance procedures for these layers are documented in [ARCHITECTURE.md](ARCHITECTURE.md).

## Future enhancements

Possible future wrapper work includes:

- Exposing home-planet selection.
- Exposing planet-parameter generation.
- Adding batch generation.
- Adding distance and spacing analysis helpers.
- Adding structured diagnostics for exact-count and minimum-spacing checks.
- Adding optional visual overlays for clusters, spiral arms, and selected homes.

## Contributing changes

When changing the wrapper:

1. Update the C API header if the wrapper interface changes.
2. Implement the C API function in `openho_galaxy_api.cpp`.
3. Expose it through `openho_galaxy_bindings.cpp`.
4. Rebuild the package.
5. Run a deterministic smoke call and, when relevant, the visualization examples.
6. Update this README or `ARCHITECTURE.md` when behavior or design changes.
