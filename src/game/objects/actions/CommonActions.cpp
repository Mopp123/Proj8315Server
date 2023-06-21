#include "CommonActions.h"
#include "Debug.h"
#include "game/Game.h"
#include "game/objects/Object.h"
#include "../../../../Proj8315Common/src/Tile.h"


namespace world
{
    namespace objects
    {
        namespace actions
        {
            GC_ubyte IdleAction::run(ObjectInstanceData* obj, uint64_t* worldState, int worldWidth)
            {
                const int tileIndex = obj->getX() + obj->getZ() * worldWidth;
                uint64_t& tileState = worldState[tileIndex];
                gamecommon::set_tile_action(tileState, 0);
                obj->setPos(0,0, tileState);

                return ACTION_STATUS_SUCCESS;
            }
        }
    }
}
