#!/bin/bash

#!/bin/bash
set -e  # Exit on error

echo "=========================================="
echo "TinyRL-Tetris Setup"
echo "=========================================="

# Detect Python version that uv will use
echo ""
echo "Detecting Python version for uv..."
cd rl
PYTHON_VERSION=$(uv run python --version | grep -oE '[0-9]+\.[0-9]+')
PYTHON_EXEC=$(uv run python -c "import sys; print(sys.executable)")
echo "Found Python ${PYTHON_VERSION} at ${PYTHON_EXEC}"
cd ..

# Build engine with correct Python version
echo ""
echo "Building Tetris engine for Python ${PYTHON_VERSION}..."
cd engine
mkdir -p build
cd build
rm -rf *  # Clean build to avoid version mismatches
Python3_EXECUTABLE="${PYTHON_EXEC}" cmake ..
make
echo "Built: $(ls lib/tinyrl_tetris*.so)"
cd ../..

# Install RL dependencies
echo ""
echo "Installing RL dependencies with uv..."
cd rl
uv sync
cd ..

echo ""
echo "=========================================="
echo "Setup complete!"
echo "=========================================="
echo ""
echo "Python module: engine/build/lib/tinyrl_tetris.cpython-${PYTHON_VERSION/./}-darwin.so"
echo ""
echo "To run RL training:"
echo "  cd rl && uv run python main.py"
echo ""
echo "To run SDL visualization:"
echo "  ./engine/build/bin/tetris_sdl"
echo ""
echo "To run tests:"
echo "  cd tests/build && cmake .. && make && ./bin/run_tests"
echo ""
