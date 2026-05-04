#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>

/*
addOrder 花了多少毫秒
modifyOrder 花了多少毫秒
deleteOrder 花了多少毫秒
整个 benchmark 总共花了多少毫秒
*/
class Timer {
private:
    std::chrono::high_resolution_clock::time_point startTime;

public:
    Timer() {
        reset();
    }

    void reset() {
        startTime = std::chrono::high_resolution_clock::now();
    }

    double elapsedMilliseconds() const {
        auto endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = endTime - startTime;
        return elapsed.count();
    }

    double elapsedSeconds() const {
        return elapsedMilliseconds() / 1000.0;
    }
};

#endif