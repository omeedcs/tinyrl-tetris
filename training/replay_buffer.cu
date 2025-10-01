// Experience replay buffer on GPU

#include <cuda_runtime.h>

struct Experience {
    float* states;
    int* actions;
    float* rewards;
    float* next_states;
    bool* dones;
    float* log_probs;
    float* values;
};

class ReplayBuffer {
public:
    ReplayBuffer(int capacity, int state_dim, int num_envs);
    ~ReplayBuffer();

    void add(float* states, int* actions, float* rewards,
             float* next_states, bool* dones, float* log_probs, float* values);

    void sample(int batch_size, Experience& batch);
    void clear();

    int size();
    bool isFull();

private:
    float* d_states;
    int* d_actions;
    float* d_rewards;
    float* d_next_states;
    bool* d_dones;
    float* d_log_probs;
    float* d_values;

    int capacity;
    int current_size;
    int write_idx;
    int state_dim;
};
