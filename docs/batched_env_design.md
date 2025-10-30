# Batched Tetris Environment Design

## Motivation
- Speed up policy training by running multiple TinyRL Tetris simulations in parallel.
- Maintain compatibility with existing Python RL stack (`TetrisEnv` + `FlattenObservation`).
- Avoid straggler bottlenecks while keeping per-thread environment affinity for RNG and cache locality.

## Terminology
- `N`: number of worker threads (one `TetrisEnv` instance per worker).
- `B`: number of episodes requested by the Python API for the next rollout chunk.
- `K`: maximum episode horizon (steps per episode). Shorter episodes are padded/masked.
- `obs_dim`: flattened observation length produced by `FlattenObservation`.

## High-Level Architecture
1. **Coordinator (Python side)**
   - Receives requests for `B` episodes from trainer.
   - Fills a work queue with `B` episode tokens.
   - Consumes completed episode buffers from workers until `B` are collected, then hands them to learner.
2. **Worker Threads (C++ or Python-managed)**
   - Each owns one `TetrisEnv`, RNG, and double-buffered episode storage.
   - Pull an episode token from the work queue, roll out the episode step by step, write results into its active buffer, and push a completion record to the result queue.
   - Immediately fetch the next token; no need to wait for other workers.
3. **Result Aggregation**
   - Completed episodes flow into a thread-safe queue as views/pointers to the worker’s buffer slots.
   - Aggregator stacks the first `B` episodes into tensors shaped `[B, K, …]` (reusing worker buffers when possible) and returns them to the training loop.

## Data Structures
- **Work Queue**
  - Lock-free or mutex-backed queue containing episode job descriptors (`EpisodeJob { job_id, max_steps=K }`).
  - Coordinator enqueues `B` jobs per request.
- **Worker Buffer**
  - Preallocated tensors per worker:
    - `observations[K, obs_dim]`
    - `rewards[K]`
    - `actions[K]`
    - `dones[K]`
    - `length` (actual steps taken)
  - Two instances (double buffering) to allow producers and consumers to operate without blocking.
- **Result Queue**
  - Holds `EpisodeResult { job_id, worker_id, buffer_ptr }` until coordinator pulls `B` results.

## Worker Loop
1. Pop `EpisodeJob` from work queue (blocks if empty).
2. Reset local env; zero cursor variables.
3. For `step` in `0 .. K-1`:
   - Observe current state, flatten to `obs_dim`, write to buffer.
   - Select action from policy callback or pre-supplied action provider.
   - Step env; record reward, action, done flag.
   - If `done`, break loop.
4. Pad remaining slots with zeros and `done=True` (or leave untouched if consumer uses `length`).
5. Write `length = step_count`.
6. Push `EpisodeResult` to result queue.
7. Swap active buffer (double buffering) and continue with next job.

## Python API Specification
```python
class BatchedTetrisCollector:
    def __init__(self, num_workers: int, max_steps: int, obs_flatten: Callable, policy_fn: Callable):
        ...

    def request_episodes(self, num_episodes: int) -> EpisodeBatch:
        """
        Generate `num_episodes` episodes using the worker pool.

        Returns:
            EpisodeBatch(
                observations: np.ndarray[num_episodes, K, obs_dim],
                rewards: np.ndarray[num_episodes, K],
                actions: np.ndarray[num_episodes, K],
                dones: np.ndarray[num_episodes, K],
                lengths: np.ndarray[num_episodes],
            )
        """

    def close(self):
        """Gracefully shut down workers and release resources."""
```

`policy_fn(worker_id, obs_batch)` can implement synchronous policy evaluation (e.g., single-threaded inference) or leverage shared inference queues. Variant: store actions provided externally (e.g., from PPO mini-batches) instead of querying the policy inside workers.

## Queue Interaction (Coordinator)
1. `request_episodes(B)` enqueues `B` jobs.
2. Collects `EpisodeResult` objects until `B` episodes gathered.
3. Copies or views buffer contents into contiguous tensors (`np.stack` or direct memory if buffer is contiguous).
4. Returns `EpisodeBatch` to learner and recycles worker buffers.

## Concurrency Considerations
- **RNG Isolation:** Each `TetrisEnv` uses a per-worker RNG (`std::mt19937`) to avoid `std::rand` collisions.
- **Memory Reuse:** Double-buffering per worker avoids reallocations. Coordinator signals when buffer can be reused.
- **Backpressure:** If learner falls behind, work queue stops growing (no pending jobs), so workers block on job fetch instead of spinning.
- **Shutdown:** Set a shared atomic flag and enqueue sentinel jobs to terminate workers cleanly.

## Integration Steps
1. Replace global `std::rand` usage in engine with per-instance RNG.
2. Implement worker buffer struct and flattening helper (reuse `FlattenObservation` layout).
3. Build `BatchedTetrisCollector` (Python orchestrator) with ctypes/pybind bindings for worker pool control.
4. Update training loop to request batches instead of stepping a single env.
5. Add stress tests to validate determinism, throughput, and padding semantics.

## Open Questions
- Shared policy inference vs. per-worker inference: decide between batched inference barrier and per-step callback latency.
- Exact padding strategy (zeroed vs. leave untouched with `length` mask).
- Should learner consume fixed-size `[B, K, …]` batches or stream episodes into replay buffer immediately?

## Next Actions
1. Prototype single-threaded collector using this interface for correctness.
2. Extend pybind module to expose worker pool lifecycle management.
3. Bench throughput vs. existing single-env loop and adjust `N`, `K`, `B` heuristics.
4. Document testing harness (unit + integration) and logging hooks.

