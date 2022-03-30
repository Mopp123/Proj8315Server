
#include "Commands.h"

#include <string>
#include <sstream>
#include <iterator>
#include <algorithm>

#include "game/Game.h"
#include "Debug.h"


Command::Command(char* raw, size_t size)
{
	memset(_userID, 0, 32);

	if(size >= CMD_MIN_LEN)
	{
		memcpy(_userID, raw, 32);
		memcpy(&_funcName, raw + 32, sizeof(int32_t));
		
		int argstrlen = size - CMD_MIN_LEN;
		if(argstrlen > 0)
		{
			std::string str(raw + CMD_MIN_LEN, argstrlen);
        		std::istringstream iss(str);
        		_args = {std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>{}};
		}
	}
}


Command::Command(const Command& other) :
	_funcName(other._funcName), _args(other._args)
{
	memcpy(_userID, other._userID, 32);
}



// -------------------------------------------------------


Response test_func(const char* requester, const std::vector<std::string>& args)
{
	std::string userStr(requester, 32);
	Debug::log("Processing test command requested by: " + userStr + "\nargs:");
	for(const std::string& arg : args)
		Debug::log(arg);

	return { nullptr, 0 };
}


Response func_createFaction(const char* requester, const std::vector<std::string>& args)
{
	std::string userStr(requester, 32);
	if(args.size() == 1)
	{
		return Game::get()->addFaction(userStr, args[0]);
	}

	return { nullptr, 0 };
}


CMDHandler::CMDHandler()
{
	_funcMapping.insert(std::make_pair(420, func_createFaction));
}

Response CMDHandler::processCommand(const Command& cmd)
{

	std::string userID(cmd.getRequester(), 32);
	int32_t funcName = cmd.getName();
	Debug::log("Attempting to process comd (requester: " + userID + " funcName: " + std::to_string(funcName));

	auto iter = _funcMapping.find(cmd.getName());
	if(iter != _funcMapping.end())
	{
		return (*_funcMapping[cmd.getName()])(cmd.getRequester(), cmd.getArgs());
	}
	else
	{
		Debug::log("Failed to process command. Couldn't find cmd name");
		return { nullptr, 0 };
	}
	
}



