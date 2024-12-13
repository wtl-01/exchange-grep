
#pragma once
#ifndef STOPWATCH_HPP
#define STOPWATCH_HPP

#include <chrono>
#include <iostream>


class StopWatch
{
public:
    using Clock = std::chrono::high_resolution_clock;

    StopWatch()
        : m_isRunning{ false }, m_startTimePoint{ Clock::now() }, m_endTimePoint{ m_startTimePoint }, m_duration{ 0 }
    {}

    void Start()
    {
        if (!m_isRunning)
        {
            m_startTimePoint = Clock::now();
            m_isRunning = true;
        }
        // Does nothing if stopwatch is already running
    }

    void Stop()
    {
        if (m_isRunning)
        {
            m_endTimePoint = Clock::now();
            std::chrono::duration<double> elapsed_time = m_endTimePoint - m_startTimePoint;
            m_duration += elapsed_time.count();
            m_isRunning = false;
        }
        // Does nothing if stopwatch is not running
    }

    void Reset()
    {
        m_startTimePoint = Clock::now();
        m_endTimePoint = m_startTimePoint;
        m_duration = 0.0;
    }

    double GetTime() const
    {
        if (m_isRunning)
        {
            std::chrono::duration<double> elapsed_time = Clock::now() - m_startTimePoint;
            return elapsed_time.count() + m_duration;
        }

        return m_duration;
    }

private:
    bool m_isRunning = false;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_startTimePoint;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_endTimePoint;
    double m_duration;
};

#endif //STOPWATCH_HPP
