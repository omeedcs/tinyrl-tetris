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

## 🤖 RL Training API

The Tetris engine exposes a Gym-like interface for training RL agents:

```cpp
// C++ API
TetrisGame game(TimeManager::REALTIME, queue_size=3);

// Reset environment
game.reset();

// Step with action
StepResult result = game.step(action);
// result.obs - Current observation (board, active piece, queue, holder)
// result.reward - Reward for this step
// result.terminated - Whether game is over

// Get observation
Observation obs = game.obs;
// obs.board - 18x24 grid with piece types (0=empty, 1-7=piece types)
// obs.active_tetromino - 18x24 binary mask of current falling piece
// obs.queue - Upcoming pieces (queue_size * 4x4 grids)
// obs.holder - Held piece (4x4 grid)
```

**Actions:**
- `LEFT` (0) - Move left
- `RIGHT` (1) - Move right  
- `DOWN` (2) - Soft drop
- `CW` (3) - Rotate clockwise
- `CCW` (4) - Rotate counter-clockwise
- `DROP` (5) - Hard drop
- `SWAP` (6) - Hold/swap piece
- `NOOP` (7) - No operation

**Rewards:**
- Line clears: Points based on standard Tetris scoring
- Game over: Negative reward
- Height penalty: Optional (configurable)

### Python Bindings (Planned)

```python
import tinyrl_tetris

env = tinyrl_tetris.TetrisEnv(queue_size=3)
obs = env.reset()

for _ in range(1000):
    action = agent.get_action(obs)
    obs, reward, done, info = env.step(action)
    
    if done:
        obs = env.reset()
```

## 📈 Roadmap

- [x] Tetris game engine implementation
- [x] SDL2 visualization
- [x] Comprehensive test suite
- [ ] Python bindings for RL training
- [ ] PPO implementation
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
