#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${OPENHO_BUILD_DIR:-${ROOT_DIR}/build}"
BUILD_TYPE="${CMAKE_BUILD_TYPE:-Release}"
BUILD_TESTS=0

usage() {
    cat <<EOF
Usage: ./build.sh [--tests]

Build the portable OpenHo CLI through the shared top-level CMake graph.

Options:
  --tests    Also build the core regression executables.
  --help     Show this help.

Environment:
  OPENHO_BUILD_DIR   Build directory (default: ./build)
  CMAKE_BUILD_TYPE   CMake build type (default: Release)
  OPENHO_BUILD_JOBS  Parallel build jobs when getconf is unavailable
EOF
}

while (($# > 0)); do
    case "$1" in
        --tests)
            BUILD_TESTS=1
            ;;
        --help|-h)
            usage
            exit 0
            ;;
        *)
            printf 'Unknown option: %s\n\n' "$1" >&2
            usage >&2
            exit 2
            ;;
    esac
    shift
done

if command -v getconf >/dev/null 2>&1; then
    JOBS="$(getconf _NPROCESSORS_ONLN 2>/dev/null || true)"
fi
JOBS="${JOBS:-${OPENHO_BUILD_JOBS:-2}}"

printf 'OpenHo portable build\n'
printf '  Source: %s\n' "${ROOT_DIR}"
printf '  Build:  %s\n' "${BUILD_DIR}"
printf '  Type:   %s\n' "${BUILD_TYPE}"
printf '  Jobs:   %s\n' "${JOBS}"
printf '  Tests:  %s\n' "$([ "$BUILD_TESTS" -eq 1 ] && printf enabled || printf disabled)"

cmake -S "${ROOT_DIR}" \
      -B "${BUILD_DIR}" \
      -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"

TARGETS=(openho_cli)
if ((BUILD_TESTS)); then
    TARGETS+=(openho_minimal_game_test openho_ship_cost_test)
fi

cmake --build "${BUILD_DIR}" --target "${TARGETS[@]}" --parallel "${JOBS}"
printf 'OpenHo portable build complete.\n'
