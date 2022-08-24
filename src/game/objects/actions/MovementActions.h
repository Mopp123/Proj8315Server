#pragma once

#include "game/objects/Action.h"
#include "game/objects/Object.h"

namespace world
{
	namespace objects
	{
		namespace actions
		{
			class Move : public Action
			{
			private:
				// Movement direction
				PK_ubyte _dir;
			public:
				Move(PK_ubyte direction);
				virtual PK_ubyte run(ObjectInstanceData* obj, uint64_t* worldState, int worldWidth);

			private:
				bool validateMove(uint64_t targetTileState);
			};
		}
	}
}
