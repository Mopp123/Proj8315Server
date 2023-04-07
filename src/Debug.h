#pragma once

#include <string>


class Debug
{
public:
    enum MessageType
    {
        INFO = 0x0,
        WARNING,
        ERROR,
        FATAL_ERROR
    };

    static void log(std::string message, MessageType t = INFO);
    static void print_bytes(const char* data, size_t dataSize, int count, std::string message = "");
};
