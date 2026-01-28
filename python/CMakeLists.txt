cmake_minimum_required(VERSION 3.10)
project(openho_galaxy_python)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Python
find_package(Python COMPONENTS Interpreter Development REQUIRED)

# Find pybind11
find_package(pybind11 CONFIG)
if(NOT pybind11_FOUND)
	# Try to download pybind11 if not found
	include(FetchContent)
	FetchContent_Declare(
		pybind11
		GIT_REPOSITORY https://github.com/pybind/pybind11.git
		GIT_TAG v2.11.1
	)
	FetchContent_MakeAvailable(pybind11)
endif()

# Find Boost
find_package(Boost REQUIRED COMPONENTS random)

# Build the core library first
add_subdirectory(../src/core ${CMAKE_CURRENT_BINARY_DIR}/core)

# Create Python extension module
pybind11_add_module(openho_galaxy openho_galaxy_bindings.cpp)

# Link against the core library
target_link_libraries(openho_galaxy PRIVATE OpenHoCore Boost::random)

# Include directories
target_include_directories(openho_galaxy PRIVATE
	../src/core/include
	${Boost_INCLUDE_DIRS}
)

# Set output directory
set_target_properties(openho_galaxy PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
)
