#pragma once
#ifndef TIMER_H
#define TIMER_H

#include <functional>
#include <chrono>
#include <thread>
#include <atomic>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <string>
#include "define.h"
#include <vector>

struct Timer
{
public:
    Timer() : _expired(true), _try_to_expire(false)
    {}

    Timer(const Timer& timer)
    {
        _expired = timer._expired.load();
        _try_to_expire = timer._try_to_expire.load();
    }

    ~Timer()
    {
        stop();
    }
    void start(int interval, std::function<void()> task);

    void startOnce(int delay, std::function<void()> task);

    void stop();

private:
    std::atomic<bool> _expired; // timer stopped status
    std::atomic<bool> _try_to_expire; // timer is in stop process
    std::mutex _mutex;
    std::condition_variable _expired_cond;
};

//struct TimerBase
//{
//    std::string name{""};
//    std::function<void()> func;
//    U32 timerLength{0};
//    std::chrono::milliseconds startTime;
//    std::chrono::milliseconds pastTime;
//    bool isValid{true};
//
//};
using TimePoint = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>;
struct TimeoutTimer
{
    TimeoutTimer(std::string _name, std::function<void()> _func, U32 length);

    void start();
    void restart();
    void stop();
    void reset(std::function<void()> _func, U32 length);

    std::string name{ "" };
    std::function<void()> func;
    U32 timerLength{ 0 };
    TimePoint startTime;
    //std::chrono::milliseconds pastTime;
    std::atomic<bool> isValid{ true };
};

struct TimeoutTimerPool
{
    TimeoutTimerPool();
    TimeoutTimerPool(const TimeoutTimerPool&) = delete;
    TimeoutTimerPool& operator=(const TimeoutTimerPool&) = delete;
    static TimeoutTimerPool& getInstance()
    {
        static TimeoutTimerPool instance;
        return instance;
    }

    void run();
    void addTimer(std::string _name, std::function<void()> _func, U32 length);
    void killTimerByName(std::string _name);
    TimeoutTimer* getTimerByName(std::string _name);
    bool isTimerExist(std::string _name);

    void delTimerByName(std::string _name);
    void clearInvalidTimers();
    void clear();

    std::vector<TimeoutTimer*> timers;

};

#endif // TIMER_H
