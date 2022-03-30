#pragma once

#include <cstring>
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>

#include "Responses.h"

/*
 
Every command has
1. requesterID (userID who is attempting to exec this)
2. command name
3. args...


*/

#define CMD_MIN_LEN 36

class Command
{
private:

	char _userID[32];
	int32_t _funcName = 0;
	std::vector<std::string> _args;

public:
	
	Command(char* raw, size_t size);
	Command(const Command& other);
	
	inline const char* getRequester() const { return _userID; } 
	inline int32_t getName() const { return _funcName; }
	inline const std::vector<std::string> getArgs() const { return _args; }
};

class CMDHandler
{
private:

	std::unordered_map<int32_t, Response(*)(const std::vector<std::string>&)> _funcMapping;

public:
	
	Response processCommand(const Command& cmd);

};
