
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
	{
		Debug::log("Failed to set sockopt SO_REUSEADDR");
	}
	
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

void Server::beginReqHandler()
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


static int s_current_conn = 0;
void Server::run()
{
	sockaddr_in clientAddress;	
	memset(&clientAddress, 0, sizeof(clientAddress));
	socklen_t clientLen = sizeof(clientAddress);
	int connSD = accept(_serverSD, (struct sockaddr*)&clientAddress, &clientLen);
	s_current_conn = connSD;
	
	// Get details of conn..
	getpeername(connSD, (struct sockaddr*)&clientAddress, &clientLen);
	//char* clientAddrName = inet_ntoa(clientAddress.sin_addr);
	//unsigned short clientPort = ntohs(clientAddress.sin_port);
	
	// TODO: connection validation (using initial message) before adding to "connected clients"
	if (connSD)
		connectNewClient(connSD);
	/*
	size_t readBytes = read(connSD, _pRecvBuf, _maxRecvBufLen);

		std::string fullMessage(_pRecvBuf, readBytes);
		Debug::log("\nFULL MESSAGE:\n" + fullMessage + "\n");
		
		Message msg()
		Request req(connSD, _pRecvBuf + bodyBeginPos, bodySize);

		//Debug::log("Parsed body size was: " + std::to_string(bodySize));
		//_reqHandler.addToReqQueue(req);

		// resert recv buf for next upcomming reqs
		memset(_pRecvBuf, 0, _maxRecvBufLen);

		// JUST FOR TESTING..
		std::string message = "Welcome websockets yey!";
		Debug::log("Attempting to send...");
		send(req.getClientSD(), message.data(), message.size(), 0);
	*/
}

// TODO: Safe and "complete" server shutdown func
void Server::shutdown()
{
	// atm these threads may have infinite loops...
	/*
	if(_reqHandlerThread)
	{
		_reqHandlerThread->join();
		delete _reqHandlerThread;
	}

	if(_gameThread)
	{
		_gameThread->join();
		delete _gameThread;
	}
	*/
	if (s_current_conn)
		close(s_current_conn);
	close(_serverSD);
}

void Server::trigger_shutdown()
{
	s_shutdown = true;
	Debug::log("Server shutdown triggered");
}

void Server::connectNewClient(int connSD)
{
	std::lock_guard<std::mutex> lock(_clientListingMutex);
}

void Server::disconnectClient(int connSD)
{
	std::lock_guard<std::mutex> lock(_clientListingMutex);
	size_t pos = 0;
	for (const ClientData& connectedClient : _clients)
	{
		if (connectedClient.connSD == connSD)
			return;
		pos++;
	}
	_clients.erase(_clients.begin() + pos);
}

std::vector<ClientData> Server::getClientConnections() const
{
	std::lock_guard<std::mutex> lock(_clientListingMutex);
	std::vector<ClientData> clientList(_clients);
	return clientList;
}

bool Server::is_shutting_down()
{
	return s_shutdown;
}

/*
size_t Server::findContentLength(const PK_byte* data, size_t dataLen) const
{
	std::string msg(data, dataLen);
	const std::string toFind = "Content-Length: ";

	size_t pos = msg.find(toFind);
	if(pos != std::string::npos)
	{
		std::string len_str(data + pos + toFind.size());
		return std::stoi(len_str);
	}
	return 0;
}

Request Server::convertToReq(PK_byte* data, size_t dataLen, size_t bodyLen, int clientSD)
{
	size_t bodyBegin = dataLen - bodyLen;
	PK_byte messageType = data[bodyBegin];
	std::vector<ByteBuffer> reqContent;
	
	// attempt to parse user id, if it is contained in the data (its the next part in body after messageType)
	char userID[32];
	memset(userID, 0, 32);
	if(bodyLen > 32)
	{
		memcpy(userID, data+ bodyBegin + 1, 32);
	}

	
	
	Request req(clientSD, data + bodyBegin, bodyLen);
	return req;
}
*/

