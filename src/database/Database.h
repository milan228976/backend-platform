#pragma once

#include <chrono>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

#include "../models/Entry.h"
#include "../utils/Logger.h"

class Database
{
private:
    std::unordered_map<std::string, Entry> storage;
    mutable std::mutex dbMutex;

    bool isExpired(const Entry& entry) const
    {
        if (!entry.hasTTL)
        {
            return false;
        }
        const auto now = std::chrono::system_clock::now();
        return now >= entry.expireTime;
    }

public:
    Database() = default;
    ~Database() = default;

    void set(const std::string& key, const std::string& value)
    {
        std::lock_guard<std::mutex> lock(dbMutex);
        storage[key] = Entry(value);
        Logger::log("SET " + key + " = " + value);
    }

    std::optional<std::string> get(const std::string& key)
    {
        std::lock_guard<std::mutex> lock(dbMutex);
        auto it = storage.find(key);
        if (it == storage.end())
        {
            return std::nullopt;
        }

        if (isExpired(it->second))
        {
            Logger::log("Expired key evicted: " + key);
            storage.erase(it);
            return std::nullopt;
        }

        Logger::log("GET " + key);
        return it->second.value;
    }

    void del(const std::string& key)
    {
        std::lock_guard<std::mutex> lock(dbMutex);
        const auto erased = storage.erase(key);
        if (erased > 0)
        {
            Logger::log("DELETE " + key);
        }
    }

    void expire(const std::string& key, int seconds)
    {
        std::lock_guard<std::mutex> lock(dbMutex);
        auto it = storage.find(key);
        if (it == storage.end())
        {
            return;
        }
        if (seconds <= 0)
        {
            Logger::log("EXPIRE immediate delete: " + key);
            storage.erase(it);
            return;
        }
        it->second.hasTTL = true;
        it->second.expireTime = std::chrono::system_clock::now() + std::chrono::seconds(seconds);
        Logger::log("EXPIRE " + key + " in " + std::to_string(seconds) + "s");
    }

    void cleanupExpired()
    {
        std::lock_guard<std::mutex> lock(dbMutex);
        for (auto it = storage.begin(); it != storage.end(); )
        {
            if (isExpired(it->second))
            {
                Logger::log("Cleanup removed expired key: " + it->first);
                it = storage.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
};
