#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CORE_BUILD_DIR="${OPENHO_CORE_BUILD_DIR:-${ROOT_DIR}/src/core/build}"
PYTHON_BUILD_DIR="${ROOT_DIR}/Python Galaxy Tests/build"

printf 'Cleaning OpenHo build artifacts...\n'
rm -rf "${CORE_BUILD_DIR}" "${PYTHON_BUILD_DIR}"
printf 'Clean complete.\n'
