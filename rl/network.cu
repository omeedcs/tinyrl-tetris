// Neural Network Implementation in Pure CUDA
// Policy and Value networks

#include <cuda_runtime.h>

// Network architecture
struct NetworkConfig {
    int input_size;
    int hidden_sizes[4];
    int num_hidden_layers;
    int policy_output_size;
    int value_output_size;
};

// Forward pass kernels
__global__ void linearForward(float* input, float* weights, float* bias,
                             float* output, int in_size, int out_size, int batch_size);

__global__ void reluActivation(float* x, int size);

__global__ void softmaxForward(float* logits, float* probs, int batch_size, int num_actions);

// Backward pass kernels
__global__ void linearBackward(float* grad_output, float* grad_input,
                              float* grad_weights, float* grad_bias,
                              float* input, float* weights,
                              int in_size, int out_size, int batch_size);

__global__ void reluBackward(float* grad_output, float* x, float* grad_input, int size);

// Policy network
class PolicyNetwork {
public:
    PolicyNetwork(NetworkConfig config);
    void forward(float* states, float* action_probs, int batch_size);
    void backward(float* grad_loss, int batch_size);

private:
    float* d_weights[5];
    float* d_biases[5];
    NetworkConfig config;
};

// Value network
class ValueNetwork {
public:
    ValueNetwork(NetworkConfig config);
    void forward(float* states, float* values, int batch_size);
    void backward(float* grad_loss, int batch_size);

private:
    float* d_weights[5];
    float* d_biases[5];
    NetworkConfig config;
};
