#include "Timer.hpp"

#include <iostream>

Timer::Timer()
{
    start = std::chrono::high_resolution_clock::now();
}

Timer::~Timer()
{
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "Timer took " << duration.count() * 1'000'000.0 << " μs " << '\n';
}