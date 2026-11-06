#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${OPENHO_BUILD_DIR:-${ROOT_DIR}/build}"
LEGACY_CORE_BUILD_DIR="${ROOT_DIR}/src/core/build"
LEGACY_CLI_BUILD_DIR="${ROOT_DIR}/src/cli/build"
PYTHON_BUILD_DIR="${ROOT_DIR}/Python Galaxy Tests/build"

printf 'Cleaning OpenHo build artifacts...\n'
rm -rf "${BUILD_DIR}" \
       "${LEGACY_CORE_BUILD_DIR}" \
       "${LEGACY_CLI_BUILD_DIR}" \
       "${PYTHON_BUILD_DIR}"
printf 'Clean complete.\n'
