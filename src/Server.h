#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include "Common.h"
#include "game/Game.h"
#include "MessageHandler.h"
#include <unordered_map>

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
	std::thread* _msgHandlerThread = nullptr;
	std::thread* _gameThread = nullptr;

	// Currently connected (and validated) clients
	mutable std::mutex _clientListingMutex;
	std::vector<ClientData> _clients;

	static bool s_shutdown;
public:

	Server(int port, size_t maxClientCount);
	~Server();

	void beginReqHandler();
	void beginGame();
	void run();
	void shutdown();

	// Establishes new connection (thread safe)
	void connectNewClient(int connSD);
	// Removes connection (thread safe)
	void disconnectClient(int connSD);

	// Returns vector containing each connection sock. desc. (Thread safely)
	std::vector<ClientData> getClientConnections() const;

	static void trigger_shutdown();
	static bool is_shutting_down();
};

