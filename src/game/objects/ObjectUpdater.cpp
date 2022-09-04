
#include "ObjectUpdater.h"
#include "game/Game.h"
#include "game/StateUpdater.h"
#include "game/world/Tile.h"
#include "Debug.h"
#include "Common.h"

#include "actions/MovementActions.h"
#include "actions/CommonActions.h"

namespace world
{
	namespace objects
	{
		ObjectUpdater::ObjectUpdater(Game& gameRef) :
			StateUpdater(gameRef)
		{
			
			_actionsMapping = { 
				new actions::IdleAction, 
				new actions::Move(TileStateDirection::TILE_STATE_dirN), 
				new actions::Move(TileStateDirection::TILE_STATE_dirNE), 
				new actions::Move(TileStateDirection::TILE_STATE_dirE), 
				new actions::Move(TileStateDirection::TILE_STATE_dirSE), 
				new actions::Move(TileStateDirection::TILE_STATE_dirS), 
				new actions::Move(TileStateDirection::TILE_STATE_dirSW), 
				new actions::Move(TileStateDirection::TILE_STATE_dirW), 
				new actions::Move(TileStateDirection::TILE_STATE_dirNW)
			};
		}
	
		ObjectUpdater::~ObjectUpdater()
		{
			for (ObjectInstanceData* obj : _allObjects)
				delete obj;

			for (Action* a : _actionsMapping)
				delete a;
		}
	
	 	void ObjectUpdater::spawnObject(int x, int z, int objLibIndex, Faction* faction)
		{
			const int tileIndex = x + z * _gameRef._worldWidth;
			uint64_t originalState = _gameRef.getTileState(tileIndex);
			// Add the "objects initial state" to the tile's original state (so all terrain-detail/effects/etc stuff may remain, if we want)
			uint64_t newState = originalState | _gameRef._objectInfo[objLibIndex].initialState;
			ObjectInstanceData* newObj = new ObjectInstanceData(x, z, newState, faction);
			_allObjects.push_back(newObj);
			_gameRef.setTileState(tileIndex, newState);
		}
	
	 	void ObjectUpdater::updateFunc()
		{
			for (ObjectInstanceData* obj : _allObjects)
			{
				std::queue<int>& actionQueue = obj->getActionQueue();
				
				// *Make sure we always have at least idle action going on (currently illegal to ever have empty action queue)
				if (actionQueue.empty())
					actionQueue.push(ACTION_IDLE);

				const int currentActionID = actionQueue.front();
				PK_ubyte actionStatus = _actionsMapping[currentActionID]->run(obj, _gameRef._pWorld, _gameRef._worldWidth);

				switch (actionStatus)
				{
					case ACTION_STATUS_SUCCESS:
						//Debug::log("Action completed successfully");
						actionQueue.pop();
						break;
					case ACTION_STATUS_FAILURE:
						//Debug::log("Action failed");
						actionQueue.pop();
						break;
					case ACTION_STATUS_PENDING:
						break;
					default:
						Debug::log("ERROR STATE!");
						break;
				}
			}
		}
	}
}
