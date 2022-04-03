
#include "WorldState.h"
#include "game/Game.h"

namespace cmds
{
	
	Response cmd_fetchWorldState(const Command& cmd)
	{
		if(cmd.getArgBufSize() == sizeof(int32_t) * 2)
		{
			const PK_byte* buf = cmd.getArgs();
			int32_t xPos = -1;
			int32_t zPos = -1;

			memcpy(&xPos, buf, sizeof(int32_t));
			memcpy(&zPos, buf + sizeof(int32_t), sizeof(int32_t));

			return Game::get()->getWorldState(xPos, zPos);
		}
		else
		{
			return { nullptr, 0 };
		}
	}
}
