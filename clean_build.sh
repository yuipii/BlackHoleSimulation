#!/bin/bash

# Clean build directory

echo "Cleaning build directory..."

if [ -d "build" ]; then
    rm -rf build/*
    echo "Build directory cleaned"
else
    echo "Build directory doesn't exist"
fi

# Remove run scripts
if [ -f "run_simulation.sh" ]; then
    rm run_simulation.sh
    echo "Removed run_simulation.sh"
fi

if [ -f "clean_build.sh" ]; then
    rm clean_build.sh
    echo "Removed clean_build.sh"
fi

echo "Cleanup complete"

