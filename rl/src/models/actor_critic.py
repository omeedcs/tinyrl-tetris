import torch
import torch.nn as nn
import torch.optim as optim
from torch.distributions import Categorical

class ActorCritic(nn.Module):
    def __init__(self, state_dim, action_dim, hidden_size):
        super(ActorCritic, self).__init__()
        self.shared = nn.Sequential(
            nn.Linear(state_dim, hidden_size), 
            nn.Tanh(), 
            nn.Linear(hidden_size, hidden_size),
            nn.Tanh())
        
        # actor head -> outputs logits for action probabilities
        self.actor = nn.Linear(hidden_size, action_dim)
        
        # outputs a single value estimate.
        self.critic = nn.Linear(hidden_size, 1)

    def forward(self, state):
        features = self.shared(state)
        action_logits = self.actor(features)
        value = self.critic(features)
        return action_logits, value

    def get_action_and_value(self, state, action = None):
        action_logits, values = self(state)
        # our action space is discrete...
        dist = Categorical(logits = action_logits) # categorical distribution computes the probability dist over discrte actions.
        if action is None:
            action = dist.sample() # sample an action during rollout.

        log_prob = dist.log_prob(action) # section 3, equation 7.
        entropy = dist.entropy() # how well our prediction matched our true distribution?
        return action, log_prob, entropy, values.squeeze(-1)