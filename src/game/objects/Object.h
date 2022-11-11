#pragma once

#include <cstring>
#include <string>
#include <cstdint>
#include <queue>
#include <vector>

#include "Debug.h"
#include "Action.h"
#include "Common.h"
#include "game/Faction.h"
#include "game/world/Tile.h"


#define OBJECT_DATA_STRLEN_NAME 32
#define OBJECT_DATA_STRLEN_DESCRIPTION 32
#define OBJECT_DATA_STRLEN_ACTION_NAME 16

#define OBJECT_DATA_COUNT_STATS 1
#define OBJECT_DATA_COUNT_TOTAL ((1 + 1) + (TILE_STATE_MAX_action + 1) + OBJECT_DATA_COUNT_STATS)


namespace world
{
	namespace objects
	{
		// Contains static info of a game object type (prev. this was called "thing")
		struct ObjectInfo
		{
			// misc. stuff
			char name[OBJECT_DATA_STRLEN_NAME];
			char description[OBJECT_DATA_STRLEN_DESCRIPTION];
			
			char actionSlot[TILE_STATE_MAX_action + 1][OBJECT_DATA_STRLEN_ACTION_NAME];

			// stats
			PK_ubyte speed;

			uint64_t initialState = 0;
			
			ObjectInfo(
				const std::string& objName,
				const std::string& objDescription,
				std::vector<std::string>& actionSlots,
				PK_ubyte speedVal,
				uint64_t beginState
			):
				speed(speedVal),
				initialState(beginState)
			{
				memset(name, 0, sizeof(char) * OBJECT_DATA_STRLEN_NAME);
				memset(description, 0, sizeof(char) * OBJECT_DATA_STRLEN_DESCRIPTION);
				
				
				size_t nameLen = objName.size();
				size_t descriptionLen = objDescription.size();

				if (nameLen > OBJECT_DATA_STRLEN_NAME)
					nameLen = OBJECT_DATA_STRLEN_NAME;
				if (descriptionLen > OBJECT_DATA_STRLEN_DESCRIPTION)
					descriptionLen = OBJECT_DATA_STRLEN_DESCRIPTION;

				memcpy(name, objName.c_str(), sizeof(char) * nameLen);
				memcpy(description, objDescription.c_str(), sizeof(char) * descriptionLen);

				for (size_t i = 0; i < TILE_STATE_MAX_action + 1; ++i)
				{
					memset(actionSlot[i], 0, sizeof(char) * OBJECT_DATA_STRLEN_ACTION_NAME);
					if (i < actionSlots.size())
					{
						const std::string& slotName = actionSlots[i];
						size_t slotLen = slotName.size();
						if (slotLen > OBJECT_DATA_STRLEN_ACTION_NAME)
							slotLen = OBJECT_DATA_STRLEN_ACTION_NAME;

						memcpy(actionSlot[i], slotName.c_str(), sizeof(char) * slotLen);
					}
				}
			}
		
			ObjectInfo(const ObjectInfo& other):
				speed(other.speed),
				initialState(other.initialState)
			{
				memcpy(name, other.name, OBJECT_DATA_STRLEN_NAME);
				memcpy(description, other.description, OBJECT_DATA_STRLEN_DESCRIPTION);
				for (int i = 0; i < TILE_STATE_MAX_action + 1; ++i)
					memcpy(actionSlot[i], other.actionSlot[i], OBJECT_DATA_STRLEN_ACTION_NAME);
			}
		};


		// Returns the size of data moving accross netw (excludes server and client specific data)
		size_t get_netw_objinfo_size();
		std::vector<ObjectInfo> load_obj_info_file(const std::string& filePath);


		class ObjectInstanceData
		{
		private:
			int _x = 0;
			int _z = 0;
			uint64_t _state = 0;
			PK_ubyte _objType = 0;
			std::queue<int> _actionQueue;
			Faction* _pFaction;

			// Current action's progress
			float _actionProgress = 0.0f;

		public:
			ObjectInstanceData(int x, int z, uint64_t currentState, Faction* pFaction) :
				_x(x), _z(z), _state(currentState), _pFaction(pFaction)
			{
				_objType = get_tile_thingid(currentState);
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
				_objType = get_tile_thingid(_state);
			}

			int getX() const { return _x; }
			int getZ() const { return _z; }
			uint64_t getTileState() const { return _state; }
			uint64_t getObjType() const { return _objType; }
			float& accessActionProgress() { return _actionProgress; }
			
			std::queue<int>& getActionQueue() { return _actionQueue; }
			void addAction(int actionID) { _actionQueue.push(actionID); }
			
			Faction* getFaction() { return _pFaction; }
		};
	}
}
