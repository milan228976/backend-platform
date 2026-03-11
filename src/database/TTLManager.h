#pragma once

#include <thread>
#include <atomic>
#include <chrono>

#include "Database.h"

class TTLManager
{
private:

    Database& database;
    int intervalSeconds;

    std::thread worker;
    std::atomic<bool> running{false};

    void run()
    {
        while (running)
        {
            std::this_thread::sleep_for(
                std::chrono::seconds(intervalSeconds)
            );

            database.cleanupExpired();
        }
    }

public:

    TTLManager(Database& db, int interval)
        : database(db), intervalSeconds(interval)
    {
    }

    void start()
    {
        running = true;
        worker = std::thread(&TTLManager::run, this);
    }

    void stop()
    {
        running = false;

        if (worker.joinable())
        {
            worker.join();
        }
    }

    ~TTLManager()
    {
        stop();
    }
};