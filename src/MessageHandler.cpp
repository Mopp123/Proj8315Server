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
#include "Debug.h"


using namespace gamecommon;

/*
Message::Message(const Client& client, char* pData, size_t totalDataSize) :
    _client(client), _totalDataSize(totalDataSize)
{
    if (pData && _totalDataSize >= MESSAGE_MIN_DATA_SIZE && _totalDataSize <= MESSAGE_MAX_DATA_SIZE)
    {
        _pData = new PK_byte[_totalDataSize];
        memcpy(_pData, pData, _totalDataSize);
    }
    else if (client != NULL_CLIENT)
    {
        std::string dataStr(pData, totalDataSize);
        Debug::log(
            "Attempted to create Message instance with invalid data. raw data: " + dataStr +
            " data size: " + std::to_string(totalDataSize) +
            " Minimum data size is: " + std::to_string(MESSAGE_MIN_DATA_SIZE) +
            " Maximum data size is: " + std::to_string(MESSAGE_MAX_DATA_SIZE)
        );
    }
}

Message::Message(const Client& client, uint32_t messageType, size_t totalDataSize) :
    _client(client), _totalDataSize(totalDataSize)
{
    _totalDataSize += sizeof(uint32_t);
    if (_totalDataSize >= MESSAGE_MIN_DATA_SIZE && _totalDataSize <= MESSAGE_MAX_DATA_SIZE)
    {
        _pData = new PK_byte[_totalDataSize];
        memset(_pData, 0, totalDataSize);
    }
    else
    {
        Debug::log("Attempted to create Message instance with invalid size: " +
            std::to_string(totalDataSize) +
            " Minimum size: " + std::to_string(MESSAGE_MIN_DATA_SIZE) +
            " Maximum data size: " + std::to_string(MESSAGE_MAX_DATA_SIZE)
        );
        return;
    }

    // NOTE: Note sure is this pointing to right place? TODO: make sure!!!!
    add((PK_byte*)&messageType, sizeof(uint32_t));
}

Message::Message(const Message& other) :
    _client(other._client), _totalDataSize(other._totalDataSize)
{
    if (other._pData && other._totalDataSize >= MESSAGE_MIN_DATA_SIZE)
    {
        _totalDataSize = other._totalDataSize;
        _pData = new PK_byte[_totalDataSize];
        memcpy(_pData, other._pData, _totalDataSize);
    }
}

Message::~Message()
{
    delete[] _pData;
}

void Message::add(PK_byte* data, size_t dataSize)
{
    if (_pData == nullptr)
    {
        Debug::log("Attempted to add data to Message but underlying data buffer was nullptr");
        return;
    }
    if (_currentDataPtr + dataSize > _totalDataSize)
    {
        Debug::log(
            "Attempted to add data to Message but inputted data size went out of bounds of allocated space."
            " Current byte position: " + std::to_string(_currentDataPtr) +
            " Size to add: " + std::to_string(dataSize) +
            " Allocated size: " + std::to_string(_totalDataSize)
        );
        return;
    }
    memcpy(_pData + _currentDataPtr, data, dataSize);
    _currentDataPtr += dataSize;
}

void Message::incrWritePos(size_t size)
{
    _currentDataPtr += size;
}

int32_t Message::getType() const
{
    int32_t type = -1;
    if(_pData && _totalDataSize >= MESSAGE_MIN_DATA_SIZE)
        memcpy(&type, _pData, MESSAGE_ENTRY_SIZE__header);

    return type;
}
*/

// ---------------------------------------------------------
/*
void MessageQueue::push(const Message& msg)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _messages.push(msg);
}

Message MessageQueue::pop()
{
    std::lock_guard<std::mutex> lock(_mutex);
    if(!_messages.empty())
    {
        Message front = _messages.front();
        _messages.pop();
        return front;
    }
    else
    {
        return NULL_MESSAGE;
    }

}

bool MessageQueue::isEmpty() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _messages.empty();
}
*/

// ---------------------------------------------------------

MessageHandler::MessageHandler(Server& server, Game& game) :
    _serverRef(server), _gameRef(game)
{
    _pRecvBuf = new GC_byte[_maxRecvBufLen];
    memset(_pRecvBuf, 0, _maxRecvBufLen);

    _msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__ServerMessage, msgs::get_server_message));
    _msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__UserLogin, msgs::user_login));
    _msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__UserRegister, msgs::user_register));
    _msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__ObjInfoLib, msgs::fetch_obj_type_lib));
    _msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__CreateFaction, msgs::create_new_faction));
    _msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__UpdateObserverProperties, msgs::update_observer));
    _msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__GetAllFactions, msgs::get_all_factions));
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
                Message msg(_pRecvBuf, readBytes, readBytes, MESSAGE_MAX_SIZE);
                Message response = processMessage(client.second, msg);

                if (response != NULL_MESSAGE)
                {
                    //std::string rawStr(response.getData(), response.getDataSize());

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
                        Debug::log("Sent response message: " + std::to_string(msg.getType()));
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
        continue;
        // Send every changed faction's data for all clients
        Message changedFactionsMsg = _gameRef.getChangedFactions();
        // Send game world state for all clients

        if (changedFactionsMsg != NULL_MESSAGE)
        {
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
