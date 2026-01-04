#!/bin/bash
# Build paq8pxd with both clang and gcc for comparison
# Usage: ./build_compare.sh

set -e

echo "======================================"
echo "Building paq8pxd with multiple compilers"
echo "======================================"

# Build with Clang
echo ""
echo "Building with Clang..."
rm -rf build_clang
cmake -B build_clang -S . \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_C_COMPILER=clang \
    -DUNIX=ON \
    -DNATIVECPU=ON \
    -DCMAKE_BUILD_TYPE=Release \
    > /dev/null 2>&1

cmake --build build_clang -j$(nproc) > /dev/null 2>&1

# Rename binary
mv build_clang/paq8pxd build_clang/paq8pxd_clang

# Build with GCC
echo "Building with GCC..."
rm -rf build_gcc
cmake -B build_gcc -S . \
    -DCMAKE_CXX_COMPILER=g++ \
    -DCMAKE_C_COMPILER=gcc \
    -DUNIX=ON \
    -DNATIVECPU=ON \
    -DCMAKE_BUILD_TYPE=Release \
    > /dev/null 2>&1

cmake --build build_gcc -j$(nproc) > /dev/null 2>&1

# Rename binary
mv build_gcc/paq8pxd build_gcc/paq8pxd_gcc

# Show comparison
echo ""
echo "======================================"
echo "Build Comparison:"
echo "======================================"
ls -lh build_clang/paq8pxd_clang build_gcc/paq8pxd_gcc

echo ""
echo "Clang version:"
clang++ --version | head -1

echo ""
echo "GCC version:"
g++ --version | head -1

echo ""
echo "======================================"
echo "Done! Binaries:"
echo "  build_clang/paq8pxd_clang"
echo "  build_gcc/paq8pxd_gcc"
echo "======================================"
