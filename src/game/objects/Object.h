#pragma once

#include <cstring>
#include <cstdint>
#include <queue>
#include <vector>
#include "Action.h"
#include "Common.h"
#include "game/Faction.h"

#define OBJECT_DATA_MAX_STRLEN 32

namespace world
{
	namespace objects
	{
		// abstract base class of every possible game object type (prev. this was called "thing")
		struct ObjectInfo
		{
			// misc. stuff
			char name[OBJECT_DATA_MAX_STRLEN];
			char description[OBJECT_DATA_MAX_STRLEN];
			
			// stats
			PK_ubyte speed;

			uint64_t initialState = 0;
			
			ObjectInfo(
				const char* objName, 
				const char* objDescription,
				PK_ubyte speedVal,
				uint64_t beginState
			):
				speed(speedVal),
				initialState(beginState)
			{
				memcpy(name, objName, OBJECT_DATA_MAX_STRLEN);
				memcpy(description, objDescription, OBJECT_DATA_MAX_STRLEN);
			}
		
			ObjectInfo(const ObjectInfo& other):
				speed(other.speed),
				initialState(other.initialState)
			{
				memcpy(name, other.name, OBJECT_DATA_MAX_STRLEN);
				memcpy(description, other.description, OBJECT_DATA_MAX_STRLEN);
			}
		
			size_t getSize() const
			{
				return 1 + ((OBJECT_DATA_MAX_STRLEN) * 2);
			}
		};


		class ObjectInstanceData
		{
		private:
			int _x = 0;
			int _z = 0;
			uint64_t _state = 0;
			std::queue<int> _actionQueue;
			Faction* _pFaction;

			// Current action's progress
			float _actionProgress = 0.0f;

		public:
			ObjectInstanceData(int x, int z, uint64_t currentState, Faction* pFaction) :
				_x(x), _z(z), _state(currentState), _pFaction(pFaction)
			{}
			ObjectInstanceData(const ObjectInstanceData& other) = delete;
			
			~ObjectInstanceData()
			{}

			void setPos(int x, int z, uint64_t tileState) 
			{
				_x = x; 
				_z = z; 
				_state = tileState;
			}
			void setState(uint64_t state) { _state = state; }

			int getX() const { return _x; }
			int getZ() const { return _z; }
			uint64_t getTileState() const { return _state; }
			float& accessActionProgress() { return _actionProgress; }
			
			std::queue<int>& getActionQueue() { return _actionQueue; }
			void addAction(int actionID) { _actionQueue.push(actionID); }
			
			Faction* getFaction() { return _pFaction; }
		};
	}
}
