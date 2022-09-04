
#include <algorithm>
#include <asm-generic/socket.h>
#include <mutex>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <string>

#include "Server.h"
#include "Debug.h"


bool Server::s_shutdown = false;

Server::Server(int port, size_t maxClientCount) :
	_port(port), _maxClientCount(maxClientCount), 
	_game(512), // *NOTE! size of the game world is just temporarely hardcoded here!
	_messageHandler(*this, _game)
{
	// Create socket
	_serverSD = socket(AF_INET, SOCK_STREAM, 0);
	if(_serverSD < 0)
	{
		Debug::log("Failed to create socket");
	}

	// Set reusable for development..
	int opt_reuse = 1;
	if (setsockopt(_serverSD, SOL_SOCKET, SO_REUSEADDR, &opt_reuse, (socklen_t)(sizeof(int))) < 0)
		Debug::log("Failed to set sockopt SO_REUSEADDR");
	
	// Config address
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
	Debug::log("Started server on port:" + std::to_string(port));
}

Server::~Server()
{
}

void Server::beginMsgHandler()
{
	if(!_msgHandlerThread)
		_msgHandlerThread = new std::thread(&MessageHandler::run, &_messageHandler);
	else
		Debug::log("Attempted to launch MessageHandler multiple times!");
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
	
	// Get details of conn..
	//getpeername(connSD, (struct sockaddr*)&clientAddress, &clientLen);
	//char* clientAddrName = inet_ntoa(clientAddress.sin_addr);
	//unsigned short clientPort = ntohs(clientAddress.sin_port);
	
	// TODO: connection validation (using initial message) before adding to "connected clients"
	if (connSD)
	{
		bool connectionExists = false;
		for (ClientData& client : _clients)
		{
			if (client.connSD == connSD)
			{
				connectionExists = true;
				break;
			}
		}
		if (!connectionExists)
			connectNewClient(connSD);
		else
			Debug::log("Double connecting prevented");
	}
}

// TODO: Safe and "complete" server shutdown func
void Server::shutdown()
{
	if(_gameThread)
	{
		_gameThread->join();
		delete _gameThread;
	}
	for (ClientData& client : _clients)
	{
		if (client.connSD)
			close(client.connSD);
	}
	close(_serverSD);
}

void Server::trigger_shutdown()
{
	s_shutdown = true;
	Debug::log("Server shutdown triggered");
}

void Server::connectNewClient(int connSD)
{
	Debug::log("Attempting to connect new client");
	std::lock_guard<std::mutex> lock(_mutex);
	// TODO: some kind of validation stuff..
	ClientData newClient(connSD, "null");
	_clients.push_back(newClient);
}

void Server::disconnectClient(int connSD)
{
	std::lock_guard<std::mutex> lock(_mutex);
	size_t pos = 0;
	for (const ClientData& connectedClient : _clients)
	{
		if (connectedClient.connSD == connSD)
			break;
		pos++;
	}
	Debug::log("Client disconnected");
	_clients.erase(_clients.begin() + pos);
}

void Server::updateClientData(const ClientData& toUpdate, int32_t xPos, int32_t zPos, int32_t observeRadius)
{
	std::lock_guard<std::mutex> lock(_mutex);
	// TODO: optimize client finding
	for (ClientData& client : _clients)
	{
		if (client == toUpdate)
		{
			client.xPos = xPos;
			client.zPos = zPos;
			client.observeRadius = observeRadius;
			break;
		}
	}
}

std::vector<ClientData> Server::getClientConnections() const
{
	std::lock_guard<std::mutex> lock(_mutex);
	std::vector<ClientData> clientList(_clients);
	return clientList;
}

bool Server::is_shutting_down()
{
	return s_shutdown;
}
