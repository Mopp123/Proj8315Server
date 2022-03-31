
#include "General.h"
#include "game/Game.h"

namespace cmds
{
	Response cmd_createNewFaction(const Command& cmd)
	{
		if(cmd.getArgBufSize() > 1)
		{
			std::string factionName(cmd.getArgs(), cmd.getArgBufSize());
			return Game::get()->addFaction(cmd.getRequester(), factionName);
		}
		else
		{
			return { nullptr, 0 };
		}
	}
        
	Response cmd_fetchServerMessage(const Command& cmd)
	{
		std::string message = "Welcome! Testing testing...";
		return { message.data(), message.size() };
	}
}
