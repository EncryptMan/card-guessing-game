#!/bin/bash

# build_and_run.sh - Compile and execute C++ files
# Usage: ./build_and_run.sh filename.cpp

# Check if a file argument is provided
if [ $# -eq 0 ]; then
    echo "Usage: $0 <filename.cpp>"
    echo "Example: $0 challenge1.cpp"
    exit 1
fi

# Get the input file
INPUT_FILE="$1"

# Check if the file exists
if [ ! -f "$INPUT_FILE" ]; then
    echo "Error: File '$INPUT_FILE' not found!"
    exit 1
fi

# Check if it's a C++ file
if [[ ! "$INPUT_FILE" =~ \.(cpp|cc|cxx)$ ]]; then
    echo "Error: File must be a C++ source file (.cpp, .cc, or .cxx)"
    exit 1
fi

# Get the filename without extension
FILENAME="${INPUT_FILE%.*}"

# Output executable name
OUTPUT_FILE="$FILENAME"

echo "=========================================="
echo "Compiling: $INPUT_FILE"
echo "Output: $OUTPUT_FILE"
echo "=========================================="

# Compile all .cpp files in the current directory so multiple sources are linked
# This avoids linker errors when the user passes a single source that depends
# on other implementation files in the repo.
SRC_FILES=(./*.cpp)

# Try to locate raylib and obtain flags via pkg-config if available
RAYLIB_FLAGS=""
if command -v pkg-config >/dev/null 2>&1 && pkg-config --exists raylib; then
    RAYLIB_FLAGS="$(pkg-config --cflags --libs raylib)"
else
    # Common Homebrew include paths on macOS
    if [ -f /usr/local/include/raylib.h ] || [ -f /opt/homebrew/include/raylib.h ]; then
        RAYLIB_FLAGS="-lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo"
    else
        echo "Error: raylib not found. Install raylib and try again."
        echo "On macOS with Homebrew: brew install raylib" 
        exit 1
    fi
fi

g++ -std=c++14 -Wall -Wextra ${SRC_FILES[@]} ${RAYLIB_FLAGS} -o "$OUTPUT_FILE"

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo ""
    echo "✓ Compilation successful!"
    echo "=========================================="
    echo "Running: ./$OUTPUT_FILE"
    echo "=========================================="
    echo ""
    
    # Execute the compiled program
    "./$OUTPUT_FILE"
    
    # Capture the exit code
    EXIT_CODE=$?
    
    echo ""
    echo "=========================================="
    echo "Program finished with exit code: $EXIT_CODE"
    echo "=========================================="
else
    echo ""
    echo "✗ Compilation failed!"
    echo "=========================================="
    exit 1
fi
