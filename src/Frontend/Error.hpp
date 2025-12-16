#pragma once

#include <string>

struct Error {
    const std::string msg;
    const size_t index;
    Error(std::string msg, const size_t index)
        : msg(std::move(msg)), index(index) {}
};