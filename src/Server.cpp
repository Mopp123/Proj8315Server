#include <algorithm>
#include <asm-generic/socket.h>
#include <mutex>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <string>
#include <fstream>
#include <stdexcept>

#include "Server.h"
#include "DatabaseManager.h"
#include "utils/StringUtil.h"
#include "Debug.h"

#include <iostream>


using namespace gamecommon;

bool Server::s_shutdown = false;

Server::Server(const std::string configFilePath, size_t maxClientCount) :
    // _port(port), _maxClientCount(maxClientCount),
    _game(512), // *NOTE! size of the game world is just temporarely hardcoded here!
    _messageHandler(*this, _game)
{
    Debug::log("Reading config file from: " + configFilePath);
    _config = readConfigFile(configFilePath);
    Debug::log("Using configuration:");
    for (const std::pair<std::string, std::string> p : _config)
    {
        Debug::log("    " + p.first + ": " + p.second);
    }

    if (_config.find("port") != _config.end())
    {
        try
        {
            _port = std::stoi(_config["port"]);
            Debug::log("Assigned port to: " + _config["port"]);
        }
        catch(const std::invalid_argument& e)
        {
            Debug::log(
                "Failed to assign port from config file. Unable to cast: " + _config["port"] + " to int!",
                Debug::MessageType::ERROR
            );
        }
    }

    // Create socket
    _serverSD = socket(AF_INET, SOCK_STREAM, 0);
    if(_serverSD < 0)
    {
        Debug::log("Failed to create socket");
    }
    // NOTE: "Server thread"(main thread) is only accepting or declining connections
    //  -> no need to be in non blocking mode!
    //fcntl(_serverSD, F_SETFL, O_NONBLOCK);

    // Set reusable for development..
    int opt_reuse = 1;
    if (setsockopt(_serverSD, SOL_SOCKET, SO_REUSEADDR, &opt_reuse, (socklen_t)(sizeof(int))) < 0)
        Debug::log("Failed to set sockopt SO_REUSEADDR");

    // Configure address
    memset(&_address, 0, sizeof(_address));
    _address.sin_family = AF_INET;
    _address.sin_addr.s_addr = INADDR_ANY;
    _address.sin_port = htons(_port);

    // Bind socket to the address..
    if(bind(_serverSD, (struct sockaddr *)&_address, sizeof(_address)) < 0)
    {
        Debug::log("Failed to bind socket to address");
    }

    const int maxSockQueLen = 10; // how many connections may wait for acceptance simultaniously
    listen(_serverSD, maxSockQueLen);
    Debug::log("Started server on port: " + std::to_string(_port));

    // Establish database connection
    // if (!DatabaseManager::connect("127.0.0.1", 5432, "test_db", "postgres", "asd"))
    // {
    //     Debug::log(
    //         "Failed to create server due to database connection error",
    //         Debug::MessageType::FATAL_ERROR
    //     );
    //     return;
    // }
    // Debug::log(
    //     "Database connection established successfully"
    // );



    // TESTING!
    // add few users
    // for (int i = 1; i < 7; ++i)
    // {
    //     std::string username = "test" + std::to_string(i);
    //     std::string password = "test" + std::to_string(i);

    //     GC_byte nameData[USER_NAME_SIZE];
    //     GC_byte passwdData[USER_PASSWD_SIZE];
    //     memset(nameData, 0, USER_NAME_SIZE);
    //     memset(passwdData, 0, USER_PASSWD_SIZE);
    //     memcpy(nameData, username.data(), username.size());
    //     memcpy(passwdData, password.data(), password.size());

    //     _users[std::string(nameData, USER_NAME_SIZE)] = User(nameData, USER_NAME_SIZE, passwdData, USER_PASSWD_SIZE);
    //     Debug::log("Created test user: " + username);
    // }
}

Server::~Server()
{
}

void Server::beginMsgHandler()
{
    if(!_pClientMessageHandlerThread)
        _pClientMessageHandlerThread = new std::thread(&MessageHandler::handleClientMessages, &_messageHandler);
    else
        Debug::log("Attempted to launch MessageHandler::handleClientMessages multiple times!");

    if(!_pWorldStateBroadcastThread)
        _pWorldStateBroadcastThread = new std::thread(&MessageHandler::broadcastWorldState, &_messageHandler);
    else
        Debug::log("Attempted to launch MessageHandler::broadcastWorldState multiple times!");

    if(!_pFactionStatesBroadcastThread)
        _pFactionStatesBroadcastThread = new std::thread(&MessageHandler::broadcastFactionStates, &_messageHandler);
    else
        Debug::log("Attempted to launch MessageHandler::broadcastFactionStates multiple times!");
}

void Server::beginGame()
{
    if(!_gameThread)
        _gameThread = new std::thread(&Game::run, &_game);
    else
        Debug::log("Attempted to launch Game multiple times!");
}

void Server::run()
{
    sockaddr_in clientAddress;
    memset(&clientAddress, 0, sizeof(clientAddress));
    socklen_t clientLen = sizeof(clientAddress);
    int connSD = accept(_serverSD, (struct sockaddr*)&clientAddress, &clientLen);

    // TODO: connection validation (using initial message) before adding to "connected clients"
    if (connSD >= 0)
    {
        // Get details of conn..
        getpeername(connSD, (struct sockaddr*)&clientAddress, &clientLen);
        char* clientAddrName = inet_ntoa(clientAddress.sin_addr);
        unsigned short clientPort = ntohs(clientAddress.sin_port);
        std::string clientAddr = std::string(clientAddrName) + ":" + std::to_string(clientPort);

        if (_clients.find(clientAddr) == _clients.end())
        {
            Debug::log("Attempting to connect new client");
            // Make this connection socket non blocking
            fcntl(connSD, F_SETFL, O_NONBLOCK);

            std::lock_guard<std::mutex> lock(_mutex);
            // TODO: some kind of validation stuff?
            _clients[clientAddr] = Client(clientAddr, connSD);
        }
        else
        {
            Debug::log("Double connecting prevented");
        }
    }
}

// TODO: Safe and "complete" server shutdown func
void Server::shutdown()
{
    // Destroy Game
    if (_gameThread)
    {
        _gameThread->join();
        delete _gameThread;
    }

    // Close client connections
    for (const std::pair<std::string, Client> client : _clients)
        disconnectClient(client.second);

    // Destroy MessageHandler
    if (_pClientMessageHandlerThread)
    {
        _pClientMessageHandlerThread->join();
        delete _pClientMessageHandlerThread;
    }
    if (_pWorldStateBroadcastThread)
    {
        _pWorldStateBroadcastThread->join();
        delete _pWorldStateBroadcastThread;
    }
    if (_pFactionStatesBroadcastThread)
    {
        _pFactionStatesBroadcastThread->join();
        delete _pFactionStatesBroadcastThread;
    }

    close(_serverSD);
}

std::unordered_map<std::string, Client> Server::getClientConnections() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _clients;
}

User Server::getUser(const Client& client)
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _clientUserMapping.find(client.getAddress());
    if (it != _clientUserMapping.end())
        return it->second;
    return NULL_USER;
}

// TODO: Make this actually login the user, like logoutUser below..
void Server::loginUser(const Client& client, const gamecommon::User& user)
{
    Debug::log("___TEST___attempting login user: " + user.getName() + " : " + user.getID());
    std::lock_guard<std::mutex> lock(_mutex);
    _clientUserMapping[client.getAddress()] = user;
}

bool Server::logoutUser(const Client& client, const gamecommon::User& user)
{
    Debug::log("Attempting logout user: " + user.getName() + " : " + user.getID());
    std::lock_guard<std::mutex> lock(_mutex);
    const std::string& clientAddr = client.getAddress();
    std::unordered_map<std::string, Client>::const_iterator it = _clients.find(clientAddr);
    if (it != _clients.end())
    {
        std::unordered_map<std::string, User>::const_iterator itUserMapping = _clientUserMapping.find(clientAddr);
        if (itUserMapping != _clientUserMapping.end())
        {
            QueryResult setLoggedOutResult = DatabaseManager::exec_query(
                "UPDATE users SET logged_in=FALSE WHERE id='" + user.getID() + "';"
            );
            if (setLoggedOutResult.status == QUERY_STATUS__SUCCESS)
            {
                Debug::log("User: " + user.getName() + " logged out successfully");
                _clientUserMapping.erase(itUserMapping);
                return true;
            }
            else
            {
                // NOTE: Don't remember is errorMsg working atm...
                Debug::log(
                    "@Server::logoutUser "
                    "Failed to logout user: " + user.getName() + " "
                    "due to database error: " + setLoggedOutResult.errorMsg,
                    Debug::MessageType::ERROR
                );
            }
        }
        else
        {
            Debug::log(
                "@Server::logoutUser "
                "Failed to find client's(" + clientAddr + ") "
                "user from clientUserMapping",
                Debug::MessageType::ERROR
            );
        }
    }
    else
    {
        Debug::log(
            "@Server::logoutUser "
            "Failed to logout user: " + user.getName() +
            ". Client wasn't found from connected clients",
            Debug::MessageType::ERROR
        );
    }
    return false;
}


//TODO: delete
// bool Server::createUser(
//     const Client& client,
//     const GC_byte* usernameData,
//     size_t usernameSize,
//     const GC_byte* passwdData,
//     size_t passwdSize
// )
// {
//     std::string usernameStr(usernameData, usernameSize);
//     if (_users.find(usernameStr) == _users.end())
//     {
//         Debug::log("Created new user: " + usernameStr + " to server");
//         _users[usernameStr] = User(usernameData, usernameSize, passwdData, passwdSize);
//         return true;
//     }
//     return false;
// }

// TODO: delete
// std::pair<bool, Faction> Server::validateLoginReq(const gamecommon::LoginRequest& msg) const
// {
//     const std::string& reqUsername = msg.getUsername();
//     const GC_byte* pReqPasswordData = msg.getPasswordData();
//     bool isValid = false;
//     Faction userFaction = NULL_FACTION;
//
//     for (auto u : _users)
//     {
//         bool ans = u.second.getName() == reqUsername;
//         Debug::log(
//             "___TEST___@validation: testing " + reqUsername +
//             "(" + std::to_string(reqUsername.size()) + ") against " + u.second.getName() + " (" +
//             std::to_string(u.second.getName().size()) + ") result: " + std::to_string(ans)
//         );
//     }
//
//     auto it = _users.find(reqUsername);
//     if (it != _users.end())
//     {
//         Debug::log("___TEST___@validation: FOUND USER!");
//         const User& user = it->second;
//         const GC_byte* userPasswordData = user.getPasswordData();
//         isValid = memcmp(pReqPasswordData, userPasswordData, USER_PASSWD_SIZE) == 0;
//         if (isValid)
//         {
//             const std::string& userFactionName = user.getFactionName();
//             if (userFactionName.size() > 0)
//             {
//                 userFaction = _game.getFaction(userFactionName);
//             }
//             // Dont allow login if logged in already
//             if (user.isLoggedIn())
//             {
//                 isValid = false;
//             }
//         }
//     }
//     else
//     {
//         Debug::log("___TEST___@validation: FAILED TO FIND USER!");
//     }
//     return std::make_pair(isValid, userFaction);
// }

void Server::trigger_shutdown()
{
    s_shutdown = true;
    Debug::log("Server shutdown triggered");
}

bool Server::is_shutting_down()
{
    return s_shutdown;
}

void Server::disconnectClient(const Client& client)
{
    std::lock_guard<std::mutex> lock(_mutex);
    const std::string& clientAddr = client.getAddress();
    std::unordered_map<std::string, Client>::const_iterator it = _clients.find(clientAddr);
    if (it != _clients.end())
    {
        std::unordered_map<std::string, User>::const_iterator itUserMapping = _clientUserMapping.find(clientAddr);
        if (itUserMapping != _clientUserMapping.end())
            _clientUserMapping.erase(itUserMapping);
        else
            Debug::log(
                "Attempted to remove entry from client-user mapping but didn't find the entry",
                Debug::MessageType::ERROR
            );
        _clients.erase(it);
        close(client.getConnSD());
        Debug::log("Client disconnected: " + clientAddr);
    }
    else
    {
        Debug::log("Failed to disconnect client: " + clientAddr +
            ". Client wasn't found from connected clients",
            Debug::MessageType::ERROR
        );
    }
}

void Server::updateUserData(const Client& client, int32_t xPos, int32_t zPos, int32_t observeRadius)
{
    std::lock_guard<std::mutex> lock(_mutex);
    // TODO: Make this safer by checking can this user even be found!
    _clientUserMapping[client.getAddress()].updateObserveProperties(xPos, zPos, observeRadius);

    // _users[user.getName()].updateObserveProperties(xPos, zPos, observeRadius);
}

void Server::updateUserFaction(const User& user, const Faction& faction)
{
    std::lock_guard<std::mutex> lock(_mutex);
    // TODO: Make this safer by checking can this user even be found!
    // _users[user.getName()].setFactionName(faction.getName());
}

std::unordered_map<std::string, std::string> Server::readConfigFile(const std::string filePath) const
{
    std::unordered_map<std::string, std::string> config;

    std::fstream fileStream(filePath);
    if (!fileStream.is_open())
    {
        Debug::log("Failed to read config file from: " + filePath);
        return config;
    }

    std::string line = "";
    while (std::getline(fileStream, line))
    {
        std::vector<std::string> lineComponents = str_util::split(line, "=");
        if (lineComponents.size() == 0)
            continue;
        if (lineComponents.size() != 2)
        {
            Debug::log("Invalid line on config file: " + line, Debug::MessageType::ERROR);
            continue;
        }
        if (lineComponents[0].empty())
        {
            Debug::log("Key was empty string while reading config file. Line: " + line, Debug::MessageType::ERROR);
            continue;
        }
        config[lineComponents[0]] = lineComponents[1];
    }
    return config;
}
