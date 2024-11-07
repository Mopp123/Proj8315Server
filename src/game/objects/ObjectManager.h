#pragma once

#include <chrono>
#include <vector>
#include <unordered_map>
#include "game/StateUpdater.h"
#include "Object.h"


namespace world
{
    namespace objects
    {
        class ObjectManager : public StateUpdater
        {
        private:
            std::vector<ObjectInstanceData*> _allObjects;

            // NOTE: These actions has nothing to do with the "tile state's current action"
            // These are used to make more complex behaviours from those simple 3 bit - instructions
            std::vector<Action*> _actionsMapping;

        public:
            ObjectManager(Game& gameRef);
            ~ObjectManager();

            // Returns true if spawning was successful
            bool spawnObject(int x, int z, int objLibIndex, gamecommon::Faction& factionRef);
            ObjectInstanceData* accessObject(int index);
            inline std::vector<ObjectInstanceData*>& accessObjects() { return _allObjects; }
            inline size_t getObjectCount() const { return _allObjects.size(); }
        protected:
            virtual void updateFunc();
        };
    }
}
