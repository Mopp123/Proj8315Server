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

