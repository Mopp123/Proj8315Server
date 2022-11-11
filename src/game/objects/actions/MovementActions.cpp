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
			// *"radius" can be specified to require certain area to be empty
			bool validate_move(uint64_t* worldState, int worldWidth, int x, int z, int radius = 1)
			{
				// Atm allow all movement if no object already in tile
				if (radius > 1)
				{
					for (int j = z - radius; j < z + radius; ++j)
					{
						for (int i = z - radius; i < z + radius; ++i)
						{
							const int tileIndex = i + j * worldWidth;
							if (tileIndex >= 0 && tileIndex < (worldWidth * worldWidth))
							{
								if (get_tile_thingid(worldState[tileIndex]) != 0)
									return false;
							}
						}
					}
					return true;
				}
				const int tileIndex = x + z * worldWidth;
				if (tileIndex >= 0 && tileIndex < (worldWidth * worldWidth))
				{
					if (get_tile_thingid(worldState[tileIndex]) != 0)
						return false;
					else
						return true;
				}
				else
				{
					return false;
				}
			}

			static float get_move_speed_val(PK_ubyte objType)
			{
				Game* game = Game::get();
				const int speedStat = game->getObjInfo(objType).speed;
				return Game::get()->getStatFloatMapping()["speed"][speedStat];
			}

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
				if (!validate_move(worldState, worldWidth, x, z))
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
					const PK_ubyte objType = get_tile_thingid(startTile);
					const float speedVal = get_move_speed_val(objType);

					set_tile_action(startTile, TileStateAction::TILE_STATE_actionMove);
					set_tile_facingdir(startTile, _dir);
					obj->setState(startTile);
					actionProgress += speedVal * Game::get()->getDeltaTime();
					return ACTION_STATUS_PENDING;
				}
			}



			MoveVertical::MoveVertical(PK_ubyte direction) : 
				_dir(direction)
			{}

			PK_ubyte MoveVertical::run(ObjectInstanceData* obj, uint64_t* worldState, int worldWidth)
			{
				const int radius = 2;
				const int str = 1;

				float& actionProgress = obj->accessActionProgress();
				uint64_t& currentTile = worldState[obj->getX() + obj->getZ() * worldWidth];
				const PK_ubyte objType = get_tile_thingid(currentTile);
				if (actionProgress >= 1.0f)
				{
					actionProgress = 0.0f;
					
					// JUST TEMP HERE!!
					// test making some effect on ground when descenting
					for (int y = obj->getZ() - radius; y <= obj->getZ() + radius; ++y)
					{
						for (int x = obj->getX() - radius; x <= obj->getX() + radius; ++x)
						{
							int tileIndex = x + y * worldWidth;
							
							if (tileIndex >= 0 && tileIndex < (worldWidth * worldWidth ) - 1)
							{
								// attempt to make area a bit less square..
								if (
										(x == obj->getX() && y == obj->getZ()) || 
										(x == obj->getX() + radius && y == obj->getZ() + radius) || 
										(x == obj->getX() + radius && y == obj->getZ()) || 
										(x == obj->getX() && y == obj->getZ() + radius)
									)
									continue;

								uint64_t& observedTile = worldState[tileIndex];
								PK_ubyte currentElevation = get_tile_terrelevation(observedTile);
								if (currentElevation - str >= 0)
									set_tile_terrelevation(observedTile, currentElevation - str);

								// make ground dirty..
								set_tile_terrtype(observedTile, 0);
							}
						}
					}
					// make tile idle
					set_tile_action(currentTile, 0);
					set_tile_thingid(currentTile, objType);
					obj->setState(currentTile);
					return ACTION_STATUS_SUCCESS;
				}
				else
				{
					const float speedVal = get_move_speed_val(objType);
					
					set_tile_action(currentTile, TileStateAction::TILE_STATE_actionMoveVertical);
					set_tile_facingdir(currentTile, _dir);
					obj->setState(currentTile);
					actionProgress += speedVal * Game::get()->getDeltaTime();
					return ACTION_STATUS_PENDING;
				}
			}
		}
	}
}
