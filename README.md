# TinyRL-Tetris

A from-scratch reinforcement learning training system with custom CUDA kernels that trains an AI agent to play Tetris at expert level.

## 🎯 Project Overview

TinyRL-Tetris is a high-performance RL training framework built from scratch using C++, CUDA, and Python. The goal is to train an AI agent to play Tetris at expert level.

### Key Features

- **Optimized Tetris Engine**: High-performance C++ implementation with SDL2 visualization
- **RL-Ready API**: Gym-like interface for training agents
- **Custom CUDA Kernels**: Neural network operations and RL algorithms in pure CUDA (planned)
- **Massively Parallel**: 1000+ parallel Tetris games on GPU (planned)

## 🏗️ Architecture

### Core Components

#### 1. Tetris Game Engine (`engine/`)
- Fully functional Tetris implementation in C++
- SDL2 renderer for visualization
- Headless mode for training

**Files:**
- `tetrisGame.cpp` - Core game logic
- `main_sdl.cpp` - SDL2 visualization version
- `main.cpp` - Headless version

<!-- 
#### 2. RL Training Framework (`rl/`) - TODO
- PPO (Proximal Policy Optimization) from scratch
- Custom CUDA kernels for all operations

#### 3. Training System (`training/`) - TODO
- Training loop coordinator
- Experience replay buffer on GPU
- Checkpointing and model persistence

#### 4. Evaluation & Visualization (`evaluation/`) - TODO
- Benchmark suite comparing to baselines
- Training curve visualizations
-->

## 🚀 Getting Started

### Prerequisites

- CMake 3.10+
- C++17 compiler
- SDL2 and SDL2_ttf libraries
- (Optional) CUDA Toolkit 11.8+ for future GPU training

### Building and Running

```bash
# Build the SDL2 visualization version
cd engine/build
cmake ..
make

# Run the game
./bin/tetris_sdl
```

**Controls:**
- A/D or Arrow Keys - Move left/right
- W or Up - Rotate clockwise
- E - Rotate counter-clockwise
- S or Down - Soft drop
- Space - Hard drop
- C - Hold piece
- R - Reset game
- ESC - Quit

### Running Tests

```bash
cd tests/build
cmake ..
make
./bin/run_tests
```

## 🐍 Python API for RL Training

### Quick Setup

```bash
./setup.sh  # Builds everything and sets up Python module
```

### Manual Setup

```bash
cd engine/build
cmake ..
make
# Module will be at: lib/tinyrl_tetris.*.so
```

### Environment Interface

```python
import sys
sys.path.insert(0, 'engine/build/lib')
import tinyrl_tetris

# Create environment
env = tinyrl_tetris.TetrisEnv(tinyrl_tetris.REALTIME, queue_size=3)

# Reset - returns observation dict
obs = env.reset()

# Step - returns (obs, reward, done, info)
obs, reward, done, info = env.step(tinyrl_tetris.LEFT)

# Access game state
print(env.score)
print(env.game_over)
```

**Observation Structure:**
```python
obs = {
    'board': np.ndarray (24, 18),        # Board state (0=empty, 1-7=piece types)
    'active_piece': np.ndarray (24, 18), # Current falling piece (binary mask)
    'queue': np.ndarray (12, 4),         # Next 3 pieces (queue_size * 4x4)
    'holder': np.ndarray (4, 4)          # Held piece
}
```

**Actions:**
```python
tinyrl_tetris.LEFT     # 0 - Move left
tinyrl_tetris.RIGHT    # 1 - Move right
tinyrl_tetris.DOWN     # 2 - Soft drop
tinyrl_tetris.CW       # 3 - Rotate clockwise
tinyrl_tetris.CCW      # 4 - Rotate counter-clockwise
tinyrl_tetris.DROP     # 5 - Hard drop
tinyrl_tetris.SWAP     # 6 - Hold/swap piece
tinyrl_tetris.NOOP     # 7 - No operation
```

**Example: Random Agent**
```python
import numpy as np

env = tinyrl_tetris.TetrisEnv(tinyrl_tetris.REALTIME, queue_size=3)
obs = env.reset()

while not env.game_over:
    action = np.random.randint(0, 8)
    obs, reward, done, info = env.step(action)
    print(f"Score: {env.score}, Reward: {reward}")
```

## ⚡ Performance Benchmarks

The custom TinyRL C++ Tetris engine significantly outperforms existing Python implementations:

| Environment | Steps/sec | Speedup |
|------------|-----------|---------|
| **TinyRL C++ Engine** | **35,089** | **baseline** |
| TinyRL + Gym Wrapper | 32,366 | 0.92x |
| Tetris-Gymnasium (Python) | 10,787 | 0.31x |

**Key Findings:**
- TinyRL is **3.3x faster** than Tetris-Gymnasium
- Gymnasium wrapper adds only ~8% overhead
- Critical for RL training with millions of steps

### Running Benchmarks

```bash
cd rl
uv run python benchmark_envs.py
```

The benchmark compares:
1. Raw C++ engine performance
2. C++ engine with Gymnasium wrapper (used for training)
3. Tetris-Gymnasium Python implementation

## 📈 Roadmap

- [x] Tetris game engine implementation
- [x] SDL2 visualization
- [x] Comprehensive test suite
- [x] Python bindings for RL training
- [x] Performance benchmarks vs existing implementations
- [ ] Train baseline PPO agent
- [ ] Custom CUDA kernels for parallel simulation
- [ ] Multi-GPU training support

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## 📝 License

MIT License - see LICENSE file for details

## 📧 Contact

Questions? Open an issue or reach out to [laithaustin@utexas.edu]

---

**Note**: This is a research/educational project. The goal is to understand RL and CUDA programming at a fundamental level by building everything from scratch.
