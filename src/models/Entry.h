#pragma once

#include <chrono>
#include <string>

struct Entry
{
    std::string value;
    bool hasTTL{false};
    std::chrono::system_clock::time_point expireTime{};

    Entry() = default;
    explicit Entry(const std::string& val) : value(val) {}
};
