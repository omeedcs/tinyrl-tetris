#include <cstdint>
#include <memory>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include "tetrisGame.h"
#include "batched_collector.h"

namespace py = pybind11;

// helper methods
// helper method to convert vec2d to numpy
template <typename T>
py::array_t<T> vec2d_to_numpy(const std::vector<std::vector<T>>& vec) {
    // first we can convert our vector to an array
    size_t rows = vec.size();
    size_t cols = vec[0].size();
    // flatten to 1d array
    py::array_t<T> arr({rows, cols});  // Create array with shape
    auto buf = arr.template mutable_unchecked<2>();    // Get mutable buffer with 2D access

    // Now copy data
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            buf(i, j) = vec[i][j];
        }
    }

    return arr;
}

// helper method to convert obs to dictionary
py::dict obs_to_dict(const Observation& obs) {
    py::dict d;
    d["board"] = vec2d_to_numpy(obs.board);
    d["active_tetromino"] = vec2d_to_numpy(obs.active_tetromino);
    d["holder"] = vec2d_to_numpy(obs.holder);
    d["queue"] = vec2d_to_numpy(obs.queue);
    return d;
}

PYBIND11_MODULE(tinyrl_tetris, m, py::mod_gil_not_used()) {
    m.doc() = "TinyRL Tetris Python Bindings";

    // expose TetrisGame class
    py::class_<TetrisGame>(m, "TetrisEnv")
        .def(py::init<TimeManager::Mode, uint8_t>(),
            py::arg("mode"), py::arg("queue_size") = 3)
        .def("reset", [](TetrisGame& self) {
            self.reset();
            return obs_to_dict(self.obs);
        })
        .def("step", [](TetrisGame& self, int action) {
            StepResult result = self.step(action);
            return py::make_tuple(
                obs_to_dict(result.obs),
                result.reward,
                result.terminated,
                py::dict()  // empty info dict
            );
        })
        .def_property_readonly("obs", [](TetrisGame& self) {
            return obs_to_dict(self.obs);
        })
        .def_readonly("score", &TetrisGame::score)
        .def_readonly("game_over", &TetrisGame::game_over);

    // Expose enums
    py::enum_<Action>(m, "Action")
        .value("LEFT", Action::LEFT)
        .value("RIGHT", Action::RIGHT)
        .value("DOWN", Action::DOWN)
        .value("CW", Action::CW)
        .value("CCW", Action::CCW)
        .value("DROP", Action::DROP)
        .value("SWAP", Action::SWAP)
        .value("NOOP", Action::NOOP)
        .export_values();

    py::enum_<TimeManager::Mode>(m, "TimeMode")
        .value("REALTIME", TimeManager::Mode::REALTIME)
        .value("STEPPED", TimeManager::Mode::SIMULATION)
        .export_values();

    py::class_<BatchedTetrisCollector>(m, "BatchedTetrisCollector")
        .def(py::init<size_t, uint32_t, uint8_t, uint32_t>(),
             py::arg("num_workers"),
             py::arg("max_steps"),
             py::arg("queue_size") = 3,
             py::arg("seed_base") = 0)
        .def("request_episodes", &BatchedTetrisCollector::request_episodes,
             py::arg("num_episodes"),
             py::arg("policy_fn"))
        .def("close", &BatchedTetrisCollector::close)
        .def_property_readonly("obs_dim", &BatchedTetrisCollector::obs_dim)
        .def_property_readonly("max_steps", &BatchedTetrisCollector::max_steps);

}
