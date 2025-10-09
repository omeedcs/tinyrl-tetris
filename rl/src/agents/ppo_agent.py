# src/agents/ppo_agent.py
import torch
import torch.nn as nn
import torch.optim as optim
from torch.distributions import Categorical
import gymnasium as gym
import numpy as np
# import tetris_gymnasium.envs  # Register the Tetris environment

from src.models.actor_critic import ActorCritic
from src.configs.hyperparameters import (
    GAMMA, LAMBDA, CLIP_EPS, VALUE_CLIP_EPS, ENTROPY_COEF,
    VALUE_LOSS_COEF, EPOCHS, BATCH_SIZE, MAX_STEPS,
    LEARNING_RATE, HIDDEN_SIZE, NUM_UPDATES, ENV_NAME
)
from src.wrappers import FlattenObservation
import src.env_wrapper # registers the env


def collect_rollouts(env, model, max_steps):
    states, actions, log_probs, rewards, dones, values = [], [], [], [], [], []
    state, _ = env.reset()
    done = False
    # when len(states) < max_steps, we haven't collected enough steps
    while len(states) < max_steps:
        #Returns a new tensor with a dimension of size one inserted at the specified position -> unsqueeze.
        state_tensor = torch.FloatTensor(state).unsqueeze(0) # we have to unsqueeze to make it a batch of size 1
        with torch.no_grad():  # Don't track gradients during rollout collection
            action, log_prob, _, value = model.get_action_and_value(state_tensor)
        action_item = action.item() # item is used in pytorch to extract a single value from a tensor.

        states.append(state)
        actions.append(action_item)
        log_probs.append(log_prob)
        values.append(value)

        # we detatch so it is not included in the computational graph, could lead to gradient explosion?
        next_state, reward, terminated, truncated, info = env.step(action_item)
        done = terminated or truncated
        rewards.append(reward)
        dones.append(done)
        state = next_state
        if done:
            state, _ = env.reset()
            done = False

    # Compute next_value for the last state
    if not dones[-1]:  # If the last episode didn't terminate
        next_state_tensor = torch.FloatTensor(state).unsqueeze(0)
        with torch.no_grad():
            _, _, _, next_value = model.get_action_and_value(next_state_tensor)
    else:
        next_value = torch.zeros(1)

    return states, actions, log_probs, rewards, dones, values, next_value


def compute_advantages_and_returns(rewards, dones, values, next_value, gamma, lambda_):
    """
    Computes GAE advantages and returns (paper Section 5, citing [Sch+15a])

    Initialize lists, gae=0, next_val from last state.

    Reverse loop for efficient GAE calculation.
    for r, d, v in zip(reversed(rewards), reversed(dones), reversed(values)):
        If done, reset gae to TD error delta = r - V(s).
        Else, delta = r + γ V(s_{t+1}) - V(s_t), gae = delta + γ λ gae.
        Append gae (A_t), return = gae + V(s_t).
        Update next_val.

    Reverse lists to original order.
    Return as tensors.
    """
    advantages = []
    returns = []
    gae = 0
    next_val = next_value.item()

    for r, d, v in zip(reversed(rewards), reversed(dones), reversed(values)):
        if d:
            delta = r - v.item()
            gae = delta
        else:
            delta = r + gamma * next_val - v.item()
            gae = delta + gamma * lambda_ * gae
        advantages.append(gae)
        returns.append(gae + v.item())
        next_val = v.item() if d else next_val

    advantages = advantages[::-1]
    returns = returns[::-1]

    return torch.tensor(advantages), torch.tensor(returns)

def ppo_update(model, optimizer, states, actions, old_log_probs, advantages, returns, clip_eps, value_clip_eps, entropy_coef, value_loss_coef, epochs, batch_size):
    """
    Performs PPO updates (paper Section 5, multiple epochs of minibatch SGD on clipped objective).

    Converts data to tensors (states to FloatTensor, etc.).
    Detaches advantages/returns (no grad flow).

    For _ in range(epochs): K epochs (paper Algorithm 1).
    indices = torch.randperm(dataset_size): Shuffle for minibatches.
    for start in range(0, dataset_size, batch_size): Minibatch loop.
    Slice batch data.
    _, new_log_probs, entropy, new_values = model.get_action_and_value(batch_states, batch_actions): Get new probs/values (with given actions for log_prob).
    ratios = torch.exp(new_log_probs - batch_old_log_probs): r_t (paper Section 3).
    surr1 = ratios * batch_advantages: Unclipped surrogate.
    surr2 = torch.clamp(ratios, 1 - clip_eps, 1 + clip_eps) * batch_advantages: Clipped.
    policy_loss = -torch.min(surr1, surr2).mean(): Negative min for maximization (L^CLIP, Equation 7).
    Value clipping (optional, common extension for stability).
    value_loss = 0.5 * torch.max(value_loss1, value_loss2).mean(): MSE with clipping.
    entropy_loss = -entropy.mean(): Negative entropy to encourage exploration.
    loss = policy_loss + value_loss_coef * value_loss + entropy_coef * entropy_loss: Total loss (paper Section 5).
    Optimize: zero_grad, backward, step.
    """
    states = torch.FloatTensor(np.array(states))
    actions = torch.LongTensor(actions)
    old_log_probs = torch.cat(old_log_probs)
    advantages = advantages.detach()
    returns = returns.detach()

    dataset_size = states.size(0)
    for _ in range(epochs):
        indices = torch.randperm(dataset_size)
        for start in range(0, dataset_size, batch_size):
            end = start + batch_size
            batch_idx = indices[start:end]

            batch_states = states[batch_idx]
            batch_actions = actions[batch_idx]
            batch_old_log_probs = old_log_probs[batch_idx]
            batch_advantages = advantages[batch_idx]
            batch_returns = returns[batch_idx]

            _, new_log_probs, entropy, new_values = model.get_action_and_value(batch_states, batch_actions)

            ratios = torch.exp(new_log_probs - batch_old_log_probs)

            surr1 = ratios * batch_advantages
            surr2 = torch.clamp(ratios, 1 - clip_eps, 1 + clip_eps) * batch_advantages
            policy_loss = -torch.min(surr1, surr2).mean()

            value_clipped = batch_returns + torch.clamp(new_values - batch_returns, -value_clip_eps, value_clip_eps)
            value_loss1 = (new_values - batch_returns).pow(2)
            value_loss2 = (value_clipped - batch_returns).pow(2)
            value_loss = 0.5 * torch.max(value_loss1, value_loss2).mean()

            entropy_loss = -entropy.mean()

            loss = policy_loss + value_loss_coef * value_loss + entropy_coef * entropy_loss

            optimizer.zero_grad()
            loss.backward()
            optimizer.step()

def train_ppo():
    """
    Main training loop (paper Algorithm 1).
    Create env, get dims.
    Instantiate model and optimizer.
    for update in range(NUM_UPDATES): Loop over updates.
    Collect rollouts.
    Compute advantages/returns.
    Perform PPO update.
    Every 10 updates, evaluate on 10 episodes and print avg reward (for monitoring).
    """
    print("Creating environment...")
    env = gym.make(ENV_NAME)
    env = FlattenObservation(env)  # Flatten dict observations
    state_dim = env.observation_space.shape[0]
    action_dim = env.action_space.n
    print(f"State dim: {state_dim}, Action dim: {action_dim}")
    model = ActorCritic(state_dim, action_dim, HIDDEN_SIZE)
    optimizer = optim.Adam(model.parameters(), lr=LEARNING_RATE)
    print("Starting training...")

    for update in range(NUM_UPDATES):
        states_list, actions_list, log_probs_list, rewards_list, dones_list, values_list, next_value = collect_rollouts(env, model, MAX_STEPS)

        advantages, returns = compute_advantages_and_returns(rewards_list, dones_list, values_list, next_value, GAMMA, LAMBDA)

        ppo_update(model, optimizer, states_list, actions_list, log_probs_list, advantages, returns, CLIP_EPS, VALUE_CLIP_EPS, ENTROPY_COEF, VALUE_LOSS_COEF, EPOCHS, BATCH_SIZE)

        if update % 10 == 0:
            eval_rewards = []
            for _ in range(10):
                state, _ = env.reset()
                done = False
                total_reward = 0
                while not done:
                    state_tensor = torch.FloatTensor(state).unsqueeze(0)
                    action, _, _, _ = model.get_action_and_value(state_tensor)
                    state, reward, done, _, _ = env.step(action.item())
                    total_reward += reward
                eval_rewards.append(total_reward)
            print(f"Update {update}: Avg reward = {np.mean(eval_rewards)}")
