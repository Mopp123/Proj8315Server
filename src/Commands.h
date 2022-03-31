#pragma once

#include <cstring>
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>

#include "Responses.h"

#include "Common.h"

/*
 
Every command has
1. requesterID (userID who is attempting to exec this)
2. command name
3. args...


*/

#define CMD_MIN_LEN (USER_ID_LEN + sizeof(int32_t))
#define CMD_MAX_ARG_BUF_LEN 64


class Command
{
private:

	char _userID[USER_ID_LEN];
	int32_t _funcName = 0;
	PK_byte _argBuf[CMD_MAX_ARG_BUF_LEN];
	size_t _argBufSize;
public:
	
	Command(char* raw, size_t size);
	Command(const Command& other);
	
	inline const char* getRequester() const { return _userID; } 
	inline int32_t getName() const { return _funcName; }
	inline const char* getArgs() const { return _argBuf; }
	inline size_t getArgBufSize() const { return _argBufSize; }
};

class CMDHandler
{
private:

	std::unordered_map<int32_t, Response(*)(const Command&)> _funcMapping;

public:
	
	CMDHandler();

	Response processCommand(const Command& cmd);

};
