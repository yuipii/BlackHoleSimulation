#!/bin/bash

# Interstellar Black Hole Simulation Runner

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
BINARY="$BUILD_DIR/interstellar_blackhole"

echo "================================================"
echo "   Interstellar Black Hole Simulation"
echo "================================================"

# Check if binary exists
if [ ! -f "$BINARY" ]; then
    echo "Error: Binary not found at $BINARY"
    echo "Please run ./build.sh first to compile the project"
    exit 1
fi

# Check if binary is executable
if [ ! -x "$BINARY" ]; then
    echo "Making binary executable..."
    chmod +x "$BINARY"
fi

# Run the simulation
echo "Starting simulation..."
echo "Controls:"
echo "  WASD - Move"
echo "  Q/E - Up/Down" 
echo "  Mouse - Look around"
echo "  Shift - Fast movement"
echo "  ESC - Exit"
echo "================================================"

cd "$SCRIPT_DIR"
"$BINARY"

EXIT_CODE=$?
if [ $EXIT_CODE -ne 0 ]; then
    echo "Simulation exited with error code: $EXIT_CODE"
    exit $EXIT_CODE
fi

