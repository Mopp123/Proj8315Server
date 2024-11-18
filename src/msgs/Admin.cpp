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
}
