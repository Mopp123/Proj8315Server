#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include "../Proj8315Common/src/Common.h"
#include "../Proj8315Common/src/messages/Message.h"
#include "../Proj8315Common/src/messages/GeneralMessages.h"
#include "game/Game.h"
#include "MessageHandler.h"
#include <unordered_map>
#include <cstring>

#include <thread>
#include <mutex>


class Server
{
private:
    std::unordered_map<std::string, std::string> _config;

    int _serverSD;
    int _port;
    sockaddr_in _address;

    size_t _maxClientCount = 1024;

    Game _game;
    MessageHandler _messageHandler;
    std::thread* _pClientMessageHandlerThread = nullptr;
    std::thread* _pWorldStateBroadcastThread = nullptr;
    std::thread* _pFactionStatesBroadcastThread = nullptr;
    std::thread* _gameThread = nullptr;

    mutable std::mutex _mutex;

    // Currently connected (and validated) clients
    // key = client's address
    // NOTE: Client may be accepted but not associated with user
    std::unordered_map<std::string, Client> _clients;
    // std::unordered_map<std::string, gamecommon::User> _users; // TODO: Delete this (Unnecessary after db integration)

    // Logged in client addr - user mapping
    std::unordered_map<std::string, gamecommon::User> _clientUserMapping;

    static bool s_shutdown;

public:
    Server(const std::string configFilePath, size_t maxClientCount);
    ~Server();

    void beginMsgHandler();
    void beginGame();
    void run();
    void shutdown();

    // Removes connection (thread safe)
    void disconnectClient(const Client& client);

    void updateUserData(const Client& client, int32_t xPos, int32_t zPos, int32_t observeRadius);
    void updateUserFaction(const gamecommon::User& user, const gamecommon::Faction& faction);

    // Returns vector containing each connection sock. desc. (Thread safely)
    std::unordered_map<std::string, Client> getClientConnections() const;

    gamecommon::User getUser(const Client& client);
    void loginUser(const Client& client, const gamecommon::User& user);
    bool logoutUser(const Client& client, const gamecommon::User& user);

    // TODO: delete
    // bool createUser(
    //     const Client& client,
    //     const GC_byte* usernameData,
    //     size_t usernameSize,
    //     const GC_byte* passwdData,
    //     size_t passwdSize
    // );

    // TODO: delete
    // Return values: first = isValid second = user's faction (NULL_FACTION if user doesn't have faction)
    // std::pair<bool, gamecommon::Faction> validateLoginReq(const gamecommon::LoginRequest& msg) const;

    static void trigger_shutdown();
    static bool is_shutting_down();

private:
    std::unordered_map<std::string, std::string> readConfigFile(const std::string filePath) const;
};

