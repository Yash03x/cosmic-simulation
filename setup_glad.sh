#!/bin/bash

# Script to download and setup GLAD for OpenGL 4.1 Core

echo "Setting up GLAD for OpenGL 4.1 Core..."

# Create directories
mkdir -p external/glad/include/glad
mkdir -p external/glad/include/KHR
mkdir -p external/glad/src

# Download GLAD files from GitHub (pre-generated for OpenGL 4.1 Core)
echo "Downloading GLAD files..."

# Download glad.h
curl -L "https://raw.githubusercontent.com/Dav1dde/glad/glad2/include/glad/gl.h" \
  -o external/glad/include/glad/glad.h 2>/dev/null || {
    echo "Failed to download glad.h"
    echo "Please generate GLAD manually from https://glad.dav1d.de/"
    echo "  - GL: Version 4.1, Profile: Core"
    echo "  - Download and extract to external/glad/"
    exit 1
}

# Download khrplatform.h
curl -L "https://raw.githubusercontent.com/Dav1dde/glad/glad2/include/KHR/khrplatform.h" \
  -o external/glad/include/KHR/khrplatform.h 2>/dev/null

# Download glad.c
curl -L "https://raw.githubusercontent.com/Dav1dde/glad/glad2/src/gl.c" \
  -o external/glad/src/glad.c 2>/dev/null

echo "GLAD setup complete!"
echo ""
echo "If download failed, please visit: https://glad.dav1d.de/"
echo "  - API: GL"
echo "  - Version: 4.1"
echo "  - Profile: Core"
echo "  - Extensions: None needed"
echo "  - Options: Generate a loader"
echo ""
echo "Then extract the generated files to external/glad/"
