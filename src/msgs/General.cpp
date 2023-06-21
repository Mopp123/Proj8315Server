#include "General.h"
#include "Server.h"
#include "../../Proj8315Common/src/Faction.h"
#include "game/Game.h"
#include "General.h"
#include "game/objects/Object.h"
#include <string>

using namespace gamecommon;

namespace msgs
{
    Message get_server_message(Server& server, const Client& client, Message& msg)
    {
        std::string message = "Welcome! Testing testing...";

        size_t bufSize = MESSAGE_MIN_DATA_SIZE + MOTD_LEN;
        GC_byte respData[bufSize];
        memset(respData, 0, bufSize);

        const int32_t messageType = MESSAGE_TYPE__GetServerMessage;
        memcpy(respData, &messageType, sizeof(int32_t));
        memcpy(respData + sizeof(int32_t), message.data(), message.size());

        return Message(respData, bufSize);
    }


    Message user_login(Server& server, const Client& client, Message& msg)
    {
        // NEW
        LoginRequest loginReqMsg(msg.getData(), msg.getDataSize());
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
        return LoginResponse(success, faction, errorMessage.data(), errorMessage.size());

        // OLD BELOW \/
        /*
        if(msgSize >= MESSAGE_MIN_DATA_SIZE + USER_NAME_SIZE + USER_PASSWD_SIZE)
        {
            const size_t dataBeginPos = MESSAGE_MIN_DATA_SIZE;
            PK_byte* data = msg.accessData();

            PK_byte usernameData[USER_NAME_SIZE];
            memset(usernameData, 0, USER_NAME_SIZE);
            memcpy(usernameData, data + dataBeginPos, USER_NAME_SIZE);
            std::string usernameStr(usernameData, USER_NAME_SIZE);

            PK_byte passwordData[USER_PASSWD_SIZE];
            memset(passwordData, 0, USER_PASSWD_SIZE);
            memcpy(passwordData, data + dataBeginPos + USER_NAME_SIZE, USER_PASSWD_SIZE);

            const std::tuple<bool, bool, const Faction*> validation = server.validateCredentials(usernameData, passwordData);

            PK_byte success = std::get<0>(validation);
            PK_byte hasFaction = std::get<1>(validation);
            const Faction* faction = std::get<2>(validation);

            std::string errorMessage = success ? "" : "Incorrect username or password";
            const size_t factionSize = Faction::get_netw_size();
            PK_byte factionData[factionSize];
            memset(factionData, 0, factionSize);

            if (hasFaction)
            {
                if (faction == nullptr)
                {
                    std::string msg = "Server user validation identified user to have faction but the returned in game faction was nullptr!";
                    Debug::log(msg, Debug::MessageType::ERROR);
                    errorMessage = msg;
                    success = 0;
                }
                else
                {
                    memcpy(factionData, (PK_byte*)(&(*faction)), Faction::get_netw_size());
                }
            }

            if (success)
            {
                if (!server.loginUser(msg.getClient(), usernameStr))
                {
                    Debug::log("User login validation was successful but server refused to make ClientAddress - User pair @server::loginUser", Debug::MessageType::ERROR);
                    success = false;
                }
            }

            Message response(NULL_CLIENT, MESSAGE_TYPE__UserLogin, 2 + Faction::get_netw_size() + MESSAGE_INFO_MESSAGE_LEN);

            response.add(&success, 1);
            response.add(&hasFaction, 1);
            response.add(factionData, factionSize);
            response.add(errorMessage.data(), errorMessage.size());
            return response;
        }
        else
        {
            return NULL_MESSAGE;
        }
        */
    }


    Message user_register(Server& server, const Client& client, Message& msg)
    {
        const size_t msgSize = msg.getDataSize();
        if(msgSize >= MESSAGE_MIN_DATA_SIZE + USER_NAME_SIZE + USER_PASSWD_SIZE * 2)
        {
            const size_t dataBeginPos = MESSAGE_MIN_DATA_SIZE;
            const GC_byte* data = msg.getData();
            std::string usrname(data + dataBeginPos, USER_NAME_SIZE);
            std::string passwd(data + dataBeginPos + USER_PASSWD_SIZE, USER_PASSWD_SIZE);
            std::string repasswd(data + dataBeginPos + USER_PASSWD_SIZE * 2, USER_PASSWD_SIZE);

            GC_byte success = 1;

            GC_byte errMsgData[MESSAGE_INFO_MESSAGE_LEN];
            memset(errMsgData, 0, MESSAGE_INFO_MESSAGE_LEN);

            // Validate received username doesn't exist
            if (server.getUser(usrname) != NULL_USER)
            {
                Debug::log("User registering failed due to username: " + usrname + " already exists");
                std::string errMsg = "Invalid username";
                memcpy(errMsgData, errMsg.data(), errMsg.size());
                success = 0;
            }
            // Validate received passwords match
            else if (passwd != repasswd)
            {
                Debug::log("User registering failed due to received passwords didn't match");
                std::string errMsg = "Passwords needs to match";
                memcpy(errMsgData, errMsg.data(), errMsg.size());
                success = 0;
            }
            if (success)
            {
                Debug::log("New user registered: " + usrname + " name size=" + std::to_string(usrname.size()));
                server.createUser(client, usrname.data(), usrname.size(), passwd.data(), passwd.size());
            }
            std::string errMsgStr(errMsgData);

            int32_t msgType = MESSAGE_TYPE__UserRegister;
            size_t bufSize = sizeof(int32_t) + 1 + MESSAGE_INFO_MESSAGE_LEN;
            GC_byte pBuf[bufSize];
            memset(pBuf, 0, bufSize);
            memcpy(pBuf, &msgType, sizeof(int32_t));
            memcpy(pBuf + sizeof(int32_t), &success, 1);
            memcpy(pBuf + sizeof(int32_t) + 1, errMsgData, MESSAGE_INFO_MESSAGE_LEN);

            //Message response(MESSAGE_TYPE__UserRegister, nullptr, 1 + MESSAGE_INFO_MESSAGE_LEN);
            //response.add(&success, 1);
            //response.add(errMsgData, MESSAGE_INFO_MESSAGE_LEN);
            return Message(pBuf, bufSize);
        }
        else
        {
            return NULL_MESSAGE;
        }
    }


    Message fetch_obj_type_lib(Server& server, const Client& client, Message& msg)
    {
        return Game::get()->getObjInfoLibMsg();
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
        return NULL_MESSAGE;
    }


    //Message server_shutdown(Server& server, Message& msg)
    //{
    //    Server::trigger_shutdown();
    //    std::string message = "Shutdown triggered";
    //    return Message(NULL_CLIENT, message.data(), message.size());
    //}
}
