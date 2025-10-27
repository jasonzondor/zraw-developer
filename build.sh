#!/bin/bash

set -e

echo "Building ZRaw Developer..."

# Create build directory
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Configure with CMake
echo "Configuring..."
cmake ..

# Build
echo "Building..."
make -j$(nproc)

echo ""
echo "Build complete! Executable: build/zraw-developer"
echo ""
echo "To install system-wide, run: sudo make install"
