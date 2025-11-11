#pragma once

#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include <pybind11/pybind11.h>

#include "tetrisGame.h"

namespace py = pybind11;

struct EpisodeJob {
    uint64_t job_id;
    uint32_t max_steps;
};

struct EpisodeResult {
    uint64_t job_id;
    uint32_t length;
    std::vector<float> observations;
    std::vector<float> rewards;
    std::vector<int32_t> actions;
    std::vector<float> log_probs;
    std::vector<float> values;
    std::vector<uint8_t> dones;
};

struct WorkerBuffers {
    std::vector<float> observations;
    std::vector<float> rewards;
    std::vector<int32_t> actions;
    std::vector<float> log_probs;
    std::vector<float> values;
    std::vector<uint8_t> dones;
};

class BatchedTetrisCollector {
public:
    BatchedTetrisCollector(size_t num_workers,
                           uint32_t max_steps,
                           uint8_t queue_size = 3,
                           uint32_t seed_base = 0);
    ~BatchedTetrisCollector();

    py::dict request_episodes(size_t num_episodes, py::function policy_fn);
    void close();

    uint32_t obs_dim() const { return obs_dim_; }
    uint32_t max_steps() const { return max_steps_; }

private:
    void worker_loop(size_t worker_idx);
    size_t flatten_observation(const Observation& obs, float* dest) const;
    static size_t compute_obs_dim(const Observation& obs);
    void enqueue_job(EpisodeJob job);
    EpisodeJob take_job();
    void push_result(EpisodeResult&& result);
    EpisodeResult take_result();

    const uint32_t max_steps_;
    const uint8_t queue_size_;
    uint32_t obs_dim_;

    std::vector<std::thread> workers_;
    std::vector<std::unique_ptr<TetrisGame>> envs_;
    std::vector<WorkerBuffers> buffers_;

    std::queue<EpisodeJob> job_queue_;
    std::queue<EpisodeResult> result_queue_;
    std::mutex job_mutex_;
    std::mutex result_mutex_;
    std::condition_variable job_cv_;
    std::condition_variable result_cv_;

    bool shutting_down_ = false;
    uint64_t next_job_id_ = 0;

    py::object policy_callback_;
};
