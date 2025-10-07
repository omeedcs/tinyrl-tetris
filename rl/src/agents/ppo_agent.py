import torch 
import torch.optim as optim
import gymnasium as gym
import numpy as np

from src.models.actor_critic import ActorCritic
from src.configs.hyperparameters import *

def collect_rollouts(env, model, max_steps):
    pass

def compute_advantages_and_returns(rewards, dones, values, next_value, gamma, lambda_):
    pass


