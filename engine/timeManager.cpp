#include "timeManager.h"
#include "constants.h"

TimeManager::TimeManager(Mode m) : mode(m) {
    if (mode == REALTIME) {
        last_time = std::chrono::steady_clock::now();
    }
}

double TimeManager::getDeltaTime() {
    if (mode == SIMULATION) {
        return Tetris::TICK_RATE;
    } else {
        auto current = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = current - last_time;
        last_time = current;
        return elapsed.count();
    }
}

bool TimeManager::needRendering() {
    return mode == REALTIME;
}
