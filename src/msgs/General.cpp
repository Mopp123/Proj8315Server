
#include "General.h"
#include "Server.h"
#include "game/Game.h"
#include "General.h"

namespace msgs
{
	Message msg_createNewFaction(const Message& msg)
	{
		const size_t msgSize = msg.getSize();
		if(msgSize > MESSAGE_MIN_DATA_SIZE + 1)
		{
			std::string factionName(msg.getData(), msgSize);
			return Game::get()->addFaction(factionName.c_str());
		}
		else
		{
			return NULL_MESSAGE;
		}
	}
        
	
	Message msg_fetchServerMessage(const Message& msg)
	{
		std::string message = "Welcome! Testing testing...";
		return Message(NULL_CLIENT, message.data(), message.size());
	}


	Message msg_serverShutdown(const Message& msg)
	{
		Server::trigger_shutdown();
		std::string message = "Shutdown triggered";
		return Message(NULL_CLIENT, message.data(), message.size());
	}
}
