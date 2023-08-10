#pragma once

// #include <cstring>
#include <string>
#include <vector>
#include <queue>
#include <mutex>

#include "../Proj8315Common/src/Common.h"
#include "../Proj8315Common/src/messages/Message.h"
#include "Debug.h"
#include "Client.h"
#include "game/Game.h"


//#define USER_ID_LEN 32
//#define MESSAGE_MAX_DATA_SIZE (100*100*8)
//#define MESSAGE_MIN_DATA_SIZE sizeof(int32_t)
//
//#define MESSAGE_ENTRY_SIZE__header sizeof(int32_t)
//
//#define NULL_MESSAGE Message(NULL_CLIENT, nullptr, 0)
//
//#define MESSAGE_INFO_MESSAGE_LEN 256
//
//#define MESSAGE_TYPE__GetServerMessage          0x1
//#define MESSAGE_TYPE__UserLogin            	0x2
//#define MESSAGE_TYPE__UserLogout            	0x3
//#define MESSAGE_TYPE__UserRegister            	0x4
//#define MESSAGE_TYPE__GetObjInfoLib             0x5
//#define MESSAGE_TYPE__CreateFaction             0x6
//#define MESSAGE_TYPE__EditFaction               0x7
//#define MESSAGE_TYPE__GetWorldState             0x8
//#define MESSAGE_TYPE__GetAllFactions            0x9
//#define MESSAGE_TYPE__GetChangedFactions        0x10
//#define MESSAGE_TYPE__UpdateObserverProperties 	0x11
//#define MESSAGE_TYPE__Deploy 	                0x12
//#define MESSAGE_TYPE__ServerShutdown            0x11

/*
class Message
{
private:
    Client _client = NULL_CLIENT; // client which sent this message
    PK_byte* _pData = nullptr;
    size_t _totalDataSize = 0;
    // Used to deterimne where to add data if calling "add" -func
    int _currentDataPtr = 0;

public:
    Message(const Client& client, char* pData, size_t totalDataSize);
    Message(const Client& client, uint32_t messageType, size_t totalDataSize);
    Message(const Message& other);
    ~Message();

    void add(PK_byte* data, size_t dataSize);
    void incrWritePos(size_t size);
    int32_t getType() const;

    inline const Client& getClient() const { return _client; }
    inline PK_byte* accessData() { return _pData; }
    inline size_t getSize() const { return _totalDataSize; }

    bool operator==(const Message& other)
    {
        bool dataEqual = false;
        if (_totalDataSize == other._totalDataSize && _pData != nullptr && other._pData != nullptr)
            dataEqual = memcmp(_pData, other._pData, _totalDataSize) == 0;
        else
            dataEqual = _pData == nullptr && other._pData == nullptr;
        return _client == other._client &&
            _totalDataSize == other._totalDataSize &&
            dataEqual;
    }

    bool operator!=(const Message& other)
    {
        bool dataEqual = false;
        if (_totalDataSize == other._totalDataSize && _pData != nullptr && other._pData != nullptr)
            dataEqual = memcmp(_pData, other._pData, _totalDataSize) == 0;
        else
            dataEqual = _pData == nullptr && other._pData == nullptr;
        return _client != other._client ||
            _totalDataSize != other._totalDataSize ||
            !dataEqual;
    }
};
*/


class Server;


class MessageHandler
{
private:
    Server& _serverRef;
    Game& _gameRef;

    const size_t _maxRecvBufLen = 512;
    GC_byte* _pRecvBuf = nullptr;

    mutable std::mutex _mutex;

    // specifies what happens on each different kind of client message
    std::unordered_map<int32_t, gamecommon::Message(*)(Server& server, const Client& client, gamecommon::Message&)> _msgFuncMapping;

    bool _run = true;

public:
    MessageHandler(Server& server, Game& game);
    ~MessageHandler();

    // Reads client messages and responds accordingly
    void handleClientMessages();
    // Broadcasts world state("tiles") to all clients
    void broadcastWorldState();
    // Broadcasts each in game faction's state to all clients
    // but ONLY if faction's state has changed
    void broadcastFactionStates();

private:
    gamecommon::Message processMessage(const Client& client, gamecommon::Message& msg);

};

