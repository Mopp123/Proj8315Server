
#include "Commands.h"

#include <string>
#include <sstream>
#include <iterator>
#include <algorithm>

#include "game/Game.h"
#include "Debug.h"

#include "cmds/CommandNames.h"
#include "cmds/General.h"
#include "cmds/WorldState.h"

Command::Command(char* raw, size_t size)
{
	memset(_userID, 0, USER_ID_LEN);

	if(size >= CMD_MIN_LEN)
	{
		memcpy(_userID, raw, USER_ID_LEN);
		memcpy(&_funcName, raw + USER_ID_LEN, sizeof(int32_t));

		Debug::log("Attempting to parse arg");

		_argBufSize = size - CMD_MIN_LEN;
		Debug::log("arg buf size was: " + std::to_string(_argBufSize));
		if(_argBufSize > 0)
			memcpy(_argBuf, raw + CMD_MIN_LEN, _argBufSize);
	}
}


Command::Command(const Command& other) :
	_funcName(other._funcName), _argBufSize(other._argBufSize)
{
	memcpy(_userID, other._userID, USER_ID_LEN);
	memcpy(_argBuf, other._argBuf, CMD_MAX_ARG_BUF_LEN);
}



// -------------------------------------------------------

CMDHandler::CMDHandler()
{
	_funcMapping.insert(std::make_pair(CMD_CreateFaction, cmds::cmd_createNewFaction));
	_funcMapping.insert(std::make_pair(CMD_FetchServerMessage, cmds::cmd_fetchServerMessage));
	_funcMapping.insert(std::make_pair(CMD_FetchWorldState, cmds::cmd_fetchWorldState));
}

Response CMDHandler::processCommand(const Command& cmd)
{
	std::string userID(cmd.getRequester(), 32);
	int32_t funcName = cmd.getName();
	Debug::log("Attempting to process cmd (requester: " + userID + " funcName: " + std::to_string(funcName));

	auto iter = _funcMapping.find(cmd.getName());
	if(iter != _funcMapping.end())
	{
		return (*_funcMapping[cmd.getName()])(cmd);
	}
	else
	{
		Debug::log("Failed to process command. Couldn't find cmd name");
		return { nullptr, 0 };
	}
	
}



