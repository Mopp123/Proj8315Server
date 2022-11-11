#include "ClassActions.h"
#include "game/objects/Object.h"
#include "game/world/Tile.h"
#include "game/objects/ObjectUpdater.h"

namespace world
{
	namespace objects
	{
		namespace actions
		{
			static PK_ubyte run_ship_deploy(
											ObjectUpdater& objUpdater, 
											ObjectInstanceData* obj, 
											uint64_t* worldState, 
											int worldWidth
											)
			{
				Faction* faction = obj->getFaction();
				if (!faction)
				{
					Debug::log("ClassAction0: run_ship_deploy failed! Object's faction was nullptr");
					return ACTION_STATUS_FAILURE;
				}

				const int xPos = obj->getX();
				const int yPos = obj->getZ();

				const int radius = 2;
				int deploymentSlot = 0;
				// TODO: This may attempt to spawn out of map bounds! -> FIX IT!!
				for (int y = yPos - radius; y <= yPos + radius; ++y) //!!!segmentation fault!!! ??
				{
					for (int x = xPos - radius; x <= xPos + radius; ++x)
					{
						const PK_ubyte toDeploy = faction->getDeploymens()[deploymentSlot];
						if (toDeploy != 0)
							objUpdater.spawnObject(x, y, toDeploy, faction);
					}
				}

				return ACTION_STATUS_SUCCESS;
			}


			ClassAction0::ClassAction0(ObjectUpdater& objUpdater):
				_objUpdaterRef(objUpdater)
			{}

			PK_ubyte ClassAction0::run(ObjectInstanceData* obj, uint64_t* worldState, int worldWidth)
			{
				const PK_ubyte objType = obj->getObjType();
				switch(objType)
				{
					case 3:
						return run_ship_deploy(_objUpdaterRef, obj, worldState, worldWidth);
					default:
						Debug::log(
								"Attempted to run ClassAction0 for "
								"object which doesnt have ClassAction0 specified. "
								"ObjectType was: " + std::to_string(objType));
						return ACTION_STATUS_FAILURE;
				}
			}
		}
	}
}
