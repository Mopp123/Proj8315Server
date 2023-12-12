#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Object.h"
#include "Debug.h"
#include "DatabaseManager.h"
#include "../../../Proj8315Common/src/Tile.h"


using namespace gamecommon;

namespace world
{
    namespace objects
    {
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

                GC_ubyte speed = (GC_ubyte)std::stoi(rawInfo[2 + (TILE_STATE_MAX_action + 1)]);

                uint64_t initialTileState = 0;
                gamecommon::set_tile_thingid(initialTileState, i);

                allObjectInfo.push_back({name, description, actions, speed, initialTileState});
                i++;
            }

            return allObjectInfo;
        }


        std::vector<gamecommon::ObjectInfo> init_obj_info_db(const std::string& filePath)
        {
            std::vector<gamecommon::ObjectInfo> objInfoLib;

            QueryResult queryResult = DatabaseManager::exec_query(
                "SELECT * FROM objects;"
            );
            switch (queryResult.status)
            {
                case QUERY_STATUS__SUCCESS:
                    for (int i = 0; i < queryResult.result.size(); ++i)
                    {
                        const std::string& name = queryResult.getValue<std::string>(
                            i, DATABASE_COLUMN__OBJECTS__NAME
                        );
                        const std::string& description = queryResult.getValue<std::string>(
                            i, DATABASE_COLUMN__OBJECTS__DESCRIPTION
                        );

                        // std::vector<std::string> actions;
                        // for (int i = 0; i < TILE_STATE_MAX_action + 1; ++i)
                        //     actions.push_back(rawInfo[2 + i]);

                        // GC_ubyte speed = (GC_ubyte)std::stoi(rawInfo[2 + (TILE_STATE_MAX_action + 1)]);

                        // uint64_t initialTileState = 0;
                        // gamecommon::set_tile_thingid(initialTileState, i);

                        // allObjectInfo.push_back({name, description, actions, speed, initialTileState});
                    }
                    break;
                case QUERY_STATUS__SYNTAX_ERROR:
                    Debug::log(
                        "@world::objects::init_obj_info_db "
                        "Failed to construct object info lib due to query syntax error: " + queryResult.errorMsg,
                        Debug::MessageType::FATAL_ERROR
                    );
                    break;
                case QUERY_STATUS__CONNECTION_ERROR:
                    Debug::log(
                        "@world::objects::init_obj_info_db "
                        "Failed to construct object info lib due to database connection error: " + queryResult.errorMsg,
                        Debug::MessageType::FATAL_ERROR
                    );
                    break;
                case QUERY_STATUS__UNEXPECTED_ERROR:
                    Debug::log(
                        "@world::objects::init_obj_info_db "
                        "Failed to construct object info lib due to unexpected error: " + queryResult.errorMsg,
                        Debug::MessageType::FATAL_ERROR
                    );
                    break;
                default:
                    Debug::log(
                        "@world::objects::init_obj_info_db "
                        "Failed to construct object info lib due to invalid query result status: " + std::to_string(queryResult.status),
                        Debug::MessageType::FATAL_ERROR
                    );
                    break;
            }
            return objInfoLib;
        }
    }
}
