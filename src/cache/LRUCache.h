#pragma once

#include <list>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

#include "../utils/Logger.h"

class LRUCache
{
private:
    size_t capacity;
    std::list<std::pair<std::string, std::string>> items; // front = most recent
    std::unordered_map<std::string, std::list<std::pair<std::string, std::string>>::iterator> index;
    mutable std::mutex cacheMutex;

public:
    explicit LRUCache(size_t cap = 128) : capacity(cap) {}

    std::optional<std::string> get(const std::string& key)
    {
        std::lock_guard<std::mutex> lock(cacheMutex);
        auto it = index.find(key);
        if (it == index.end())
        {
            Logger::log("Cache MISS: " + key);
            return std::nullopt;
        }

        items.splice(items.begin(), items, it->second);
        Logger::log("Cache HIT: " + key);
        return it->second->second;
    }

    void put(const std::string& key, const std::string& value)
    {
        std::lock_guard<std::mutex> lock(cacheMutex);

        auto it = index.find(key);
        if (it != index.end())
        {
            it->second->second = value;
            items.splice(items.begin(), items, it->second);
            Logger::log("Cache UPDATE: " + key);
            return;
        }

        if (capacity == 0)
        {
            return;
        }

        if (items.size() >= capacity)
        {
            const auto& last = items.back();
            Logger::log("Cache EVICT: " + last.first);
            index.erase(last.first);
            items.pop_back();
        }

        items.emplace_front(key, value);
        index[key] = items.begin();
        Logger::log("Cache PUT: " + key);
    }

    void remove(const std::string& key)
    {
        std::lock_guard<std::mutex> lock(cacheMutex);
        auto it = index.find(key);
        if (it == index.end())
        {
            return;
        }
        items.erase(it->second);
        index.erase(it);
        Logger::log("Cache REMOVE: " + key);
    }
};
