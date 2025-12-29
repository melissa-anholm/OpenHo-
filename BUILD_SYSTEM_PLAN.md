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
enable_language(OBJC)
set(CMAKE_OBJC_COMPILER clang)

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

### Phase 3b: SwiftUI Application

**Build Tool:** Swift Package Manager (SPM)
**Compiler:** Swift compiler (swiftc)
**Output:** Executable macOS app (`OpenHo.app`)

```
macos-app/
├── Package.swift           # SPM manifest
├── Sources/
│   └── OpenHo/
│       ├── main.swift      # App entry point
│       ├── ContentView.swift
│       ├── GameViewController.swift
│       ├── Models/
│       ├── Views/
│       └── ...
└── Tests/
    └── OpenHoTests/
```

**Package.swift structure:**
```swift
// swift-tools-version:5.5
import PackageDescription

let package = Package(
    name: "OpenHo",
    platforms: [
        .macOS(.v10_14)
    ],
    products: [
        .executable(name: "OpenHo", targets: ["OpenHo"])
    ],
    targets: [
        .executableTarget(
            name: "OpenHo",
            dependencies: ["OpenHoMacOS"],
            swiftSettings: [
                .unsafeFlags(["-suppress-warnings"])
            ]
        ),
        // Bridge target linking C++ libraries
        .target(
            name: "OpenHoMacOS",
            dependencies: [],
            linkerSettings: [
                .linkedLibrary("OpenHoCore"),
                .linkedLibrary("OpenHoMacOS"),
                .linkedFramework("Foundation"),
                .linkedFramework("AppKit")
            ]
        )
    ]
)
```

**Build process:**
```bash
cd macos-app
swift build -c release
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

# Phase 3b: Build SwiftUI Application (if exists)
if [ -d "macos-app" ]; then
    echo -e "${BLUE}Building SwiftUI Application...${NC}"
    cd macos-app
    swift build -c release
    cd ..
    echo -e "${GREEN}✓ SwiftUI Application built${NC}"
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
- **Swift Standard Library** - Included with Swift compiler
- **SwiftUI** - Included with macOS 10.14+

## Build Configurations

### Debug Build
```bash
# Phase 2
cmake .. -DCMAKE_BUILD_TYPE=Debug
make

# Phase 3b
swift build
```

### Release Build
```bash
# Phase 2
cmake .. -DCMAKE_BUILD_TYPE=Release
make

# Phase 3b
swift build -c release
```

### Optimized Build
```bash
# Phase 2
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3"
make

# Phase 3b
swift build -c release -Xswiftc -Osize
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
rm -rf macos-app/.build
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

### Phase 3b: Swift Unit Tests
```bash
cd macos-app
swift test
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
cp -r macos-app/.build/release/OpenHo.app /Applications/

# Or create a symlink for development
ln -s $(pwd)/macos-app/.build/release/OpenHo.app /Applications/OpenHo
```

## Development Workflow

1. **Edit code** in any editor (VS Code, Vim, etc.)
2. **Build** with `./build.sh` or individual phase builds
3. **Test** with `./test.sh`
4. **Run** with `./macos-app/.build/release/OpenHo.app/Contents/MacOS/OpenHo`
5. **Debug** with `lldb` command-line debugger

## Debugging

### Debug C++ Core
```bash
cd src/core/build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
lldb ./libOpenHoCore.a
```

### Debug Swift Application
```bash
cd macos-app
swift build
lldb .build/debug/OpenHo
```

## Cross-Platform Considerations

This build system is designed for macOS but could be extended to support:
- **Linux:** Use same CMake/Swift build process
- **Windows:** Requires MSVC compiler and different framework linking

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
