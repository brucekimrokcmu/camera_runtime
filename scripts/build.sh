#!/usr/bin/env bash
set -e

BUILD_DIR="/workspace/build"
BUILD_TYPE="${1:-Debug}"

echo "==> Setting up build directory (${BUILD_TYPE})..."
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

echo "==> Configuring with CMake..."
cmake -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" ..

echo "==> Compiling project..."
make -j$(nproc)

echo ""
echo "Build complete! Executables built in: ${BUILD_DIR}"
