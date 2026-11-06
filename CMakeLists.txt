cmake_minimum_required(VERSION 3.10)

project(OpenHo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

# The portable development graph. OpenHoCore is defined once and shared by
# the CLI and the core regression executables. The macOS AppKit target is a
# separate project under src/macos because it requires a Darwin toolchain.
add_subdirectory(src/core)
add_subdirectory(src/cli)
