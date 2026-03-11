#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <optional>

#include "CommandParser.h"
#include "../cache/LRUCache.h"
#include "../database/Database.h"
#include "../database/TTLManager.h"
#include "../limiter/RateLimiter.h"
#include "../utils/Logger.h"

class Engine
{
private:

    Database db;
    RateLimiter limiter;
    LRUCache cache;
    TTLManager ttlManager;

    std::size_t cacheHits{0};
    std::size_t cacheMisses{0};
    std::size_t totalCommands{0};

    static std::string joinFrom(const std::vector<std::string>& tokens, std::size_t start)
    {
        std::string out;

        for (std::size_t i = start; i < tokens.size(); ++i)
        {
            if (!out.empty())
            {
                out += " ";
            }

            out += tokens[i];
        }

        return out;
    }

public:

    Engine()
        : limiter(1),
          cache(64),
          ttlManager(db, 1)
    {
        ttlManager.start();
        Logger::log("Engine started");
    }

    ~Engine()
    {
        ttlManager.stop();
        Logger::log("Engine stopped");
    }

    void process(const std::string& input)
    {
        const auto tokens = CommandParser::parse(input);

        if (tokens.empty())
        {
            return;
        }

        ++totalCommands;

        const std::string cmd = tokens[0];

        if (cmd == "SET" && tokens.size() >= 3)
        {
            const std::string& key = tokens[1];
            const std::string value = joinFrom(tokens, 2);

            db.set(key, value);
            cache.put(key, value);

            std::cout << "OK" << std::endl;
        }

        else if (cmd == "GET" && tokens.size() >= 2)
        {
            const std::string& key = tokens[1];

            auto cached = cache.get(key);

            if (cached)
            {
                ++cacheHits;
                std::cout << *cached << " (cache)" << std::endl;
                return;
            }

            ++cacheMisses;

            auto value = db.get(key);

            if (value)
            {
                cache.put(key, *value);
                std::cout << *value << std::endl;
            }
            else
            {
                std::cout << "NULL" << std::endl;
            }
        }

        else if (cmd == "DELETE" && tokens.size() >= 2)
        {
            const std::string& key = tokens[1];

            db.del(key);
            cache.remove(key);

            std::cout << "Deleted" << std::endl;
        }

        else if (cmd == "EXPIRE" && tokens.size() >= 3)
        {
            const std::string& key = tokens[1];

            int seconds = 0;

            try
            {
                seconds = std::stoi(tokens[2]);
            }
            catch (...)
            {
                std::cout << "Invalid TTL" << std::endl;
                return;
            }

            db.expire(key, seconds);

            std::cout << "TTL set" << std::endl;
        }

        else if (cmd == "RATE" && tokens.size() >= 2)
        {
            const std::string& user = tokens[1];

            if (limiter.allow(user))
            {
                std::cout << "Allowed" << std::endl;
            }
            else
            {
                std::cout << "Rate limit exceeded" << std::endl;
            }
        }

        else if (cmd == "STATS")
        {
            const std::size_t totalGets = cacheHits + cacheMisses;

            double hitRate = 0.0;

            if (totalGets > 0)
            {
                hitRate = static_cast<double>(cacheHits) /
                          static_cast<double>(totalGets);
            }

            std::cout
                << "commands=" << totalCommands
                << " cache_hits=" << cacheHits
                << " cache_misses=" << cacheMisses
                << " hit_rate=" << hitRate
                << std::endl;
        }

        else
        {
            std::cout << "Unknown command" << std::endl;
        }
    }
};