#pragma once

#include <cstring>
#include <string>
#include <cstdint>
#include <queue>
#include <vector>

#include "../../../Proj8315Common/src/Faction.h"
#include "../../../Proj8315Common/src/Tile.h"
#include "../../../Proj8315Common/src/Object.h"
#include "Debug.h"
#include "Action.h"


namespace world
{
    namespace objects
    {
        std::vector<gamecommon::ObjectInfo> load_obj_info_file(const std::string& filePath);

        // Initializes Object Info Lib from database
        std::vector<gamecommon::ObjectInfo> init_obj_info_db(const std::string& filePath);

        class ObjectInstanceData
        {
        private:
            int _x = 0;
            int _z = 0;
            uint64_t _state = 0;
            GC_ubyte _objType = 0;
            std::queue<int> _actionQueue;
            gamecommon::Faction& _factionRef;

            // Current action's progress
            float _actionProgress = 0.0f;

        public:
            ObjectInstanceData(int x, int z, uint64_t currentState, gamecommon::Faction& factionRef) :
                _x(x), _z(z), _state(currentState), _factionRef(factionRef)
        {
            _objType = gamecommon::get_tile_thingid(currentState);
        }
            ObjectInstanceData(const ObjectInstanceData& other) = delete;

            ~ObjectInstanceData()
            {}

            void setPos(int x, int z, uint64_t tileState)
            {
                _x = x;
                _z = z;
                _state = tileState;
            }
            void setState(uint64_t state)
            {
                _state = state;
                _objType = gamecommon::get_tile_thingid(_state);
            }

            int getX() const { return _x; }
            int getZ() const { return _z; }
            uint64_t getTileState() const { return _state; }
            uint64_t getObjType() const { return _objType; }
            float& accessActionProgress() { return _actionProgress; }

            std::queue<int>& getActionQueue() { return _actionQueue; }
            void addAction(int actionID) { _actionQueue.push(actionID); }

            gamecommon::Faction& getFaction() { return _factionRef; }
        };
    }
}
