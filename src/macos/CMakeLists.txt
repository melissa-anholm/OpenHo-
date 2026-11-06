cmake_minimum_required(VERSION 3.10)

project(OpenHoMacOS LANGUAGES CXX OBJCXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OBJCXX_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.14 CACHE STRING "Minimum supported macOS version")
endif()

# Reuse the portable game core.  The core's Boost.Random implementation is
# header-based in this target; do not propagate the host Boost binary that is
# used by the Linux regression build into the Darwin application link.
add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/../core"
    "${CMAKE_CURRENT_BINARY_DIR}/OpenHoCore-build"
    EXCLUDE_FROM_ALL
)
set_property(TARGET OpenHoCore PROPERTY INTERFACE_LINK_LIBRARIES "")

add_executable(OpenHoMinimalApp MACOSX_BUNDLE
    src/main.mm
    src/ui_bridge.mm
)

target_include_directories(OpenHoMinimalApp PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../core/include"
)

target_link_libraries(OpenHoMinimalApp PRIVATE
    OpenHoCore
    "-framework Foundation"
    "-framework AppKit"
)

set_target_properties(OpenHoMinimalApp PROPERTIES
    OUTPUT_NAME "OpenHo"
    MACOSX_BUNDLE_GUI_IDENTIFIER "org.openho.SpacewardHo"
    MACOSX_BUNDLE_BUNDLE_NAME "OpenHo"
)

if(NOT MSVC)
    target_compile_options(OpenHoMinimalApp PRIVATE -Wall -Wextra -Wpedantic)
endif()
