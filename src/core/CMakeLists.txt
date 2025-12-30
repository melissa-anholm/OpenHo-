cmake_minimum_required(VERSION 3.10)
project(OpenHoCore)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Boost
find_package(Boost REQUIRED COMPONENTS random)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(CORE_SOURCES
	src/rng.cpp
	src/types.cpp
	src/game.cpp
	src/game_formulas.cpp
	src/player.cpp
	src/c_api.cpp
)

# Create the static library
add_library(OpenHoCore STATIC ${CORE_SOURCES})

# Link Boost
target_link_libraries(OpenHoCore PUBLIC Boost::random)

# Set compiler flags for better warnings
if(MSVC)
	target_compile_options(OpenHoCore PRIVATE /W4)
else()
	target_compile_options(OpenHoCore PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Export the library for use by other projects
set(OPENHO_CORE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include PARENT_SCOPE)
set(OPENHO_CORE_LIB OpenHoCore PARENT_SCOPE)
