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
