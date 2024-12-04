#include "Admin.h"
#include "Server.h"
#include "DatabaseManager.h"
#include "../../Proj8315Common/src/messages/AdminMessages.h"


using namespace gamecommon;


namespace msgs
{
    bool validate_admin_status(const std::string& usrname, const std::string passwd)
    {
        return false;
    }


    // Atm just attempt to spawn without sending any response
    // TODO: Maybe send some response in future to view errors on client side
    gamecommon::Message spawn_object(Server& server, const Client& client, gamecommon::Message& msg)
    {
        // TODO:
        // *validate client sending message is valid admin user
        // *spawn the actual object
        SpawnRequest spawnReq(msg.getData(), msg.getDataSize());
        if (spawnReq != NULL_MESSAGE)
        {
            const User user = server.getUser(client);
            const std::string& username = user.getName();
            if (!user.isAdmin())
            {
                Debug::log(
                    "User: " + username + " sent spawn request without being admin!",
                    Debug::MessageType::WARNING
                );
                return NULL_MESSAGE;
            }

            Debug::log("Handling admin spawn request message by user: " + username);
            GC_ubyte objectType = spawnReq.getObjectType();
            int32_t targetX = spawnReq.getTargetX();
            int32_t targetY = spawnReq.getTargetY();
            std::string factionName = user.getFactionName();
            if (factionName.empty())
                factionName = "Neutral";

            if (Game::get()->spawnObject(factionName, objectType, targetX, targetY))
            {
                Debug::log(
                    "Spawned object: " + std::to_string(objectType) + " "
                    "by admin: " + username + " "
                    " at: " + std::to_string(targetX) + ", " + std::to_string(targetY) + " "
                    "for faction: " + factionName
                );
            }
            else
            {
                Debug::log(
                    "Failed to spawn object: " + std::to_string(objectType) + " "
                    "by admin: " + username + " "
                    " at: " + std::to_string(targetX) + ", " + std::to_string(targetY) + " "
                    "for faction: " + factionName,
                    Debug::MessageType::ERROR
                );
            }
        }
        return NULL_MESSAGE;
    }


    gamecommon::Message terrain_modification(Server& server, const Client& client, gamecommon::Message& msg)
    {
        TerrainModRequest req(msg.getData(), msg.getDataSize());
        if (req != NULL_MESSAGE)
        {
            const User user = server.getUser(client);
            const std::string& username = user.getName();
            if (!user.isAdmin())
            {
                Debug::log(
                    "User: " + username + " sent terrain modification request without being admin!",
                    Debug::MessageType::WARNING
                );
                return NULL_MESSAGE;
            }

            Debug::log("Handling admin terrain modification request message by user: " + username);
            GC_ubyte radius = req.getRadius();
            int32_t targetX = req.getTargetX();
            int32_t targetY = req.getTargetY();
            GC_ubyte elevation = req.getElevation();
            GC_ubyte temperature = req.getTemperature();
            GC_ubyte terrainType = req.getTerrainType();

            if (elevation > TILE_STATE_MAX_terrElevation)
            {
                Debug::log(
                    "Failed to modify terrain by admin " + username + " "
                    "elevation value too high(" + std::to_string(elevation) + ") "
                    "Max elevation value is " + std::to_string(TILE_STATE_MAX_terrElevation),
                    Debug::MessageType::ERROR
                );
                return NULL_MESSAGE;
            }
            if (temperature > TILE_STATE_MAX_temperature)
            {
                Debug::log(
                    "Failed to modify terrain by admin " + username + " "
                    "temperature value too high(" + std::to_string(temperature) + ") "
                    "Max temperature value is " + std::to_string(TILE_STATE_MAX_temperature),
                    Debug::MessageType::ERROR
                );
                return NULL_MESSAGE;
            }
            if (terrainType > TILE_STATE_MAX_terrType)
            {
                Debug::log(
                    "Failed to modify terrain by admin " + username + " "
                    "terrain type value too high(" + std::to_string(terrainType) + ") "
                    "Max elevation value is " + std::to_string(TILE_STATE_MAX_terrType),
                    Debug::MessageType::ERROR
                );
                return NULL_MESSAGE;
            }

            // TODO: Some error messaging if invalid params?
            Game::get()->setTileState(
                radius,
                targetX,
                targetY,
                elevation,
                temperature,
                terrainType
            );

            Debug::log(
                "Modified terrain by admin " + username + " "
                "at location: " + std::to_string(targetX) + ", " + std::to_string(targetY) + " by radius of: " + std::to_string(radius) + " "
                "elevation: " + std::to_string(elevation) + " "
                "temperature: " + std::to_string(temperature) + " "
                "terrain type: " + std::to_string(terrainType)
            );
        }

        return NULL_MESSAGE;
    }
}
