#include "timer.h"
#include "log.h"

void Timer::start(int interval, std::function<void()> task)
{
    // is started, do not start again
    if (_expired == false)
        return;

    // start async timer, launch thread and wait in that thread
    _expired = false;
    std::thread([this, interval, task]() {
        while (!_try_to_expire)
        {
            // sleep every interval and do the task again and again until times up
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            task();
        }

        {
            // timer be stopped, update the condition variable expired and wake main thread
            std::lock_guard<std::mutex> locker(_mutex);
            _expired = true;
            _expired_cond.notify_one();
        }
        }).detach();
}

void Timer::startOnce(int delay, std::function<void()> task)
{
    std::thread([delay, task]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        task();
        }).detach();
}

void Timer::stop()
{
    // do not stop again
    if (_expired)
        return;

    if (_try_to_expire)
        return;

    // wait until timer
    _try_to_expire = true; // change this bool value to make timer while loop stop
    {
        std::unique_lock<std::mutex> locker(_mutex);
        _expired_cond.wait(locker, [this] {return _expired == true; });

        // reset the timer
        if (_expired == true)
            _try_to_expire = false;
    }
}


TimeoutTimer::TimeoutTimer(std::string _name, std::function<void()> _func, U32 length)
{
    this->name = _name;
    this->func = _func;
    this->timerLength = length;
}

void TimeoutTimer::start()
{
    this->startTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    this->isValid = true;
}
void TimeoutTimer::stop()
{
    this->isValid = false;
}
void TimeoutTimer::restart()
{
    this->start();
}
void TimeoutTimer::reset(std::function<void()> _func, U32 length)
{
    this->func = _func;
    this->timerLength = length;
}


TimeoutTimerPool::TimeoutTimerPool()
{
    run();
}

void TimeoutTimerPool::run()
{
    auto timerPoolFunc = [this]() {
        while (true)
        {
            TimePoint now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
            for (auto& timer : this->timers)
            {
                if (timer->isValid
                    && (now - timer->startTime).count() > timer->timerLength)
                {
                    timer->isValid = false;
                    std::thread(timer->func).detach();
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    };
    std::thread(timerPoolFunc).detach();
}

void TimeoutTimerPool::addTimer(std::string _name, std::function<void()> _func, U32 length)
{
    for (const auto timer : this->timers)
    {
        if (timer->name == _name)
        {
            return;
        }
    }
    TimeoutTimer* newTimer = new TimeoutTimer(_name, _func, length);
    this->timers.push_back(newTimer);
    newTimer->start();
}

void TimeoutTimerPool::killTimerByName(std::string _name)
{
    for (U32 i = 0; i < this->timers.size(); i++)
    {
        if (this->timers[i]->name == _name)
        {
            TimeoutTimer* pTimer = this->timers[i];
            this->timers.erase(this->timers.begin() + i);
            delete pTimer;
            pTimer = nullptr;
            return;
        }
    }
    return;
}

void TimeoutTimerPool::delTimerByName(std::string _name)
{
    killTimerByName(_name);
}

TimeoutTimer* TimeoutTimerPool::getTimerByName(std::string _name)
{
    for (auto& timer : this->timers)
    {
        if (timer->name == _name)
        {
            return timer;
        }
    }
    return nullptr;
}

bool TimeoutTimerPool::isTimerExist(std::string _name)
{
    for (auto& timer : this->timers)
    {
        if (timer->name == _name)
        {
            return true;
        }
    }
    return false;
}





