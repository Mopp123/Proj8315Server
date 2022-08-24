
#include "CommonActions.h"
#include "game/objects/Object.h"

namespace world
{
	namespace objects
	{
		namespace actions
		{
			PK_ubyte IdleAction::run(ObjectInstanceData* obj, uint64_t* worldState, int worldWidth)
			{
				const int tileIndex = obj->getX() + obj->getZ() * worldWidth;
				uint64_t& tileState = worldState[tileIndex];
				set_tile_action(tileState, 0);
				obj->setPos(0,0, tileState);

				return ACTION_STATUS_SUCCESS;
			}
		}
	}
}
