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
	        class ObjectUpdater : public StateUpdater
	        {
		private:
			std::vector<ObjectInstanceData*> _allObjects;
	
			const std::unordered_map<const char*, std::pair<int ,float>> _statFloatMapping = 
			{
				{"speed", {1, 0.5f}}
			};

			std::vector<Action*> _actionsMapping;

	        public:
			ObjectUpdater(Game& gameRef);
			~ObjectUpdater();
	
			void spawnObject(int x, int z, int objLibIndex, Faction* faction);
			inline ObjectInstanceData* accessObject(int index) { return _allObjects[index]; }
	        protected:
			virtual void updateFunc();
	        };
	}
}
