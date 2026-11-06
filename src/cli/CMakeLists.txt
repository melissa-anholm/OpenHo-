cmake_minimum_required(VERSION 3.10)

project(OpenHoCli LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT TARGET OpenHoCore)
    add_subdirectory(
        "${CMAKE_CURRENT_SOURCE_DIR}/../core"
        "${CMAKE_CURRENT_BINARY_DIR}/OpenHoCore-build"
        EXCLUDE_FROM_ALL
    )
endif()

add_executable(openho_cli
    src/main.cpp
    src/cli_application.cpp
)

target_include_directories(openho_cli PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../core/include"
)

target_link_libraries(openho_cli PRIVATE OpenHoCore)

enable_testing()
add_test(
    NAME openho_cli_economic_smoke
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/test_cli_economic.sh"
            "$<TARGET_FILE:openho_cli>"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../TextAssets"
)
add_test(
    NAME openho_cli_multiplayer_smoke
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/test_cli_multiplayer.sh"
            "$<TARGET_FILE:openho_cli>"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../TextAssets"
)

if(MSVC)
    target_compile_options(openho_cli PRIVATE /W4)
else()
    target_compile_options(openho_cli PRIVATE -Wall -Wextra -Wpedantic)
endif()
