# Python Wrapper Module Structure

This directory contains a **self-contained Python wrapper module** for OpenHo galaxy coordinate generation. All wrapper-specific code lives in this directory, keeping it separate from the core OpenHo C++ library.

## Directory Structure

```
python/
├── openho_galaxy_api.h          # C API header (wrapper interface)
├── openho_galaxy_api.cpp        # C API implementation (wrapper logic)
├── openho_galaxy_bindings.cpp   # pybind11 Python bindings
├── CMakeLists.txt               # Build configuration
├── setup.py                     # Python package configuration
├── README.md                    # Installation and usage guide
├── IMPLEMENTATION_SUMMARY.md    # Technical documentation
├── MODULE_STRUCTURE.md          # This file
├── .gitignore                   # Ignore build artifacts
└── examples/
    └── visualize_galaxy.py      # Example usage and visualization
```

## Design Philosophy

### Self-Contained Module

The wrapper is designed as a **separate module** that:

1. **Lives entirely in `python/`** - No wrapper code mixed into core OpenHo sources
2. **Links to core library** - Uses the existing OpenHoCore static library
3. **Adds minimal overhead** - Just a thin C API layer + Python bindings
4. **Maintains stability** - C API insulates Python from C++ changes

### Why This Structure?

**Separation of Concerns**
- Core OpenHo library (`src/core/`) focuses on game logic
- Python wrapper (`python/`) focuses on Python integration
- Clear boundaries make maintenance easier

**Independent Evolution**
- Core library can change without affecting wrapper structure
- Wrapper can be updated without touching core code
- Easy to add/remove the Python module

**Clean Repository**
- Wrapper code doesn't clutter core source tree
- Build artifacts stay in `python/build/`
- Easy to see what's part of the wrapper vs. core

## How It Works

### Build Process

1. **Core library builds first** (`src/core/`)
   - Creates `libOpenHoCore.a` static library
   - Compiled with `-fPIC` to allow linking into shared libraries

2. **C API wrapper compiles** (`python/openho_galaxy_api.cpp`)
   - Provides stable C interface to core functionality
   - Handles C++ ↔ C type conversions
   - Compiled as part of the Python extension

3. **Python bindings compile** (`python/openho_galaxy_bindings.cpp`)
   - Uses pybind11 to wrap the C API
   - Converts C arrays to NumPy arrays
   - Compiled as part of the Python extension

4. **Extension links** (`openho_galaxy.so`)
   - Links C API + bindings + core library
   - Creates a single Python extension module
   - Installable via pip

### Runtime

```
Python Code
    ↓
openho_galaxy.so (Python extension)
    ├── openho_galaxy_bindings.cpp (pybind11 layer)
    ├── openho_galaxy_api.cpp (C API layer)
    └── libOpenHoCore.a (core library)
            └── galaxy.cpp (actual implementation)
```

## Modifying the Wrapper

### Adding New Functions

1. Add C API function to `openho_galaxy_api.h`
2. Implement it in `openho_galaxy_api.cpp`
3. Expose it in `openho_galaxy_bindings.cpp`
4. Rebuild: `pip3 install --force-reinstall .`

### Updating for Core Changes

If core library changes:

1. **Algorithm changes**: Just rebuild, no wrapper changes needed
2. **New parameters**: Update `GalaxyParamsC` struct in `openho_galaxy_api.h`
3. **New functions**: Add to C API as above

### Removing the Wrapper

To remove the Python wrapper from the project:

1. Delete the `python/` directory
2. Revert the `-fPIC` change in `src/core/CMakeLists.txt` (if desired)
3. Done - no other core code affected

## Core Library Dependency

The wrapper requires **one small change** to the core library:

**`src/core/CMakeLists.txt`**
```cmake
# Enable position-independent code for linking into shared libraries
set_target_properties(OpenHoCore PROPERTIES POSITION_INDEPENDENT_CODE ON)
```

This allows `libOpenHoCore.a` to be linked into shared libraries (like Python extensions). It's a standard practice and has negligible performance impact.

If you remove the Python wrapper, you can optionally revert this change, though leaving it doesn't hurt.

## Benefits of This Approach

✅ **Clean separation** - Wrapper code isolated from core  
✅ **Easy maintenance** - Clear what belongs to wrapper vs. core  
✅ **Optional feature** - Can be removed without affecting core  
✅ **Stable interface** - C API layer protects Python from C++ changes  
✅ **Standard practice** - Follows Python C extension conventions  

## Alternative Approaches Considered

### ❌ Direct pybind11 binding to C++ classes
- **Problem**: Fragile, breaks with C++ refactoring
- **Problem**: Exposes C++ implementation details to Python

### ❌ Separate repository for wrapper
- **Problem**: Version synchronization issues
- **Problem**: Harder to keep in sync with core changes

### ❌ Mixing wrapper code into core sources
- **Problem**: Clutters core library with Python-specific code
- **Problem**: Harder to maintain and remove if needed

### ✅ Self-contained module in `python/` directory (chosen)
- **Benefit**: Clean separation, easy to maintain
- **Benefit**: Lives with the project but doesn't clutter core
- **Benefit**: Easy to remove if no longer needed
