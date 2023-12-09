#include "General.h"
#include "Server.h"
#include "../../Proj8315Common/src/messages/GeneralMessages.h"
#include "../../Proj8315Common/src/messages/WorldMessages.h"
#include "../../Proj8315Common/src/Faction.h"
#include "game/Game.h"
#include "General.h"
#include "game/objects/Object.h"
#include <string>

using namespace gamecommon;

namespace msgs
{
    // TODO: Create message of type "GetServerMessage" which takes just takes the message string as parameter
    Message get_server_message(Server& server, const Client& client, Message& msg)
    {
        std::string message = "Welcome! Testing testing...";

        size_t bufSize = MESSAGE_SIZE__ServerMessageResponse;
        GC_byte respData[bufSize];
        memset(respData, 0, bufSize);

        const int32_t messageType = MESSAGE_TYPE__ServerMessage;
        memcpy(respData, &messageType, sizeof(int32_t));
        memcpy(respData + sizeof(int32_t), message.data(), message.size());

        // NOTE: just a hack atm
        return Message(respData, bufSize, bufSize);
    }


    Message user_login(Server& server, const Client& client, Message& msg)
    {
        LoginRequest loginReqMsg(msg.getData(), msg.getDataSize());
        if (loginReqMsg != NULL_MESSAGE)
        {
            const std::pair<bool, const Faction> validation = server.validateLoginReq(loginReqMsg);
            bool success = validation.first;
            Faction faction = validation.second;
            std::string errorMessage = "";
            if (!success)
            {
                errorMessage = "Invalid username or password";
                faction = NULL_FACTION;
            }
            if (!server.loginUser(client, loginReqMsg.getUsername()))
            {
                Debug::log("User login validation was successful but server refused to make ClientAddress - User pair @server::loginUser", Debug::MessageType::ERROR);
                errorMessage = "Failed to login";
                faction = NULL_FACTION;
                success = false;
            }
            LoginResponse resp(success, faction, errorMessage.data(), errorMessage.size());
            return resp;
        }
        Debug::log("Failed to construct LoginRequest message from incoming data", Debug::MessageType::WARNING);
        return NULL_MESSAGE;
    }


    Message user_register(Server& server, const Client& client, Message& msg)
    {
        UserRegisterRequest registerReqMsg(msg.getData(), msg.getDataSize());
        if (registerReqMsg != NULL_MESSAGE)
        {
            const std::string usrname = registerReqMsg.getUsername();
            const std::string passwd = registerReqMsg.getPassword();
            const std::string repasswd = registerReqMsg.getRePassword();
            std::string errorMessage = "";
            GC_byte success = 1;

            // Validate received username doesn't exist
            if (server.getUser(usrname) != NULL_USER)
            {
                Debug::log("User registering failed due to username: " + usrname + " already exists");
                errorMessage = "Invalid username";
                success = 0;
            }
            // Validate received passwords match
            else if (passwd != repasswd)
            {
                Debug::log("User registering failed due to received passwords didn't match");
                errorMessage = "Passwords needs to match";
                success = 0;
            }
            if (success)
            {
                Debug::log("New user registered: " + usrname + " name size=" + std::to_string(usrname.size()));
                server.createUser(client, usrname.data(), usrname.size(), passwd.data(), passwd.size());
            }
            return UserRegisterResponse(success, errorMessage.data());
        }
        return NULL_MESSAGE;
    }


    Message fetch_obj_type_lib(Server& server, const Client& client, Message& msg)
    {
        return ObjInfoLibMsg(Game::get()->getObjInfoLib());
    }


    Message update_observer(Server& server, const Client& client, Message& msg)
    {
        const User user = server.getUser(client);
        if (user == NULL_USER)
        {
            Debug::log(
                "Attempted to update observer for client: " + client.getAddress() +
                " but client's user was nullptr",
                Debug::MessageType::ERROR
            );
            return NULL_MESSAGE;
        }

        UpdateObserverMsg updateMsg(msg.getData(), msg.getDataSize());
        if (updateMsg != NULL_MESSAGE)
        {
            server.updateUserData(user, updateMsg.getX(), updateMsg.getZ(), updateMsg.getRadius());
        }

        // OLD BELOW
        /*
        const size_t msgSize = msg.getDataSize();
        if(msgSize == sizeof(int32_t) * 4)
        {
            int32_t xPos = 0;
            int32_t zPos = 0;
            int32_t observeRadius = 0;

            const GC_byte* msgData = msg.getData();

            memcpy((void*)&xPos, (const void*)(msgData + MESSAGE_ENTRY_SIZE__header), sizeof(int32_t));
            memcpy((void*)&zPos, (const void*)(msgData + MESSAGE_ENTRY_SIZE__header * 2), sizeof(int32_t));
            memcpy((void*)&observeRadius, (const void*)(msgData + MESSAGE_ENTRY_SIZE__header * 3), sizeof(int32_t));

            server.updateUserData(user, xPos, zPos, observeRadius);
        }
        */

        return NULL_MESSAGE;
    }


    //Message server_shutdown(Server& server, Message& msg)
    //{
    //    Server::trigger_shutdown();
    //    std::string message = "Shutdown triggered";
    //    return Message(NULL_CLIENT, message.data(), message.size());
    //}
}
