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
./build.sh                 # Builds openho_cli and OpenHoCore once
./build.sh --tests         # Also builds the core regression executables
```

### Phase 3a: Objective-C++ Bridge

**Build Tool:** CMake 3.10+ (extended)
**Compiler:** Clang with Objective-C++ support
**Current output:** The bridge is currently compiled as part of the minimal `OpenHoMinimalApp` target rather than as a separate library.

```
src/macos/
├── CMakeLists.txt          # Defines the native application target
└── src/
    ├── main.mm             # Minimal NSApplication entry point
    └── ui_bridge.mm        # Objective-C++ compile/link boundary smoke code
```

The current bridge checks that Objective-C++ can include the portable `GameUiAdapter` boundary and that the native target links against `OpenHoCore`. It intentionally does not yet own a full `GameState` controller or expose window/view classes.

**Build process:**
```bash
cmake -S src/macos -B build/macos-native -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=10.14
cmake --build build/macos-native --target OpenHoMinimalApp
```

For the reproducible Linux build, use the OSXCross architecture-specific CMake launcher and keep the Apple SDK/toolchain outside the repository:

```bash
export OSX_CMAKE=/path/to/osxcross/target/bin/x86_64-apple-darwin18-cmake
export BOOST_HEADERS=/path/to/portable/boost-headers

"$OSX_CMAKE" -S src/macos -B build/macos-cross -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_OSX_ARCHITECTURES=x86_64 \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=10.14 \
  -DCMAKE_CXX_FLAGS="-isystem$BOOST_HEADERS" \
  -DCMAKE_OBJCXX_FLAGS="-isystem$BOOST_HEADERS"
cmake --build build/macos-cross --target OpenHoMinimalApp
```

### Phase 3b: Native AppKit Application

**Build Tool:** CMake 3.10+ (extended)
**Compiler:** Clang with Objective-C++ support
**Deployment target:** macOS 10.14 (Mojave) and later
**Current output:** Minimal native macOS application (`OpenHo.app`); interactive application work remains future work.

SwiftUI is intentionally not part of this plan: it is unavailable on macOS 10.14. If Swift is used later for non-UI utility code, it must not raise the application deployment target or become a requirement for the AppKit interface.

```
src/macos/
├── CMakeLists.txt
└── src/
    ├── main.mm             # Current minimal NSApplication entry point
    └── ui_bridge.mm        # Current portable-boundary smoke translation unit
```

The bridge should expose immutable UI snapshots and validated commands. AppKit view and controller code must not retain raw pointers to C++ domain objects.

The eventual application will add window controllers, galaxy views, and planet/player inspectors only after the non-interactive target is replaced by a real UI vertical slice.

**Build process:**
```bash
cmake -S src/macos -B build/macos-native -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=10.14
cmake --build build/macos-native --target OpenHoMinimalApp
```

## Master Build Script

The checked-in `build.sh` configures one shared portable CMake graph at the repository root. `OpenHoCore` is defined once and shared by `openho_cli`, `openho_minimal_game_test`, and `openho_ship_cost_test`; the default build only builds `openho_cli`, while `--tests` also builds the regression executables. It does not silently select an Apple SDK or attempt the macOS application build.

```bash
./build.sh
./build.sh --tests
```

The macOS target is intentionally a separate invocation because it requires either a native Apple toolchain or the externally installed OSXCross toolchain and SDK. This separation keeps ordinary Linux core and CLI builds reproducible and prevents accidental use of host libraries in a Darwin link.

### Portable CLI frontend

The current CLI target lives under `src/cli` and links only against the single `OpenHoCore` target in the shared root graph. It accepts `--players N`, `--planets N`, and `--humans N` at startup; the remaining players are configured as no-action AI players that automatically mark themselves ready. The CLI selects the first human as its initial active player, and `play_as NAME` switches among non-eliminated human players without changing the other players' readiness. The `players` command explicitly displays each player's human/AI type, readiness, elimination state, and active-player marker. Each human player retains an independent pending economic submission while the CLI switches between them.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target openho_cli
./build/openho_cli
```

Configuring `src/cli` directly remains supported for frontend-focused work, but the canonical development path is the root graph. This prevents the core from being compiled once as a standalone project and again as a nested CLI dependency.

The initial CLI is deliberately line-oriented and scriptable. It supports `help`, `status`, `players`, `planets`, `economy`, `set_spending`, `set_savings`, `set_research_budget`, `set_planets`, `set_planet_budget`, `set_research`, `set_research_stream`, `set_planet`, `set_mining`, `set_terraforming`, `dip`, `abandon`, `cancel_abandon`, `ready`, `unready`, `process`, `save`, `load`, and `quit`. Planet-targeting commands use exact planet names, including names containing spaces, rather than internal IDs. The one-parameter top-level commands proportionally rebalance all other top-level buckets; `set_planet_budget` treats its value as that planet's share of the total budget and proportionally rebalances savings, research, and other planets; `set_research_stream` only rebalances the six research-stream fractions; and `set_mining`/`set_terraforming` proportionally rebalance only the selected planet's two-way development split. The full-vector commands remain available for explicit bulk assignment. Economic edits remain pending until `ready`, which automatically submits the complete pending snapshot before marking the local player ready. `unready` leaves the pending snapshot intact; the next `ready` resubmits it. It uses a compact deterministic 10-planet game, with player 1 as the local player and non-local players automatically marked ready. The asset directory can be supplied with `--assets PATH` or `OPENHO_ASSETS_DIR`; the default remains `TextAssets` for repository-root compatibility.

The `openho_cli_economic_smoke` CTest target runs a scripted economic scenario covering pending edits, submission, readiness gating, turn processing, Dip Into Savings, and abandonment queue/cancel. The current command names and argument structures are provisional and are expected to be refined before the CLI becomes a stable user-facing interface.

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
