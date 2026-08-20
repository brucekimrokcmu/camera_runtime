#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"
BUILD_TYPE="${1:-Debug}"
GENERATOR="Ninja"

echo "==> Project: ${PROJECT_DIR}"
echo "==> Configuring (${BUILD_TYPE})..."

# Clean build directory if it was previously configured
# with a different CMake generator.
if [[ -f "${BUILD_DIR}/CMakeCache.txt" ]]; then
  EXISTING_GENERATOR="$(
    grep '^CMAKE_GENERATOR:INTERNAL=' "${BUILD_DIR}/CMakeCache.txt" |
      cut -d= -f2-
  )"

  if [[ "${EXISTING_GENERATOR}" != "${GENERATOR}" ]]; then
    echo "==> Generator changed: ${EXISTING_GENERATOR} -> ${GENERATOR}"
    echo "==> Removing old build directory..."
    rm -rf "${BUILD_DIR}"
  fi
fi

cmake \
  -S "${PROJECT_DIR}" \
  -B "${BUILD_DIR}" \
  -G "${GENERATOR}" \
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

echo "==> Compiling..."

cmake --build "${BUILD_DIR}" --parallel "$(nproc)"

# Expose compile database at repo root for host-side clangd.
ln -sfn \
  "${BUILD_DIR}/compile_commands.json" \
  "${PROJECT_DIR}/compile_commands.json"

echo
echo "==> Build complete: ${BUILD_DIR}"
echo "==> compile_commands.json -> build/compile_commands.json"
