#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${OPENHO_CORE_BUILD_DIR:-${ROOT_DIR}/src/core/build}"
BUILD_TYPE="${CMAKE_BUILD_TYPE:-Release}"

if command -v getconf >/dev/null 2>&1; then
    JOBS="$(getconf _NPROCESSORS_ONLN 2>/dev/null || true)"
fi
JOBS="${JOBS:-${OPENHO_BUILD_JOBS:-2}}"

printf 'OpenHo core build\n'
printf '  Source: %s\n' "${ROOT_DIR}/src/core"
printf '  Build:  %s\n' "${BUILD_DIR}"
printf '  Type:   %s\n' "${BUILD_TYPE}"
printf '  Jobs:   %s\n' "${JOBS}"

cmake -S "${ROOT_DIR}/src/core" \
      -B "${BUILD_DIR}" \
      -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"
cmake --build "${BUILD_DIR}" --parallel "${JOBS}"

printf 'OpenHo core build complete.\n'
