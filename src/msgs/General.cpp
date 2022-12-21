#include "General.h"
#include "Server.h"
#include "game/Game.h"
#include "General.h"
#include "game/objects/Object.h"


namespace msgs
{
    Message get_server_message(Server& server, Message& msg)
    {
        std::string message = "Welcome! Testing testing...";

        size_t bufSize = MESSAGE_MIN_DATA_SIZE + MOTD_LEN;
        PK_byte respData[bufSize];
        memset(respData, 0, bufSize);

        const int32_t messageType = MESSAGE_TYPE__GetServerMessage;
        memcpy(respData, &messageType, sizeof(int32_t));
        memcpy(respData + sizeof(int32_t), message.data(), message.size());

        return Message(NULL_CLIENT, respData, bufSize);
    }


    Message user_login(Server& server, Message& msg)
    {
        const size_t msgSize = msg.getSize();
        if(msgSize >= MESSAGE_MIN_DATA_SIZE + USER_NAME_LEN + USER_PASSWD_LEN)
        {
            const size_t dataBeginPos = MESSAGE_MIN_DATA_SIZE;
            PK_byte* data = msg.accessData();
            std::string usrName(data + dataBeginPos);
            std::string password(data + dataBeginPos + USER_PASSWD_LEN);
            const PK_byte success = server.validateCredentials(usrName, password);
            PK_byte respData[MESSAGE_MIN_DATA_SIZE + 1];

            const int32_t messageType = MESSAGE_TYPE__UserLogin;
            memcpy(respData, &messageType, sizeof(int32_t));
            memcpy(respData + sizeof(int32_t), &success, 1);

            return Message(NULL_CLIENT, respData, sizeof(int32_t) + 1);
        }
        else
        {
            return NULL_MESSAGE;
        }
    }


    Message fetch_obj_type_lib(Server& server, Message& msg)
    {
        return Game::get()->getObjInfoLibMsg();
    }


    Message create_new_faction(Server& server, Message& msg)
    {
        const size_t msgSize = msg.getSize();
        if(msgSize > MESSAGE_MIN_DATA_SIZE + 1)
        {
            std::string factionName(msg.accessData(), msgSize);
            return Game::get()->addFaction(factionName.c_str(), factionName.size());
        }
        else
        {
            return NULL_MESSAGE;
        }
    }


    Message update_observer(Server& server, Message& msg)
    {
        const size_t msgSize = msg.getSize();
        if(msgSize == sizeof(int32_t) * 4)
        {
            int32_t xPos = 0;
            int32_t zPos = 0;
            int32_t observeRadius = 0;

            PK_byte* msgData = msg.accessData();

            memcpy((void*)&xPos, (const void*)(msgData + sizeof(int32_t)), sizeof(int32_t));
            memcpy((void*)&zPos, (const void*)(msgData + sizeof(int32_t) * 2), sizeof(int32_t));
            memcpy((void*)&observeRadius, (const void*)(msgData + sizeof(int32_t) * 3), sizeof(int32_t));

            server.updateClientData(msg.getClient(), xPos, zPos, observeRadius);
        }
        return NULL_MESSAGE;
    }


    Message server_shutdown(Server& server, Message& msg)
    {
        Server::trigger_shutdown();
        std::string message = "Shutdown triggered";
        return Message(NULL_CLIENT, message.data(), message.size());
    }
}
