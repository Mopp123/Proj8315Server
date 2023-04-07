#include "Debug.h"
#include <iostream>

#define LS_DEBUG_MESSAGE_TAG__INFO          ""
#define LS_DEBUG_MESSAGE_TAG__WARNING       "(Warning)"
#define LS_DEBUG_MESSAGE_TAG__ERROR         "[ERROR]"
#define LS_DEBUG_MESSAGE_TAG__FATAL_ERROR   "<!FATAL ERROR!>"


void Debug::log(std::string message, MessageType t)
{
    switch (t)
    {
        case INFO:          std::cout << LS_DEBUG_MESSAGE_TAG__INFO << " " << message << std::endl; break;
        case WARNING:       std::cout << LS_DEBUG_MESSAGE_TAG__WARNING << " " << message << std::endl; break;
        case ERROR:         std::cout << LS_DEBUG_MESSAGE_TAG__ERROR << " " << message << std::endl; break;
        case FATAL_ERROR:   std::cout << LS_DEBUG_MESSAGE_TAG__FATAL_ERROR << " " << message << std::endl; break;
        default:
            break;
    }
}

void Debug::print_bytes(const char* data, size_t dataSize, int count, std::string message)
{
    std::string bytesStr = "";
    for (int i = 0; i < count; ++i)
    {
        if ((int)dataSize <= i)
        {
            Debug::log(
                "Attempted to print bytes but byte index went out of bounds. Byte index: " +
                std::to_string(i) + " inputted data size: " + std::to_string(dataSize),
                MessageType::ERROR
            );
            break;
        }
        int val = (int)data[i];
        bytesStr += std::to_string(val);
        if (i < count - 1)
            bytesStr += ",";
    }
    Debug::log(message + ": " + bytesStr);
}
