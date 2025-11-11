"""
Benchmark single-env rollout loop vs. BatchedTetrisCollector.

This is a diagnostic script; it runs entirely in Python so you can compare the
old `env.step` loop against the new collector (which currently executes
sequentially but mirrors the batched interface).
"""

from __future__ import annotations

import argparse
import statistics
import time

import gymnasium as gym
import numpy as np

import src.env_wrapper  # Registers TinyRL env
from src.batched_collector import BatchedTetrisCollector
from src.configs.hyperparameters import ENV_NAME, COLLECTOR_WORKERS
from src.wrappers import FlattenObservation


def old_style_rollouts(env_id: str, num_episodes: int, max_steps: int) -> float:
    env = FlattenObservation(gym.make(env_id))
    start = time.perf_counter()
    for _ in range(num_episodes):
        state, _ = env.reset()
        for _ in range(max_steps):
            action = env.action_space.sample()
            state, _, terminated, truncated, _ = env.step(action)
            if terminated or truncated:
                break
    env.close()
    return time.perf_counter() - start


def batched_rollouts(env_id: str, num_episodes: int, max_steps: int) -> float:
    collector = BatchedTetrisCollector(COLLECTOR_WORKERS, max_steps)
    start = time.perf_counter()
    collector.request_episodes(num_episodes)  # random policy if none provided
    collector.close()
    return time.perf_counter() - start


def run_benchmark(env_id: str, num_episodes: int, max_steps: int, repeats: int):
    old_times = [old_style_rollouts(env_id, num_episodes, max_steps) for _ in range(repeats)]
    new_times = [batched_rollouts(env_id, num_episodes, max_steps) for _ in range(repeats)]

    def summarize(label: str, data):
        avg = statistics.mean(data)
        std = statistics.pstdev(data) if len(data) > 1 else 0.0
        print(f"{label}: avg {avg:.4f}s ± {std:.4f}s | runs={data}")

    print(f"\nBenchmark results ({num_episodes=} episodes, {max_steps=} max steps, repeats={repeats}):")
    summarize("Old env.step loop", old_times)
    summarize("Batched collector", new_times)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Compare rollout collection strategies.")
    parser.add_argument("--episodes", type=int, default=4, help="Episodes per measurement run.")
    parser.add_argument("--max-steps", type=int, default=256, help="Max steps per episode.")
    parser.add_argument("--repeats", type=int, default=5, help="How many times to repeat each measurement.")
    parser.add_argument("--env-id", default=ENV_NAME, help="Gym env ID to benchmark.")
    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()
    run_benchmark(args.env_id, args.episodes, args.max_steps, args.repeats)
