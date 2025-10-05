#include <pybind11/pybind11.h>
#include "tetrisGame.h"

namespace py = pybind11;

PYBIND11_MODULE(tinyrl_tetris, m, py::mod_gil_not_used()) {
    m.doc() = "TinyRL Tetris Python Bindings";

    // expose TetrisGame class
    py::class_<TetrisGame>(m, "TetrisEnv")
        .def(py::init<TimeManager::Mode, uint8_t>(),
            py::arg("mode"), py::arg("queue_size") = 3)
        .def("reset", &TetrisGame::reset)
        .def("step", &TetrisGame::step)
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
}
