#pragma once

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

class Logger
{
private:
    static std::mutex logMutex;

public:
    // Thread-safe log with timestamp
    static void log(const std::string& message)
    {
        std::lock_guard<std::mutex> lock(logMutex);

        const auto now = std::chrono::system_clock::now();
        const auto timeNow = std::chrono::system_clock::to_time_t(now);
        std::tm tmBuf{};
    #if defined(_WIN32)
        localtime_s(&tmBuf, &timeNow);
    #else
        localtime_r(&timeNow, &tmBuf);
    #endif

        std::ostringstream oss;
        oss << std::put_time(&tmBuf, "%Y-%m-%d %H:%M:%S");

        std::cout << "[" << oss.str() << "] " << message << std::endl;
    }
};

inline std::mutex Logger::logMutex;
