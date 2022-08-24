#pragma once

#include "game/objects//Action.h"

namespace world
{
	namespace objects
	{
		namespace actions
		{
			class IdleAction : public Action
			{
			public:
				virtual PK_ubyte run(ObjectInstanceData* obj, uint64_t* worldState, int worldWidth);
			};
		}
	}
}
