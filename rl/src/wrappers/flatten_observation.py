"""Wrapper to flatten dict observations into a single vector."""
import gymnasium as gym
import numpy as np
from gymnasium.spaces import Box


class FlattenObservation(gym.ObservationWrapper):
    """Flattens dict observation spaces into a single flat vector."""

    def __init__(self, env):
        super().__init__(env)
        
        # Flatten the dict observation space
        if isinstance(env.observation_space, gym.spaces.Dict):
            # Calculate total size
            total_size = 0
            for key, space in env.observation_space.spaces.items():
                if isinstance(space, Box):
                    total_size += np.prod(space.shape)
            
            # Create flat box space
            self.observation_space = Box(
                low=-np.inf,
                high=np.inf,
                shape=(total_size,),
                dtype=np.float32
            )
        
    def observation(self, observation):
        """Flatten the observation dict into a vector."""
        if isinstance(observation, dict):
            # Concatenate all observation components
            flat_obs = []
            for key in sorted(observation.keys()):  # Sort for consistency
                obs_component = observation[key]
                flat_obs.append(obs_component.flatten())
            return np.concatenate(flat_obs).astype(np.float32)
        return observation
