#pragma once

#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>

#include "../utils/Logger.h"

class RateLimiter
{
private:
    std::unordered_map<std::string, std::chrono::system_clock::time_point> users;
    std::mutex limiterMutex;
    int intervalSeconds;

    void cleanup(const std::chrono::system_clock::time_point& now)
    {
        for (auto it = users.begin(); it != users.end(); )
        {
            const auto age = std::chrono::duration_cast<std::chrono::seconds>(now - it->second).count();
            if (age > intervalSeconds * 4)
            {
                it = users.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

public:
    explicit RateLimiter(int interval = 1) : intervalSeconds(interval) {}

    bool allow(const std::string& user)
    {
        std::lock_guard<std::mutex> lock(limiterMutex);
        const auto now = std::chrono::system_clock::now();

        cleanup(now);

        auto it = users.find(user);
        if (it == users.end())
        {
            users[user] = now;
            Logger::log("RATE allow: " + user);
            return true;
        }

        const auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - it->second).count();
        if (diff >= intervalSeconds)
        {
            it->second = now;
            Logger::log("RATE allow: " + user);
            return true;
        }

        Logger::log("RATE block: " + user);
        return false;
    }
};
