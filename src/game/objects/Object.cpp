#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Object.h"
#include "Debug.h"
#include "game/world/Tile.h"


namespace world
{
	namespace objects
	{
		size_t get_netw_objinfo_size()
		{
			size_t combinedStrLen = (OBJECT_DATA_STRLEN_NAME + OBJECT_DATA_STRLEN_DESCRIPTION);
			for (int i = 0; i < TILE_STATE_MAX_action + 1; ++i)
				combinedStrLen += OBJECT_DATA_STRLEN_ACTION_NAME;
			return combinedStrLen + 1;
		}


		std::vector<ObjectInfo> load_obj_info_file(const std::string& filePath)
		{
			std::fstream fileStream(filePath);
			if (!fileStream.is_open())
				throw std::runtime_error("Failed to open obj info file from: " + filePath);
			std::string line;
			
			/* Current obj info schema in the config file:
				name
				description
				action0
				action1
				action2
				action3
				action4
				action5
				action6
				action7
				speed
				---
			*/
			const std::string objDelim = "---";
			std::vector<std::string> currentRawInfo;
			std::vector<std::vector<std::string>> allRawInfo;
			while (std::getline(fileStream, line))
			{
				if (line.compare(objDelim) == 0)
				{
					allRawInfo.push_back(currentRawInfo);
					currentRawInfo.clear();
				}
				else
				{
					currentRawInfo.push_back(line);
				}
			}
			fileStream.close();

			// Conv those strings into actual ObjectInfo - objects
			std::vector<ObjectInfo> allObjectInfo;
			int i = 0;
			for (std::vector<std::string>& rawInfo : allRawInfo)
			{
				const std::string& name = rawInfo[0];
				const std::string& description = rawInfo[1];
				
				std::vector<std::string> actions;
				for (int i = 0; i < TILE_STATE_MAX_action + 1; ++i)
					actions.push_back(rawInfo[2 + i]);
				
				PK_ubyte speed = (PK_ubyte)std::stoi(rawInfo[2 + (TILE_STATE_MAX_action + 1)]);
				
				uint64_t initialTileState = 0;
				world::set_tile_thingid(initialTileState, i);

				allObjectInfo.push_back({name, description, actions, speed, initialTileState});
				i++;
			}
			
			return allObjectInfo;
		}
	}
}