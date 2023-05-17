#ifndef TIMER_HPP
#define TIMER_HPP
#include <Arduino.h>

class Timer
{
public:
    Timer() : delta_time(0),
              start_time(0),
              is_started(false){};

    uint16_t GetDeltaTime();

    bool IsStarted() { return is_started; };
    void Restart();
    void Stop() { is_started = false; };

private:
    // time elapsed from last call, in ms
    uint16_t delta_time;
    ulong start_time;
    bool is_started;
};

void Timer::Restart()
{
    is_started = true;
    start_time = millis();
}

uint16_t Timer::GetDeltaTime()
{
    delta_time = millis() - (start_time);
    is_started = false;
    return delta_time;
}

#endif