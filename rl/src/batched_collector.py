"""
Python façade over the multithreaded C++ BatchedTetrisCollector.
"""

from __future__ import annotations

from collections import namedtuple
from typing import Callable, Optional, Tuple

import gymnasium as gym
import numpy as np

import src.env_wrapper  # Ensures engine library is on sys.path
import tinyrl_tetris


EpisodeBatch = namedtuple(
    "EpisodeBatch",
    ["observations", "actions", "log_probs", "values", "rewards", "dones", "lengths"],
)


class BatchedTetrisCollector:
    """Wrapper that exposes the C++ collector to Python."""

    def __init__(self, num_workers: int, max_steps: int, queue_size: int = 3):
        if num_workers <= 0:
            raise ValueError("num_workers must be positive")
        self.core = tinyrl_tetris.BatchedTetrisCollector(num_workers, max_steps, queue_size)
        self.max_steps = max_steps
        self.obs_dim = self.core.obs_dim
        self.action_space = gym.spaces.Discrete(7)  # Matches engine action count

    def request_episodes(
        self,
        num_episodes: int,
        policy_fn: Optional[Callable[[np.ndarray], Tuple[int, float, float]]] = None,
    ) -> EpisodeBatch:
        if policy_fn is None:
            def policy_fn(_state: np.ndarray):
                action = self.action_space.sample()
                return action, 0.0, 0.0

        data = self.core.request_episodes(num_episodes, policy_fn)
        dones = data["dones"].astype(bool, copy=False)

        return EpisodeBatch(
            observations=data["observations"],
            actions=data["actions"],
            log_probs=data["log_probs"],
            values=data["values"],
            rewards=data["rewards"],
            dones=dones,
            lengths=data["lengths"],
        )

    def close(self):
        self.core.close()
