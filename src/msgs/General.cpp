
#include "General.h"
#include "Server.h"
#include "game/Game.h"
#include "General.h"

namespace msgs
{
	Message msg_createNewFaction(Server& server, Message& msg)
	{
		const size_t msgSize = msg.getSize();
		if(msgSize > MESSAGE_MIN_DATA_SIZE + 1)
		{
			std::string factionName(msg.accessData(), msgSize);
			return Game::get()->addFaction(factionName.c_str(), factionName.size());
		}
		else
		{
			return NULL_MESSAGE;
		}
	}
        
	
	Message msg_getServerMessage(Server& server, Message& msg)
	{
		std::string message = "Welcome! Testing testing...";
		return Message(NULL_CLIENT, message.data(), message.size());
	}


	Message msg_updateObserver(Server& server, Message& msg)
	{
		const size_t msgSize = msg.getSize();
		if(msgSize == sizeof(int32_t) * 4)
		{
			int32_t xPos = 0;
			int32_t zPos = 0;
			int32_t observeRadius = 0;

			PK_byte* msgData = msg.accessData();

			memcpy((void*)&xPos, (const void*)(msgData + sizeof(int32_t)), sizeof(int32_t));
			memcpy((void*)&zPos, (const void*)(msgData + sizeof(int32_t) * 2), sizeof(int32_t));
			memcpy((void*)&observeRadius, (const void*)(msgData + sizeof(int32_t) * 3), sizeof(int32_t));

			server.updateClientData(msg.getClient(), xPos, zPos, observeRadius);
		}
		return NULL_MESSAGE;
	}


	Message msg_serverShutdown(Server& server, Message& msg)
	{
		Server::trigger_shutdown();
		std::string message = "Shutdown triggered";
		return Message(NULL_CLIENT, message.data(), message.size());
	}
}
