## wrapper for getting our custom env to work with gymnasium

from src import setup_env  # Add engine to path
import tinyrl_tetris
from typing import Optional
import numpy as np
import gymnasium as gym

class TetrisEnv(gym.Env):
    """
    Gymnasium wrapper for TinyRL Tetris C++ engine.

    Observation space is a Dict containing:
        - board: (24, 18) - Board state (0=empty, 1-7=piece types)
        - active_piece: (24, 18) - Current falling piece (binary mask)
        - queue: (queue_size*4, 4) - Next pieces stacked vertically
        - holder: (4, 4) - Held piece
    """
    def __init__(self, queue_size=3):
        super().__init__()
        self.queue_size = queue_size
        self.env = tinyrl_tetris.TetrisEnv(tinyrl_tetris.REALTIME, queue_size=queue_size)

        # Action space: 7 discrete actions (LEFT, RIGHT, DOWN, CW, CCW, DROP, SWAP)
        self.action_space = gym.spaces.Discrete(7)

        # Observation space: Dict of numpy arrays (keys must match C++ engine output)
        self.observation_space = gym.spaces.Dict({
            'board': gym.spaces.Box(low=0, high=7, shape=(24, 18), dtype=np.uint8),
            'active_tetromino': gym.spaces.Box(low=0, high=1, shape=(24, 18), dtype=np.uint8),
            'queue': gym.spaces.Box(low=0, high=7, shape=(queue_size * 4, 4), dtype=np.uint8),
            'holder': gym.spaces.Box(low=0, high=7, shape=(4, 4), dtype=np.uint8),
        })

    def _get_obs(self):
        return self.env.obs

    def _get_info(self):
        return {"score": self.env.score, "game_over": self.env.game_over}

    def reset(self, seed=None, options=None):
        super().reset(seed=seed)
        self.env.reset()
        return self._get_obs(), self._get_info()

    def step(self, action):
        obs, reward, done, _  = self.env.step(action)
        truncated = False
        return obs, reward, done, truncated, self._get_info()

# register the env
gym.register(
    id="gymnasium_env/TetrisEnv-v0",
    entry_point=TetrisEnv,
    max_episode_steps=300,  # Prevent infinite episodes
)

# quick testing
if __name__ == "__main__":
    env = TetrisEnv()
    obs, info = env.reset()

    obs, reward, done, info  = env.step(0)
    print(reward)
    print(info)
    print()
