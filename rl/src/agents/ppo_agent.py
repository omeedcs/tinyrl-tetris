import torch 
import torch.optim as optim
import gymnasium as gym
import numpy as np

from src.models.actor_critic import ActorCritic
from src.configs.hyperparameters import *

def collect_rollouts(env, model, max_steps):
    states, actions, log_probs, rewards, dones, values = [], [], [], [], [], []
    state, _ = env.reset()
    done = False
    # when len(states) < max_steps, we haven't collected enough steps
    while len(states) < max_steps and not done:
        #Returns a new tensor with a dimension of size one inserted at the specified position -> unsqueeze. 
        state_tensor = torch.FloatTensor(state).unsqueeze(0) # we have to unsqueeze to make it a batch of size 1
        action, log_prob, _, value = model.get_action_and_value(state_tensor)
        action = action.item() # item is used in pytorch to extract a single value from a tensor.
        log_prob = log_prob.item()
        

        

def compute_advantages_and_returns(rewards, dones, values, next_value, gamma, lambda_):
    pass

def ppo_update(model, optimizer, states, actions, old_log_probs, advantages, returns, clip_eps, value_clip_eps, entropy_coef, value_loss_coef, epochs, batch_size):
    pass

def train(env, model, optimizer, num_updates):
    pass

