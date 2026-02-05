# OpenHo Galaxy Python Bindings

Python bindings for the OpenHo galaxy coordinate generation system. This package allows you to rapidly generate and visualize galaxy layouts using the same algorithms as the OpenHo game engine.

## Features

- **Fast C++ backend**: Uses the same coordinate generation algorithms as the game
- **NumPy integration**: Returns coordinates as NumPy arrays for easy manipulation
- **Multiple galaxy shapes**: RANDOM, SPIRAL, CIRCLE, RING, CLUSTER, GRID
- **Reproducible**: Seed-based generation for consistent results
- **Stable API**: Wrapper insulates Python code from C++ implementation changes

## Requirements

### System Dependencies

- **CMake** 3.10 or higher
- **C++17 compiler** (clang++ recommended on macOS, g++ on Linux)
- **Boost** library (specifically Boost.Random)
- **Python** 3.6 or higher with development headers

### Python Dependencies

- `numpy` >= 1.15.0
- `matplotlib` >= 3.0.0 (optional, for visualization)

## Installation

### Step 1: Install System Dependencies

#### macOS (with MacPorts)

```bash
# Install Xcode Command Line Tools if not already installed
xcode-select --install

# Install MacPorts from https://www.macports.org/install.php

# Install dependencies
sudo port install boost cmake python310
```

#### macOS (with Homebrew - requires macOS 10.15+)

```bash
brew install boost cmake python
```

#### Linux (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install build-essential cmake libboost-all-dev python3-dev
```

### Step 2: Install pybind11

```bash
pip3 install pybind11
```

### Step 3: Build and Install the Package

From the `python/` directory:

```bash
cd python
pip3 install .
```

For development (editable install):

```bash
pip3 install -e .
```

## Usage

### Basic Example

```python
import openho_galaxy as og
import matplotlib.pyplot as plt

# Generate a circular galaxy with 200 planets
coords = og.generate_coordinates(
    n_planets=200,
    n_players=4,
    density=0.5,
    shape='CIRCLE',
    seed=42
)

# coords is a NumPy array of shape (200, 2)
print(coords.shape)  # (200, 2)
print(coords[:3])    # First 3 coordinates

# Visualize
plt.scatter(coords[:, 0], coords[:, 1])
plt.axis('equal')
plt.show()
```

### Available Galaxy Shapes

```python
# Random distribution
coords = og.generate_coordinates(100, 4, 0.5, 'RANDOM', 42)

# Circular distribution
coords = og.generate_coordinates(100, 4, 0.5, 'CIRCLE', 42)

# Ring distribution
coords = og.generate_coordinates(100, 4, 0.5, 'RING', 42)

# Grid distribution
coords = og.generate_coordinates(100, 4, 0.5, 'GRID', 42)

# Spiral distribution
coords = og.generate_coordinates(100, 4, 0.5, 'SPIRAL', 42)

# Cluster distribution
coords = og.generate_coordinates(100, 4, 0.5, 'CLUSTER', 42)
```

### Parameters

- **n_planets** (int): Number of planets to generate (5-500)
- **n_players** (int): Number of players (affects home planet selection)
- **density** (float): Planet distribution density (0.0-1.0)
  - Lower values = more spread out
  - Higher values = more compact
- **shape** (str): Galaxy shape - 'RANDOM', 'SPIRAL', 'CIRCLE', 'RING', 'CLUSTER', or 'GRID'
- **seed** (int): Random seed for reproducible generation

### Running Examples

```bash
cd python/examples
python3 visualize_galaxy.py
```

This will generate several visualizations demonstrating different galaxy shapes and density settings.

## API Stability

The Python API is designed to remain stable even as the underlying C++ implementation evolves:

- **Algorithm changes**: Automatically picked up on rebuild (no API changes needed)
- **New galaxy shapes**: Just add new shape string to the enum
- **Parameter changes**: Would require API update (rare)

When you update the C++ code, simply rebuild the Python package:

```bash
cd python
pip3 install --force-reinstall .
```

## Architecture

The bindings use a three-layer architecture for maximum stability:

```
Python Code
    ↓
pybind11 Bindings (openho_galaxy_bindings.cpp)
    ↓
C API Layer (openho_galaxy_api.h/cpp)
    ↓
C++ Implementation (galaxy.cpp)
```

The C API layer provides a stable interface that rarely changes, even when the C++ implementation is refactored.

## Troubleshooting

### CMake can't find Boost

If CMake fails to find Boost, specify the Boost root directory:

```bash
export BOOST_ROOT=/opt/local  # For MacPorts
# or
export BOOST_ROOT=/usr/local  # For Homebrew
```

Then rebuild:

```bash
pip3 install --force-reinstall .
```

### Compiler not found

Make sure you have a C++17-compatible compiler:

```bash
# Check compiler version
clang++ --version  # macOS
g++ --version      # Linux
```

### Import error: module not found

Make sure the package is installed:

```bash
pip3 list | grep openho
```

If not listed, reinstall:

```bash
cd python
pip3 install .
```

### macOS: "library not loaded" error

This usually means Boost libraries aren't in the library path. Try:

```bash
export DYLD_LIBRARY_PATH=/opt/local/lib:$DYLD_LIBRARY_PATH  # MacPorts
# or
export DYLD_LIBRARY_PATH=/usr/local/lib:$DYLD_LIBRARY_PATH  # Homebrew
```

## Development

### Building for Development

```bash
cd python
pip3 install -e .
```

This creates an "editable" install that reflects C++ changes after rebuilding.

### Running Tests

```bash
cd python
python3 -c "import openho_galaxy as og; print(og.generate_coordinates(10, 2, 0.5, 'GRID', 42))"
```

## License

This package is part of the OpenHo project.

## Contributing

When modifying the C++ code:

1. Update the C API layer only if the interface needs to change
2. Rebuild the Python package: `pip3 install --force-reinstall .`
3. Test with the example scripts: `python3 examples/visualize_galaxy.py`

The Python bindings should continue to work as long as the C API remains stable.
