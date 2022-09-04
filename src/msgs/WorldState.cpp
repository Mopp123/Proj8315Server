
#include "WorldState.h"
#include "Server.h"
#include "game/Game.h"


namespace msgs
{
	Message msg_fetchWorldState(Server& server, Message& msg)
	{
		/*
		const size_t msgSize = msg.getSize();
		if(msgSize == MESSAGE_MIN_DATA_SIZE + sizeof(int32_t) * 3)
		{
			const PK_byte* buf = msg.getData() + MESSAGE_MIN_DATA_SIZE;
			int32_t xPos = -1;
			int32_t zPos = -1;
			int32_t observeRadius = -1;

			memcpy(&xPos, buf, sizeof(int32_t));
			memcpy(&zPos, buf + sizeof(int32_t), sizeof(int32_t));
			memcpy(&observeRadius, buf + sizeof(int32_t) * 2, sizeof(int32_t));

			return Game::get()->getWorldState(xPos, zPos, observeRadius);
		}
		else
		{
			return NULL_MESSAGE;
		}
		*/
		return NULL_MESSAGE;
	}
}
