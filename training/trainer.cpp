// Training loop coordinator

#include <vector>
#include <string>

struct TrainingConfig {
    int num_envs;
    int num_steps;
    int num_epochs;
    int total_timesteps;
    float learning_rate;
    std::string checkpoint_dir;
    int save_frequency;
    int log_frequency;
};

class Trainer {
public:
    Trainer(TrainingConfig config);
    ~Trainer();

    void train();
    void saveCheckpoint(std::string path);
    void loadCheckpoint(std::string path);

private:
    void collectRollouts();
    void updateNetworks();
    void logMetrics();

    TrainingConfig config;
    int current_step;
};

// Entry point
int main(int argc, char** argv);
