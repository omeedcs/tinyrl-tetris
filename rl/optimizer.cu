// Custom CUDA optimizer (Adam)

#include <cuda_runtime.h>

struct OptimizerState {
    float* m;  // First moment estimate
    float* v;  // Second moment estimate
    int t;     // Timestep
};

// Adam optimizer kernel
__global__ void adamStep(float* params, float* grads,
                        float* m, float* v, int t,
                        float lr, float beta1, float beta2, float epsilon,
                        int num_params) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= num_params) return;

    // Update biased first moment estimate
    m[idx] = beta1 * m[idx] + (1.0f - beta1) * grads[idx];

    // Update biased second moment estimate
    v[idx] = beta2 * v[idx] + (1.0f - beta2) * grads[idx] * grads[idx];

    // Compute bias-corrected moment estimates
    float m_hat = m[idx] / (1.0f - powf(beta1, t));
    float v_hat = v[idx] / (1.0f - powf(beta2, t));

    // Update parameters
    params[idx] -= lr * m_hat / (sqrtf(v_hat) + epsilon);
}

class AdamOptimizer {
public:
    AdamOptimizer(int num_params, float lr = 3e-4f);
    void step(float* params, float* grads);
    void reset();

private:
    OptimizerState state;
    float learning_rate;
};
