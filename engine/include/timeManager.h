#pragma once

#include <chrono>

class TimeManager {
public:
    enum Mode { REALTIME, SIMULATION };

    TimeManager(Mode m);
    double getDeltaTime();
    bool needRendering();

private:
    Mode mode;
    std::chrono::steady_clock::time_point last_time;
};
