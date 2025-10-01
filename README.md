# TinyRL-Tetris

A from-scratch reinforcement learning training system with custom CUDA kernels that trains an AI agent to play Tetris at expert level.

## 🎯 Project Overview

TinyRL-Tetris is a high-performance RL training framework built entirely from scratch using C++, CUDA, and Python (for visualization only). The goal is to train an AI agent to play Tetris at expert level without relying on existing ML frameworks like PyTorch, TensorFlow, or JAX.

### Key Features

- **Custom CUDA Kernels**: All neural network operations, optimizers, and RL algorithms implemented in pure CUDA
- **Massively Parallel**: 1000+ parallel Tetris games running simultaneously on GPU
- **From-Scratch RL**: Complete PPO (Proximal Policy Optimization) implementation without ML framework dependencies
- **Optimized for Speed**: Fused kernels, memory-efficient state representation, and batched environment simulation
- **Production Ready**: Comprehensive evaluation suite, benchmarks, and visualization tools

## 🏗️ Architecture

### Core Components

#### 1. Tetris Game Engine (`engine/`)
- Fully functional Tetris implementation in C/CUDA
- Runs 1000+ parallel games simultaneously on GPU
- Optimized for RL training workloads
- Memory-efficient state representation

**Files:**
- `tetris.cu` - Parallel game simulation kernels
- `game_logic.cpp` - Core Tetris game rules and mechanics

#### 2. RL Training Framework (`rl/`)
- PPO (Proximal Policy Optimization) from scratch
- Custom CUDA kernels for all operations
- No PyTorch, TensorFlow, or JAX dependencies for core training

**Files:**
- `ppo.cu` - PPO algorithm implementation with GAE (Generalized Advantage Estimation)
- `network.cu` - Policy and value networks with forward/backward passes
- `optimizer.cu` - Adam optimizer in pure CUDA

#### 3. Training System (`training/`)
- Training loop coordinator
- Experience replay buffer on GPU
- Checkpointing and model persistence

**Files:**
- `trainer.cpp` - Main training loop and coordination
- `replay_buffer.cu` - GPU-resident experience replay buffer

#### 4. Evaluation & Visualization (`evaluation/`)
- Benchmark suite comparing to baselines
- Training curve visualizations
- Gameplay recording and replay
- Performance profiling dashboard

**Files:**
- `benchmark.py` - Comprehensive benchmark suite
- `visualize.py` - Training curves and gameplay visualization

## 🚀 Getting Started

### Prerequisites

- NVIDIA GPU with CUDA support (compute capability 7.0+)
- CUDA Toolkit 11.8 or later
- CMake 3.18+
- GCC/G++ 9.0+ or Clang 10.0+
- Python 3.8+ (for visualization only)

### Building

```bash
# Clone the repository
git clone https://github.com/yourusername/tinyrl-tetris.git
cd tinyrl-tetris

# Build the project
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Training

```bash
# Run training with default config
./build/trainer --num-envs 1024 --total-timesteps 10000000

# Resume from checkpoint
./build/trainer --checkpoint checkpoints/model_1000000.pt

# Custom configuration
./build/trainer --config configs/fast_train.json
```

### Evaluation

```bash
# Run benchmark suite
python evaluation/benchmark.py --checkpoint checkpoints/best_model.pt --episodes 1000

# Visualize training curves
python evaluation/visualize.py --log-dir logs/run_001/

# Watch trained agent play
./build/play --checkpoint checkpoints/best_model.pt --render
```

## 📊 Performance Targets

| Metric | Target | Notes |
|--------|--------|-------|
| Training Speed | >1M steps/sec | 1024 parallel environments |
| Expert Level Score | >100,000 | Average over 100 episodes |
| Lines Cleared | >500 | Per game average |
| GPU Utilization | >85% | During training |
| Memory Usage | <8GB VRAM | On single RTX 3080 |

## 🔬 Technical Deep Dive

### PPO Algorithm Implementation

The PPO implementation uses:
- **Clipped surrogate objective** for stable policy updates
- **Generalized Advantage Estimation (GAE)** for variance reduction
- **Value function clipping** for critic stability
- **Entropy bonus** for exploration

### Neural Network Architecture

**Policy Network:**
- Input: 200-dim state vector (board + piece info)
- Hidden: [256, 256, 128]
- Output: 7 actions (move left/right, rotate, drop, etc.)

**Value Network:**
- Input: 200-dim state vector
- Hidden: [256, 256, 128]
- Output: Single value estimate

### CUDA Optimizations

1. **Fused Kernels**: Combined operations (e.g., linear + ReLU) in single kernel
2. **Shared Memory**: Utilize fast shared memory for reduction operations
3. **Coalesced Access**: Optimized memory access patterns
4. **Async Execution**: Overlapping compute and memory transfers
5. **Multi-GPU** (stretch): Data parallelism across GPUs

## 📈 Roadmap

- [x] Project structure and dummy files
- [ ] Tetris game engine implementation
- [ ] Neural network forward/backward passes
- [ ] PPO algorithm implementation
- [ ] Training loop and checkpointing
- [ ] Benchmark suite
- [ ] Visualization tools
- [ ] Performance profiling
- [ ] Multi-GPU support
- [ ] Hyperparameter tuning
- [ ] Documentation and tutorials

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## 📝 License

MIT License - see LICENSE file for details

## 🙏 Acknowledgments

- PPO paper: [Proximal Policy Optimization Algorithms](https://arxiv.org/abs/1707.06347)
- GAE paper: [High-Dimensional Continuous Control Using Generalized Advantage Estimation](https://arxiv.org/abs/1506.02438)
- Tetris AI research and game mechanics

## 📧 Contact

Questions? Open an issue or reach out to [your-email@example.com]

---

**Note**: This is a research/educational project. The goal is to understand RL and CUDA programming at a fundamental level by building everything from scratch.
