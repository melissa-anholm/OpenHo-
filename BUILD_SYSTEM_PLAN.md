# OpenHo Build System Plan

This document describes the command-line build system architecture for the OpenHo project, enabling development without Xcode while maintaining compatibility with macOS Mojave and later.

## Philosophy

- **No Xcode dependency:** All builds use command-line tools
- **Reproducible builds:** Same build on any machine with same tools
- **Modular compilation:** Each layer builds independently
- **CI/CD friendly:** Automated testing and deployment
- **Editor agnostic:** Works with any text editor or IDE

## Build System Architecture

### Phase 2: C++ Core (Current)

**Build Tool:** CMake 3.10+
**Compiler:** Clang (via Xcode Command Line Tools)
**Output:** Static library (`libOpenHoCore.a`)

```
src/core/
├── CMakeLists.txt          # Defines library target
├── include/                # Public headers
└── src/                    # Implementation files
```

**Build process:**
```bash
cd src/core/build
cmake ..
make -j$(sysctl -n hw.ncpu)
```

### Phase 3a: Objective-C++ Bridge

**Build Tool:** CMake 3.10+ (extended)
**Compiler:** Clang with Objective-C++ support
**Output:** Static library (`libOpenHoMacOS.a`)

```
src/macos/
├── CMakeLists.txt          # Defines bridge library
├── include/
│   ├── OpenHoMacOS.h       # Objective-C++ wrapper classes
│   └── GameBridge.h
└── src/
    ├── OpenHoMacOS.mm      # Implementation
    └── GameBridge.mm
```

**CMakeLists.txt additions:**
```cmake
# Enable Objective-C++
enable_language(OBJCXX)
set(CMAKE_OBJCXX_COMPILER clang++)

# Link against macOS frameworks
target_link_libraries(OpenHoMacOS PUBLIC
    "-framework Foundation"
    "-framework AppKit"
)

# Link against C++ core
target_link_libraries(OpenHoMacOS PUBLIC OpenHoCore)
```

**Build process:**
```bash
cd src/macos/build
cmake ..
make -j$(sysctl -n hw.ncpu)
```

### Phase 3b: Native AppKit Application

**Build Tool:** CMake 3.10+ (extended)
**Compiler:** Clang with Objective-C++ support
**Deployment target:** macOS 10.14 (Mojave) and later
**Output:** Native macOS application (`OpenHo.app`)

SwiftUI is intentionally not part of this plan: it is unavailable on macOS 10.14. If Swift is used later for non-UI utility code, it must not raise the application deployment target or become a requirement for the AppKit interface.

```
src/macos/
├── CMakeLists.txt
├── include/
│   ├── OpenHoMacOS.h       # Objective-C++ bridge declarations
│   └── GameBridge.h        # Stable UI-facing value/command boundary
└── src/
    ├── main.mm             # NSApplication entry point
    ├── GameBridge.mm       # Owns and adapts GameState
    ├── MainWindowController.mm
    ├── GalaxyView.mm
    └── PlanetInspectorViewController.mm
```

The bridge should expose immutable UI snapshots and validated commands. AppKit view and controller code must not retain raw pointers to C++ domain objects.

**Build process:**
```bash
cd src/macos/build
cmake .. -DCMAKE_OSX_DEPLOYMENT_TARGET=10.14
make -j$(sysctl -n hw.ncpu)
```

## Master Build Script

A top-level `build.sh` script orchestrates the entire build:

```bash
#!/bin/bash
set -e

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}OpenHo Build System${NC}"

# Phase 2: Build C++ Core
echo -e "${BLUE}Building C++ Core...${NC}"
mkdir -p src/core/build
cd src/core/build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)
cd ../../..
echo -e "${GREEN}✓ C++ Core built${NC}"

# Phase 3a: Build Objective-C++ Bridge (if exists)
if [ -d "src/macos" ]; then
    echo -e "${BLUE}Building Objective-C++ Bridge...${NC}"
    mkdir -p src/macos/build
    cd src/macos/build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j$(sysctl -n hw.ncpu)
    cd ../../..
    echo -e "${GREEN}✓ Objective-C++ Bridge built${NC}"
fi

# Phase 3b: Build native AppKit application (if exists)
if [ -d "src/macos" ]; then
    echo -e "${BLUE}Building native AppKit application...${NC}"
    mkdir -p src/macos/build
    cd src/macos/build
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_DEPLOYMENT_TARGET=10.14
    make -j$(sysctl -n hw.ncpu)
    cd ../../..
    echo -e "${GREEN}✓ Native AppKit application built${NC}"
fi

echo -e "${GREEN}Build complete!${NC}"
```

## Dependency Management

### Phase 2 Dependencies
- **Boost.Random** - Via Homebrew or system package manager
  ```bash
  brew install boost
  ```

### Phase 3a Dependencies
- **macOS Frameworks** - Included with Xcode Command Line Tools
  - Foundation
  - AppKit
  - Objective-C runtime

### Phase 3b Dependencies
- **Foundation** - Included with macOS
- **AppKit** - Included with macOS 10.14+
- **Objective-C runtime** - Included with the system toolchain

## Build Configurations

### Debug Build
```bash
# Phase 2
cmake .. -DCMAKE_BUILD_TYPE=Debug
make

# Phase 3b
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_OSX_DEPLOYMENT_TARGET=10.14
make
```

### Release Build
```bash
# Phase 2
cmake .. -DCMAKE_BUILD_TYPE=Release
make

# Phase 3b
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_DEPLOYMENT_TARGET=10.14
make
```

### Optimized Build
```bash
# Phase 2
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3"
make

# Phase 3b
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_DEPLOYMENT_TARGET=10.14 -DCMAKE_CXX_FLAGS="-O3"
make
```

## Clean and Rebuild

```bash
# Clean all build artifacts
./clean.sh

# Rebuild from scratch
./build.sh
```

**clean.sh:**
```bash
#!/bin/bash
rm -rf src/core/build
rm -rf src/macos/build
echo "Clean complete"
```

## Testing Strategy

### Phase 2: C++ Unit Tests
```bash
# Add to src/core/CMakeLists.txt
enable_testing()
add_executable(CoreTests tests/rng_test.cpp tests/game_test.cpp)
target_link_libraries(CoreTests OpenHoCore)
add_test(NAME CoreTests COMMAND CoreTests)

# Run tests
cd src/core/build
ctest
```

### Phase 3b: Native UI Tests
```bash
cd src/macos/build
ctest
```

## Continuous Integration

### GitHub Actions Workflow

```yaml
name: Build

on: [push, pull_request]

jobs:
  build:
    runs-on: macos-latest
    steps:
      - uses: actions/checkout@v2
      - name: Install dependencies
        run: brew install boost
      - name: Build
        run: ./build.sh
      - name: Run tests
        run: ./test.sh
```

## Installation

After successful build, the application can be installed:

```bash
# Copy to Applications folder
cp -r src/macos/build/OpenHo.app /Applications/

# Or create a symlink for development
ln -s $(pwd)/src/macos/build/OpenHo.app /Applications/OpenHo
```

## Development Workflow

1. **Edit code** in any editor (VS Code, Vim, etc.)
2. **Build** with `./build.sh` or individual phase builds
3. **Test** with `./test.sh`
4. **Run** with `./src/macos/build/OpenHo.app/Contents/MacOS/OpenHo`
5. **Debug** with `lldb` command-line debugger

## Debugging

### Debug C++ Core
```bash
cd src/core/build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
lldb ./libOpenHoCore.a
```

### Debug Native AppKit Application
```bash
cd src/macos/build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_OSX_DEPLOYMENT_TARGET=10.14
make
lldb ./OpenHo.app/Contents/MacOS/OpenHo
```

## Cross-Platform Considerations

The C++ core is intended to remain portable across macOS, Linux, and Windows. Future ports should add a platform adapter and UI target rather than introducing platform frameworks into `src/core`:

- **macOS:** Objective-C++ and AppKit, with macOS-specific filesystem and application-lifecycle code confined to `src/macos`.
- **Linux:** A future adapter/UI target using a framework selected when the port is undertaken; the core can continue to use CMake.
- **Windows:** A future adapter/UI target using a framework selected when the port is undertaken; the core can continue to use CMake with the appropriate compiler.

The core/public boundary should exchange value-oriented snapshots and validated commands. It should not expose UI objects, platform handles, or ownership of internal domain objects to a frontend. The save format must remain an explicit, versioned, checksum-protected format independent of C++ object layout, pointers, host endianness, or platform-specific paths.

## Future Enhancements

- [ ] Automated code formatting (clang-format, swiftformat)
- [ ] Static analysis (clang-tidy, swiftlint)
- [ ] Code coverage reporting
- [ ] Performance profiling integration
- [ ] Automated release builds and packaging

## Summary

This command-line build system provides:
- ✅ No Xcode dependency
- ✅ Reproducible builds
- ✅ Modular compilation
- ✅ CI/CD friendly
- ✅ Editor agnostic
- ✅ Scalable from Phase 2 to Phase 3
