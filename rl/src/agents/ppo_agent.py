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
    ROLLOUT_EPISODES, COLLECTOR_WORKERS,
    LEARNING_RATE, HIDDEN_SIZE,
    NUM_UPDATES, ENV_NAME
)
from src.wrappers import FlattenObservation
import src.env_wrapper # registers the env
from src.batched_collector import BatchedTetrisCollector

def collect_rollouts(collector, model, num_episodes):
    """Use the multithreaded collector to gather padded episode batches."""
    def policy_fn(state: np.ndarray):
        state_tensor = torch.from_numpy(state).float().unsqueeze(0)
        with torch.no_grad():
            action, log_prob, _, value = model.get_action_and_value(state_tensor)
        return action.item(), float(log_prob.item()), float(value.item())

    batch = collector.request_episodes(num_episodes, policy_fn)

    states, actions, log_probs, rewards, dones, values = [], [], [], [], [], []
    for ep in range(num_episodes):
        length = int(batch.lengths[ep])
        if length <= 0:
            continue

        states.extend(batch.observations[ep, :length])
        actions.extend(batch.actions[ep, :length])
        rewards.extend(batch.rewards[ep, :length])
        dones.extend(batch.dones[ep, :length])

        for step_idx in range(length):
            log_probs.append(torch.tensor([batch.log_probs[ep, step_idx]], dtype=torch.float32))
            values.append(torch.tensor([batch.values[ep, step_idx]], dtype=torch.float32))

    if states and not dones[-1]:
        next_state_tensor = torch.from_numpy(states[-1]).float().unsqueeze(0)
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
    print("Creating environment and collector...")
    collector = BatchedTetrisCollector(COLLECTOR_WORKERS, MAX_STEPS)
    eval_env = FlattenObservation(gym.make(ENV_NAME))
    state_dim = collector.obs_dim
    action_dim = collector.action_space.n
    print(f"State dim: {state_dim}, Action dim: {action_dim}")
    model = ActorCritic(state_dim, action_dim, HIDDEN_SIZE)
    optimizer = optim.Adam(model.parameters(), lr=LEARNING_RATE)
    print("Starting training...")

    for update in range(NUM_UPDATES):
        states_list, actions_list, log_probs_list, rewards_list, dones_list, values_list, next_value = collect_rollouts(
            collector, model, ROLLOUT_EPISODES
        )

        advantages, returns = compute_advantages_and_returns(rewards_list, dones_list, values_list, next_value, GAMMA, LAMBDA)

        ppo_update(model, optimizer, states_list, actions_list, log_probs_list, advantages, returns, CLIP_EPS, VALUE_CLIP_EPS, ENTROPY_COEF, VALUE_LOSS_COEF, EPOCHS, BATCH_SIZE)

        # Show progress more frequently early on
        if update < 10 or update % 10 == 0:
            # Quick eval with fewer episodes
            num_eval = 3 if update < 100 else 10
            eval_rewards = []
            for _ in range(num_eval):
                state, _ = eval_env.reset()
                done = False
                total_reward = 0
                while not done:
                    state_tensor = torch.FloatTensor(state).unsqueeze(0)
                    action, _, _, _ = model.get_action_and_value(state_tensor)
                    state, reward, done, _, _ = eval_env.step(action.item())
                    total_reward += reward
                eval_rewards.append(total_reward)
            print(f"Update {update}/{NUM_UPDATES}: Avg reward = {np.mean(eval_rewards):.2f} (±{np.std(eval_rewards):.2f})")

    collector.close()
