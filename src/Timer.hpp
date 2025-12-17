#pragma once

#include <chrono>

struct Timer {
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
    std::chrono::duration<double> duration;
    Timer();
    ~Timer();
};