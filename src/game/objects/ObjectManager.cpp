#include "ObjectManager.h"
#include "game/Game.h"
#include "game/StateUpdater.h"
#include "Debug.h"
#include "../../../Proj8315Common/src/Tile.h"
#include "../../../Proj8315Common/src/Common.h"

#include "actions/MovementActions.h"
#include "actions/CommonActions.h"
#include "actions/ClassActions.h"


using namespace gamecommon;

namespace world
{
    namespace objects
    {
        ObjectManager::ObjectManager(Game& gameRef) :
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
                new actions::Move(TileStateDirection::TILE_STATE_dirNW),

                new actions::MoveVertical(TileStateDirection::TILE_STATE_dirN),
                new actions::MoveVertical(TileStateDirection::TILE_STATE_dirS),

                new actions::ClassAction0(*this)
            };

        }

        ObjectManager::~ObjectManager()
        {
            for (ObjectInstanceData* obj : _allObjects)
                delete obj;

            for (Action* a : _actionsMapping)
                delete a;
        }

        bool ObjectManager::spawnObject(int x, int z, int objLibIndex, gamecommon::Faction& factionRef)
        {
            // *Don't allow spawning "empty" objects accidentally
            if (objLibIndex == 0)
            {
                Debug::log("Attempted to spawn empty object", Debug::MessageType::ERROR);
                return false;
            }
            const int worldWidth = _gameRef._worldWidth;
            const int tileIndex = x + z * worldWidth;
            if (tileIndex >= 0 && tileIndex < (worldWidth * worldWidth))
            {
                uint64_t originalState = _gameRef.getTileState(tileIndex);
                if (!get_tile_thingid(originalState))
                {
                    // Add the "objects initial state" to the tile's original state (so all terrain-detail/effects/etc stuff may remain, if we want)
                    uint64_t newState = originalState | _gameRef._objectInfo[objLibIndex].initialState;
                    ObjectInstanceData* newObj = new ObjectInstanceData(x, z, newState, factionRef);
                    _allObjects.push_back(newObj);
                    _gameRef.setTileState(tileIndex, newState);
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                //Debug::log("Attempted to spawn object outside of map bounds!");
                return false;
            }
        }

        ObjectInstanceData* ObjectManager::accessObject(int index)
        {
            if (index >= 0 && index < (int)_allObjects.size())
            {
                return _allObjects[index];
            }
            else
            {
                Debug::log("Attempted to access object but index was out of bounds");
                return nullptr;
            }
        }

        void ObjectManager::updateFunc()
        {
            for (ObjectInstanceData* obj : _allObjects)
            {
                std::queue<int>& actionQueue = obj->getActionQueue();

                // *Make sure we always have at least idle action going on (currently illegal to ever have empty action queue)
                if (actionQueue.empty())
                    actionQueue.push(TileStateAction::TILE_STATE_actionIdle);

                const int currentActionID = actionQueue.front();
                GC_ubyte actionStatus = _actionsMapping[currentActionID]->run(obj, _gameRef._pWorld, _gameRef._worldWidth);

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
};
