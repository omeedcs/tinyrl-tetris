"""Benchmark script to compare TinyRL C++ engine vs Tetris-Gymnasium Python env."""
import time
import numpy as np
import sys
from pathlib import Path

# Setup paths for both environments
project_root = Path(__file__).parent.parent  # rl dir -> project root
engine_lib = project_root / "engine" / "build" / "lib"
sys.path.insert(0, str(engine_lib))

# Add Tetris-Gymnasium to path
tetris_gym_path = project_root / "Tetris-Gymnasium"
if tetris_gym_path.exists():
    sys.path.insert(0, str(tetris_gym_path))

def benchmark_tinyrl_env():
    """Benchmark the TinyRL C++ Tetris engine."""
    print("\n" + "="*60)
    print("BENCHMARKING: TinyRL C++ Engine")
    print("="*60)

    import tinyrl_tetris

    env = tinyrl_tetris.TetrisEnv(tinyrl_tetris.STEPPED, queue_size=3)

    num_steps = 10000
    num_resets = 0

    start_time = time.time()
    env.reset()

    for i in range(num_steps):
        action = np.random.randint(0, 7)
        obs, reward, done, _ = env.step(action)

        if done:
            env.reset()
            num_resets += 1

    elapsed = time.time() - start_time
    steps_per_sec = num_steps / elapsed

    print(f"Steps: {num_steps:,}")
    print(f"Episodes: {num_resets}")
    print(f"Time: {elapsed:.3f}s")
    print(f"Speed: {steps_per_sec:,.1f} steps/sec")

    return {
        'name': 'TinyRL C++ Engine',
        'steps': num_steps,
        'episodes': num_resets,
        'time': elapsed,
        'steps_per_sec': steps_per_sec
    }

def benchmark_gymnasium_env():
    """Benchmark the Tetris-Gymnasium Python environment."""
    print("\n" + "="*60)
    print("BENCHMARKING: Tetris-Gymnasium Python")
    print("="*60)

    try:
        import gymnasium as gym
        from tetris_gymnasium.envs.tetris import Tetris

        # Try to create the environment
        env = gym.make('tetris_gymnasium/Tetris', render_mode=None)

        num_steps = 10000
        num_resets = 0

        start_time = time.time()
        obs, info = env.reset()

        for i in range(num_steps):
            action = env.action_space.sample()
            obs, reward, terminated, truncated, info = env.step(action)

            if terminated or truncated:
                obs, info = env.reset()
                num_resets += 1

        elapsed = time.time() - start_time
        steps_per_sec = num_steps / elapsed

        print(f"Steps: {num_steps:,}")
        print(f"Episodes: {num_resets}")
        print(f"Time: {elapsed:.3f}s")
        print(f"Speed: {steps_per_sec:,.1f} steps/sec")

        env.close()

        return {
            'name': 'Tetris-Gymnasium Python',
            'steps': num_steps,
            'episodes': num_resets,
            'time': elapsed,
            'steps_per_sec': steps_per_sec
        }

    except Exception as e:
        print(f"Error: Could not benchmark Tetris-Gymnasium: {e}")
        print("Make sure it's installed: pip install tetris-gymnasium")
        return None

def benchmark_wrapped_tinyrl():
    """Benchmark TinyRL with Gymnasium wrapper."""
    print("\n" + "="*60)
    print("BENCHMARKING: TinyRL with Gymnasium Wrapper")
    print("="*60)

    sys.path.insert(0, str(project_root / "rl"))

    import gymnasium as gym
    from src.env_wrapper import TetrisEnv

    env = TetrisEnv(queue_size=3)

    num_steps = 10000
    num_resets = 0

    start_time = time.time()
    obs, info = env.reset()

    for i in range(num_steps):
        action = np.random.randint(0, 7)
        obs, reward, terminated, truncated, info = env.step(action)

        if terminated or truncated:
            obs, info = env.reset()
            num_resets += 1

    elapsed = time.time() - start_time
    steps_per_sec = num_steps / elapsed

    print(f"Steps: {num_steps:,}")
    print(f"Episodes: {num_resets}")
    print(f"Time: {elapsed:.3f}s")
    print(f"Speed: {steps_per_sec:,.1f} steps/sec")

    return {
        'name': 'TinyRL + Gym Wrapper',
        'steps': num_steps,
        'episodes': num_resets,
        'time': elapsed,
        'steps_per_sec': steps_per_sec
    }

def print_comparison(results):
    """Print comparison table."""
    print("\n" + "="*60)
    print("PERFORMANCE COMPARISON")
    print("="*60)
    print()

    # Filter out None results
    valid_results = [r for r in results if r is not None]

    if len(valid_results) == 0:
        print("No valid benchmark results!")
        return

    # Find fastest
    fastest = max(valid_results, key=lambda x: x['steps_per_sec'])

    # Print table
    print(f"{'Environment':<30} {'Steps/sec':>15} {'Speedup':>10}")
    print("-" * 60)

    for result in valid_results:
        speedup = result['steps_per_sec'] / fastest['steps_per_sec']
        speedup_str = f"{speedup:.2f}x" if result != fastest else "baseline"
        print(f"{result['name']:<30} {result['steps_per_sec']:>15,.1f} {speedup_str:>10}")

    print()
    print(f"🏆 Winner: {fastest['name']}")
    print(f"   Speed: {fastest['steps_per_sec']:,.1f} steps/sec")

if __name__ == "__main__":
    print("="*60)
    print("Tetris Environment Performance Benchmark")
    print("="*60)

    results = []

    # Benchmark all environments
    results.append(benchmark_tinyrl_env())
    results.append(benchmark_wrapped_tinyrl())
    results.append(benchmark_gymnasium_env())

    # Print comparison
    print_comparison(results)
