#!/bin/bash

# Linux compile script for MATERIAL Framework

echo "Compiling Linux EdgeSys Base Application..."

# Clean previous build
make clean

# Build the application
make all

if [ $? -eq 0 ]; then
    echo "Compilation successful!"
    echo "Executable: build/Linux_EdgeSys_Base"
else
    echo "Compilation failed!"
    exit 1
fi