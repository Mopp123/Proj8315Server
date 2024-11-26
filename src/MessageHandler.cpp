#include <mutex>
#include <string>
#include <unistd.h>
#include <vector>
#include <thread>
#include <chrono>

#include "../Proj8315Common/src/Common.h"
#include "MessageHandler.h"
#include "Server.h"
#include "msgs/General.h"
#include "msgs/WorldState.h"
#include "msgs/Admin.h"
#include "Debug.h"


using namespace gamecommon;


MessageHandler::MessageHandler(Server& server, Game& game) :
    _serverRef(server), _gameRef(game)
{
    _pRecvBuf = new GC_byte[_maxRecvBufLen];
    memset(_pRecvBuf, 0, _maxRecvBufLen);

    _msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__ServerInfo, msgs::get_server_info));
    _msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__LoginRequest, msgs::user_login));
    _msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__LogoutRequest, msgs::user_logout));
    _msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__UserRegisterRequest, msgs::user_register));
    _msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__ObjInfoLibRequest, msgs::fetch_obj_type_lib));
    _msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__CreateFactionRequest, msgs::create_new_faction));
    _msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__UpdateObserverProperties, msgs::update_observer));
    _msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__FactionListRequest, msgs::get_all_factions));

    // Admin messages
    _msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__SpawnRequest, msgs::spawn_object));
    _msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__TerrainModRequest, msgs::terrain_modification));

    //_msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__GetWorldState, msgs::msg_fetchWorldState));
    //_msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__ServerShutdown, msgs::server_shutdown));
}

MessageHandler::~MessageHandler()
{
    delete[] _pRecvBuf;
}

void MessageHandler::handleClientMessages()
{
    while(_run)
    {
        // Check for new messages from each client connection
        std::unordered_map<std::string, Client> currentClients = _serverRef.getClientConnections();
        for (const std::pair<std::string, Client> client : currentClients)
        {
            ssize_t readBytes = read(client.second.getConnSD(), _pRecvBuf, _maxRecvBufLen);

            if (readBytes > 0)
            {
                ssize_t currentOffset = 0;

                // NOTE: Atm with websockify multiple messages may be concatenated
                //  -> attempt to deal with that
                // NOTE: If too many concatenated messages we fuck up since receive buffer is set to be
                // quite small atm!
                while (currentOffset < readBytes)
                {
                    GC_byte* pMessageBuf = _pRecvBuf + currentOffset;
                    int32_t messageType = *((int32_t*)pMessageBuf);
                    size_t messageSize = get_message_size(messageType);

                    Message msg(pMessageBuf, messageSize, messageSize);
                    Message response = processMessage(client.second, msg);
                    if (response != NULL_MESSAGE)
                    {
                        std::lock_guard<std::mutex> lock(_mutex);
                        ssize_t sentBytes = send(
                            client.second.getConnSD(),
                            response.getData(),
                            response.getDataSize(),
                            MSG_NOSIGNAL
                        );
                        if (sentBytes < 0)
                            Debug::log("ERROR ON SENDING!");
                        else
                            Debug::log("Sent response message to req of type: " + std::to_string(msg.getType()) + " size = " + std::to_string(response.getDataSize()));
                    }
                    currentOffset += messageSize;
                }
            }
            memset(_pRecvBuf, 0, _maxRecvBufLen);
        }
    }
}

void MessageHandler::broadcastWorldState()
{
    while(_run)
    {
        // NOTE: HARDCODED ONLY FOR TESTING ATM!!!
        //std::this_thread::sleep_for(std::chrono::milliseconds(600));
        std::this_thread::sleep_for(std::chrono::milliseconds(250));

        // Send game world state for all clients
        std::unordered_map<std::string, Client> currentClients = _serverRef.getClientConnections();
        for (const std::pair<std::string, Client> client : currentClients)
        {
            const User user = _serverRef.getUser(client.second);
            if (user != NULL_USER)
            {
                const int& x = user.getX();
                const int& z = user.getZ();
                const int& observeRadius = user.getObserveRadius();
                Message worldStateMsg = _gameRef.getWorldState(x, z, observeRadius);

                std::lock_guard<std::mutex> lock(_mutex);
                ssize_t sentBytes = send(
                    client.second.getConnSD(),
                    worldStateMsg.getData(),
                    worldStateMsg.getDataSize(),
                    MSG_NOSIGNAL
                );
                // TODO: Better probing for dropped connections!
                if (sentBytes <= 0)
                {
                    Debug::log("___TEST___disconnecting client due to no sent bytes");
                    _serverRef.disconnectClient(client.second);
                }
            }
        }
    }
}

void MessageHandler::broadcastFactionStates()
{
    while(_run)
    {
        // Send every changed faction's data for all clients
        Message changedFactionsMsg = _gameRef.getChangedFactions();
        // Send game world state for all clients

        if (changedFactionsMsg != NULL_MESSAGE)
        {
            Debug::log("___TEST___FACTIONS UPDATED -> sending to clients...");
            //Debug::log("Changed faction msg: " + std::to_string(changedFactionsMsg.getType()));

            std::unordered_map<std::string, Client> currentClients = _serverRef.getClientConnections();
            for (const std::pair<std::string, Client> client : currentClients)
            {
                std::lock_guard<std::mutex> lock(_mutex);
                ssize_t sentBytes = send(
                    client.second.getConnSD(),
                    changedFactionsMsg.getData(),
                    changedFactionsMsg.getDataSize(),
                    MSG_NOSIGNAL
                );
                //Debug::log(
                //    "___TEST___sent changed factions to: " + client.second.getAddress() +
                //    " Sent bytes: " + std::to_string(sentBytes)
                //);

                // TODO: Better probing for dropped connections!
                if (sentBytes <= 0)
                {
                    Debug::log("___TEST___disconnecting client due to no sent bytes");
                    _serverRef.disconnectClient(client.second);
                }
            }
            _gameRef.resetChangedFactionsStatus();
        }
    }
}

Message MessageHandler::processMessage(const Client& client, Message& msg)
{
    const int32_t msgType = msg.getType();

    auto iter = _msgFuncMapping.find(msgType);
    if(iter != _msgFuncMapping.end())
    {
        return (*_msgFuncMapping[msgType])(_serverRef, client, msg);
    }
    else
    {
        Debug::log("Failed to process message. Couldn't find message name: " + std::to_string(msg.getType()));
        return NULL_MESSAGE;
    }
}
