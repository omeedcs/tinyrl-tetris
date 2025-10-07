#!/bin/bash

set -e  # Exit on error

echo "=========================================="
echo "TinyRL-Tetris Setup"
echo "=========================================="

# Build engine
echo ""
echo "Building Tetris engine..."
cd engine/build
cmake ..
make

echo ""
echo "=========================================="
echo "Setup complete!"
echo "=========================================="
echo ""
echo "Python module location: engine/build/lib/tinyrl_tetris.*.so"
echo ""
echo "To use in Python:"
echo "  import sys"
echo "  sys.path.insert(0, 'engine/build/lib')"
echo "  import tinyrl_tetris"
echo ""
echo "To run SDL visualization:"
echo "  ./engine/build/bin/tetris_sdl"
echo ""
echo "To run tests:"
echo "  cd tests/build && cmake .. && make && ./bin/run_tests"
echo ""
