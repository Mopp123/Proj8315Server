#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include "Common.h"
#include "game/Game.h"
#include "MessageHandler.h"
#include <unordered_map>
#include <cstring>

#include <thread>
#include <mutex>


class Server
{
private:
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
    std::unordered_map<std::string, User> _users;

    // Logged in client addr - user mapping
    std::unordered_map<std::string, User*> _clientUserMapping;

    static bool s_shutdown;

public:
    Server(int port, size_t maxClientCount);
    ~Server();

    void beginMsgHandler();
    void beginGame();
    void run();
    void shutdown();

    // Removes connection (thread safe)
    void disconnectClient(const Client& client);

    void updateUserData(const User& user, int32_t xPos, int32_t zPos, int32_t observeRadius);
    void updateUserFaction(const User& user, const Faction& faction);

    // Returns vector containing each connection sock. desc. (Thread safely)
    std::unordered_map<std::string, Client> getClientConnections() const;
    User getUser(const std::string& name);
    User getUser(const Client& client);
    bool loginUser(const Client& client, const std::string& username);
    bool createUser(
        const Client& client,
        const PK_byte* usernameData,
        size_t usernameSize,
        const PK_byte* passwdData,
        size_t passwdSize
    );

    // Return values: { validationStatus, hasFaction, factionData }
    std::tuple<bool, bool, const Faction*> validateCredentials(
        const PK_byte* pUsernameData,
        const PK_byte* pPasswordData
    ) const;

    static void trigger_shutdown();
    static bool is_shutting_down();
};

