#include "batched_collector.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>

#include <pybind11/numpy.h>

namespace {

void zero_fill(py::array& arr) {
    py::buffer_info info = arr.request();
    std::memset(info.ptr, 0, info.size * info.itemsize);
}

}  // namespace

BatchedTetrisCollector::BatchedTetrisCollector(size_t num_workers,
                                               uint32_t max_steps,
                                               uint8_t queue_size,
                                               uint32_t seed_base)
    : max_steps_(max_steps),
      queue_size_(queue_size),
      obs_dim_(0),
      policy_callback_(py::none()) {
    if (num_workers == 0) {
        throw std::invalid_argument("num_workers must be > 0");
    }
    envs_.reserve(num_workers);
    buffers_.reserve(num_workers);
    workers_.reserve(num_workers);

    for (size_t i = 0; i < num_workers; ++i) {
        envs_.emplace_back(std::make_unique<TetrisGame>(TimeManager::Mode::SIMULATION,
                                                        queue_size_,
                                                        seed_base + static_cast<uint32_t>(i)));
    }

    obs_dim_ = compute_obs_dim(envs_.front()->obs);

    for (size_t i = 0; i < num_workers; ++i) {
        WorkerBuffers buf;
        buf.observations.resize(static_cast<size_t>(max_steps_) * obs_dim_);
        buf.rewards.resize(max_steps_);
        buf.actions.resize(max_steps_);
        buf.log_probs.resize(max_steps_);
        buf.values.resize(max_steps_);
        buf.dones.resize(max_steps_);
        buffers_.push_back(std::move(buf));
    }

    for (size_t i = 0; i < num_workers; ++i) {
        workers_.emplace_back(&BatchedTetrisCollector::worker_loop, this, i);
    }
}

BatchedTetrisCollector::~BatchedTetrisCollector() {
    close();
}

void BatchedTetrisCollector::close() {
    {
        std::lock_guard<std::mutex> lock(job_mutex_);
        shutting_down_ = true;
    }
    job_cv_.notify_all();
    result_cv_.notify_all();
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    workers_.clear();
}

py::dict BatchedTetrisCollector::request_episodes(size_t num_episodes, py::function policy_fn) {
    if (num_episodes == 0) {
        throw std::invalid_argument("num_episodes must be > 0");
    }

    policy_callback_ = std::move(policy_fn);

    {
        std::lock_guard<std::mutex> lock(job_mutex_);
        for (size_t i = 0; i < num_episodes; ++i) {
            job_queue_.push(EpisodeJob{next_job_id_++, max_steps_});
        }
    }
    job_cv_.notify_all();

    std::vector<EpisodeResult> finished;
    finished.reserve(num_episodes);

    {
        py::gil_scoped_release release;
        while (finished.size() < num_episodes) {
            EpisodeResult result = take_result();
            finished.push_back(std::move(result));
        }
    }

    policy_callback_ = py::none();

    const ssize_t episodes = static_cast<ssize_t>(num_episodes);
    const ssize_t max_steps = static_cast<ssize_t>(max_steps_);
    const ssize_t obs_dim = static_cast<ssize_t>(obs_dim_);

    py::array_t<float> observations({episodes, max_steps, obs_dim});
    py::array_t<int32_t> actions({episodes, max_steps});
    py::array_t<float> log_probs({episodes, max_steps});
    py::array_t<float> values({episodes, max_steps});
    py::array_t<float> rewards({episodes, max_steps});
    py::array_t<uint8_t> dones({episodes, max_steps});
    py::array_t<uint32_t> lengths({episodes});

    zero_fill(observations);
    zero_fill(actions);
    zero_fill(log_probs);
    zero_fill(values);
    zero_fill(rewards);
    zero_fill(dones);
    zero_fill(lengths);

    auto* obs_ptr = observations.mutable_data();
    auto* act_ptr = actions.mutable_data();
    auto* log_ptr = log_probs.mutable_data();
    auto* val_ptr = values.mutable_data();
    auto* rew_ptr = rewards.mutable_data();
    auto* done_ptr = dones.mutable_data();
    auto* len_ptr = lengths.mutable_data();

    const size_t obs_stride = static_cast<size_t>(max_steps_) * obs_dim_;
    const size_t step_stride = static_cast<size_t>(max_steps_);

    for (size_t ep = 0; ep < finished.size(); ++ep) {
        const auto& episode = finished[ep];
        const size_t L = episode.length;
        len_ptr[ep] = static_cast<uint32_t>(L);

        std::copy(episode.observations.begin(),
                  episode.observations.end(),
                  obs_ptr + ep * obs_stride);
        std::copy(episode.actions.begin(),
                  episode.actions.end(),
                  act_ptr + ep * step_stride);
        std::copy(episode.log_probs.begin(),
                  episode.log_probs.end(),
                  log_ptr + ep * step_stride);
        std::copy(episode.values.begin(),
                  episode.values.end(),
                  val_ptr + ep * step_stride);
        std::copy(episode.rewards.begin(),
                  episode.rewards.end(),
                  rew_ptr + ep * step_stride);
        std::copy(episode.dones.begin(),
                  episode.dones.end(),
                  done_ptr + ep * step_stride);
    }

    py::dict result;
    result["observations"] = std::move(observations);
    result["actions"] = std::move(actions);
    result["log_probs"] = std::move(log_probs);
    result["values"] = std::move(values);
    result["rewards"] = std::move(rewards);
    result["dones"] = std::move(dones);
    result["lengths"] = std::move(lengths);
    return result;
}

void BatchedTetrisCollector::worker_loop(size_t worker_idx) {
    auto& env = *envs_[worker_idx];
    auto& buf = buffers_[worker_idx];

    while (true) {
        EpisodeJob job = take_job();
        if (shutting_down_) {
            return;
        }

        env.reset();
        uint32_t step_count = 0;

        while (step_count < job.max_steps) {
            flatten_observation(env.obs, buf.observations.data() + static_cast<size_t>(step_count) * obs_dim_);

            int action = 0;
            double log_prob = 0.0;
            double value = 0.0;
            {
                py::gil_scoped_acquire gil;
                if (policy_callback_.is_none()) {
                    throw std::runtime_error("Policy callback not set before worker execution.");
                }
                py::array_t<float> obs_array({static_cast<ssize_t>(obs_dim_)},
                                             buf.observations.data() + static_cast<size_t>(step_count) * obs_dim_);
                py::object out = policy_callback_(obs_array);
                auto tuple = out.cast<py::tuple>();
                if (tuple.size() != 3) {
                    throw std::runtime_error("policy_fn must return (action, log_prob, value)");
                }
                action = tuple[0].cast<int>();
                log_prob = tuple[1].cast<double>();
                value = tuple[2].cast<double>();
            }

            auto result = env.step(action);
            buf.actions[step_count] = action;
            buf.log_probs[step_count] = static_cast<float>(log_prob);
            buf.values[step_count] = static_cast<float>(value);
            buf.rewards[step_count] = result.reward;
            buf.dones[step_count] = result.terminated ? 1 : 0;

            ++step_count;
            if (result.terminated) {
                break;
            }
        }

        EpisodeResult episode;
        episode.job_id = job.job_id;
        episode.length = step_count;
        episode.observations.assign(buf.observations.begin(),
                                    buf.observations.begin() + static_cast<size_t>(step_count) * obs_dim_);
        episode.actions.assign(buf.actions.begin(), buf.actions.begin() + step_count);
        episode.log_probs.assign(buf.log_probs.begin(), buf.log_probs.begin() + step_count);
        episode.values.assign(buf.values.begin(), buf.values.begin() + step_count);
        episode.rewards.assign(buf.rewards.begin(), buf.rewards.begin() + step_count);
        episode.dones.assign(buf.dones.begin(), buf.dones.begin() + step_count);

        push_result(std::move(episode));
    }
}

size_t BatchedTetrisCollector::flatten_observation(const Observation& obs, float* dest) const {
    size_t count = 0;
    auto write_matrix = [&](const std::vector<std::vector<uint8_t>>& mat) {
        if (mat.empty()) {
            return;
        }
        const size_t rows = mat.size();
        const size_t cols = mat[0].size();
        for (size_t r = 0; r < rows; ++r) {
            for (size_t c = 0; c < cols; ++c) {
                dest[count++] = static_cast<float>(mat[r][c]);
            }
        }
    };

    write_matrix(obs.active_tetromino);
    write_matrix(obs.board);
    write_matrix(obs.holder);
    write_matrix(obs.queue);

    return count;
}

size_t BatchedTetrisCollector::compute_obs_dim(const Observation& obs) {
    auto matrix_size = [](const std::vector<std::vector<uint8_t>>& mat) -> size_t {
        if (mat.empty()) {
            return 0;
        }
        return static_cast<size_t>(mat.size()) * mat[0].size();
    };

    return matrix_size(obs.active_tetromino) +
           matrix_size(obs.board) +
           matrix_size(obs.holder) +
           matrix_size(obs.queue);
}

void BatchedTetrisCollector::enqueue_job(EpisodeJob job) {
    {
        std::lock_guard<std::mutex> lock(job_mutex_);
        job_queue_.push(job);
    }
    job_cv_.notify_one();
}

EpisodeJob BatchedTetrisCollector::take_job() {
    std::unique_lock<std::mutex> lock(job_mutex_);
    job_cv_.wait(lock, [&] { return shutting_down_ || !job_queue_.empty(); });
    if (shutting_down_ && job_queue_.empty()) {
        return EpisodeJob{};
    }
    EpisodeJob job = job_queue_.front();
    job_queue_.pop();
    return job;
}

void BatchedTetrisCollector::push_result(EpisodeResult&& result) {
    {
        std::lock_guard<std::mutex> lock(result_mutex_);
        result_queue_.push(std::move(result));
    }
    result_cv_.notify_one();
}

EpisodeResult BatchedTetrisCollector::take_result() {
    std::unique_lock<std::mutex> lock(result_mutex_);
    result_cv_.wait(lock, [&] { return shutting_down_ || !result_queue_.empty(); });
    if (shutting_down_ && result_queue_.empty()) {
        return EpisodeResult{};
    }
    EpisodeResult result = std::move(result_queue_.front());
    result_queue_.pop();
    return result;
}
