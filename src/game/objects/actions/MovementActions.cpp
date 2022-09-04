
#include "MovementActions.h"
#include "game/Game.h"
#include "game/objects/Action.h"
#include "game/objects/Object.h"
#include "game/world/Tile.h"
#include "Debug.h"

namespace world
{
	namespace objects
	{
		namespace actions
		{
			Move::Move(PK_ubyte direction) : 
				_dir(direction)
			{}

			PK_ubyte Move::run(ObjectInstanceData* obj, uint64_t* worldState, int worldWidth)
			{
				int x = obj->getX();
				int z = obj->getZ();
				const int startIndex = x + z * worldWidth;
				switch (_dir)
				{
					case DIR_N:
						z -= 1;
						break;
					case DIR_NE:
						x += 1;
						z -= 1;
						break;
					case DIR_E:
						x += 1;
						break;
					case DIR_SE:
						x += 1;
						z += 1;
						break;
					case DIR_S:
						z += 1;
						break;
					case DIR_SW:
						x -= 1;
						z += 1;
						break;
					case DIR_W:
						x -= 1;
						break;
					case DIR_NW:
						x -= 1;
						z -= 1;
						break;
				}

				const int destinationIndex = x + z * worldWidth;
				float& actionProgress = obj->accessActionProgress();
				
				// Make sure target is in range
				if (destinationIndex < 0 || destinationIndex >= worldWidth * worldWidth)
				{
					actionProgress = 0.0f;
					return ACTION_STATUS_FAILURE;
				}

				uint64_t& startTile = worldState[startIndex];
				uint64_t& destinationTile = worldState[destinationIndex];
				if (!validateMove(destinationTile))
				{
					actionProgress = 0.0f;
					return ACTION_STATUS_FAILURE;
				}

				// TODO: Self status checking
				
				// Continue action if not ready yet (also check)
				// Perform state change after finishing..
				if (actionProgress >= 1.0f)
				{
					actionProgress = 0.0f;
					// TODO: Better move validation!!!
					transfer_obj_to(startTile, destinationTile);
					obj->setPos(x, z, destinationTile);
					return ACTION_STATUS_SUCCESS;
				}
				else
				{
					// ONLY FOR TESTING ATM!!!
					float speed = 1.0f;
					set_tile_action(startTile, 1);
					set_tile_facingdir(startTile, _dir);
					obj->setState(startTile);
					actionProgress += speed * Game::get()->getDeltaTime();
					return ACTION_STATUS_PENDING;
				}
			}

			bool Move::validateMove(uint64_t targetTileState)
			{
				// Atm allow all movement if no object already in tile
				PK_ubyte thing = get_tile_thingid(targetTileState);
				return thing == 0;
			}
		}
	}
}
